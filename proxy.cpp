#include "proxy.hpp"

#include <fcntl.h>

#include <fstream>
#include <vector>

std::ofstream proxy_log("/var/log/erss/proxy.log");

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

Cache my_cache(100);

void Proxy::makeDaemon() {
  // pid_t pid = fork();
  // if (pid < 0) {
  //   cerr << "Error: fork() failed" << std::endl;
  //   exit(EXIT_FAILURE);
  // }
  // if (pid > 0) {
  //   // Exit the parent process
  //   exit(EXIT_SUCCESS);
  // }
  // std::cout << pid << std::endl;

  // // create a new session and become the session leader
  // if (setsid() < 0) {
  //   cerr << "Error: setsid() failed" << std::endl;
  //   exit(EXIT_FAILURE);
  // }

  // // Close stdin/stderr/stdout, open them to /dev/null
  // // freopen("/dev/null", "r", stdin);
  // // freopen("/dev/null", "w", stdout);
  // // freopen("/dev/null", "w", stderr);

  // // std::cout << "4" << std::endl;
  // // Change current working directory to the root directory
  // if (chdir("/") < 0) {
  //   cerr << "Error: chdir() failed" << std::endl;
  //   exit(EXIT_FAILURE);
  // }

  // // std::cout << "5" << std::endl;
  // // Clear umask
  // umask(0);

  // // Fork again and exit parent
  // pid = fork();
  // if (pid < 0) {
  //   perror("fork");
  //   exit(EXIT_FAILURE);
  // }
  // if (pid > 0) {
  //   exit(EXIT_SUCCESS);
  // }
  run();
}

void Proxy::run() {
  Server proxy_server(port);
  proxy_server.createServer();

  // need assign unique id to each request.
  int request_id = -1;
  while (true) {
    // std::cout << "??" << std::endl;
    std::pair<int, std::string> result = proxy_server.acceptConnection();
    int client_fd = result.first;
    std::string client_ip = result.second;

    // do i need to lock this?? I don't think i need
    pthread_mutex_lock(&mutex);
    request_id += 1;
    pthread_mutex_unlock(&mutex);

    ClientInfo client_info;
    client_info.client_fd = client_fd;
    client_info.client_ip = client_ip;
    client_info.request_id = request_id;

    pthread_t thread;
    pthread_create(&thread, NULL, handleRequest, &client_info);
    // handleRequest(&client_fd);
  }
}

void * Proxy::handleRequest(void * args) {
  // std::cout << "handle request begin" << std::endl;

  // get client info
  ClientInfo * client_info = static_cast<ClientInfo *>(args);
  int client_fd = client_info->client_fd;
  int request_id = client_info->request_id;
  std::string client_ip = client_info->client_ip;

  // receive a request from a client
  vector<char> request_message(1024 * 1024);
  int len = recv(client_fd, &request_message.data()[0], 1000 * 1000, 0);
  // std::cout << "len: " << len << std::endl;
  // std::cout << "sizeof(request_message): " << sizeof(request_message) << std::endl;
  // makestd::cout << "Let me see see: " << request_message << std::endl;
  if (len <= 0) {
    //error to logfile, now I just print to std::cout
    return NULL;
  }
  request_message.data()[len] = '\0';

  // covert request to string
  string request_str(request_message.begin(), request_message.end());

  // std::cout << "et me see see: request_str: " << request_str << std::endl;

  // make a Request object
  Request req(request_str, request_id);
  // Request initial_request(request_str);
  // std::cout << "Request content is:" << initial_request.request_content << endl;
  // initial_request.parseMethod();

  // std::cout << "Method is:" << req.getMethod() << endl;
  // std::cout << "Host is:" << req.getHost() << endl;
  // std::cout << "Port is:" << req.getPort() << endl;

  // ID: "REQUEST" from IPFROM @ TIME
  // ****************change it to log file later***********************//
  std::time_t now = std::time(nullptr);
  std::tm * utc = std::gmtime(&now);
  std::cout << request_id << ": \"" << req.getFirstLine() << "\" from " << client_ip
            << " @ " << std::asctime(utc);

  if (req.getMethod() == "GET") {
    handleGET(req, client_fd, request_id);
    // return NULL;
  }
  else if (req.getMethod() == "POST") {
    handlePOST(req, client_fd);
    // return NULL;
  }
  else if (req.getMethod() == "CONNECT") {
    // ??? do i need to output here?
    // ID: Requesting "REQUEST" from SERVER
    // ****************change it to log file later***********************//
    std::cout << request_id << ": Requesting \"" << req.getFirstLine() << "\" from "
              << req.getHost() << std::endl;
    handleCONNECT(req, client_fd);
    // ID: Tunnel closed
    std::cout << request_id << ": Tunnel closed" << std::endl;
  }
  else {
    // error print to logfile
    // return NULL;
  }

  close(client_fd);

  pthread_exit(NULL);
  return NULL;
}

void Proxy::handleGET(Request req, int client_fd, int request_id) {
  // Need to consider cache
  // cout << "handleGet begin  HOST: " << req.getHost().c_str() << " "
  //      << req.getPort().c_str() << endl;
  Client my_client(req.getHost().c_str(), req.getPort().c_str());

  // connect to the remote server
  int my_client_fd = my_client.createConnection();
  // cout << "Connect to remote server successfully!" << endl;

  // if in cache
  if (my_cache.isInCache(req)) {
    // response
    Response res_in_cache = *(my_cache.getCacheResonse(req, my_client_fd));
    int num_sent = send(client_fd,
                        res_in_cache.getContent().c_str(),
                        res_in_cache.getContent().length(),
                        0);
    if (num_sent == -1) {
      // Handle error
      return;
    }
    // ID: Responding "RESPONSE"
    std::cout << request_id << ": Responding \"" << res_in_cache.getStatus() << "\""
              << std::endl;
    return;
  }

  // if not in cache
  // ****************change it to log file later***********************//
  std::cout << request_id << ": not in cache" << std::endl;

  // try \0
  // const char * request_message = req.getContent().c_str();
  char request_message[req.getContent().length() + 1];
  strcpy(request_message, req.getContent().c_str());
  request_message[req.getContent().length()] = '\0';

  int client_len = send(my_client_fd, request_message, strlen(request_message), 0);
  // ID: Requesting "REQUEST" from SERVER
  std::cout << request_id << ": Requesting \"" << req.getFirstLine() << "\" from "
            << req.getHost() << std::endl;
  // std::cout << "client_len" << client_len << std::endl;
  // std::cout << "GET request to remote server: " << request_message << std::endl;

  //const size_t buffer_size = BUFSIZ;
  char buffer[BUFSIZ];

  int total_length = 0;
  string response_message;
  while (1) {
    std::cout << request_id << ": Enter the loop??????????" << std::endl;
    int server_len = recv(my_client_fd, buffer, BUFSIZ, 0);
    std::cout << request_id << ": 111111111111 server_len: " << server_len << std::endl;
    if (server_len < 0) {
      std::cerr << "Error" << endl;
      return;
    }
    else if (server_len <= 0) {
      break;
    }
    else {
      // response_message.insert(response_message.end(), buffer, buffer + server_len);
      std::cout << request_id << ": 222222222 server_len: " << server_len << std::endl;
      response_message.append(buffer, server_len);
    }
  }

  Response res_return(response_message);

  // ID: Received "RESPONSE" from	SERVER
  std::cout << request_id << ": Received \"" << res_return.getStatus() << "\" from	"
            << req.getHost() << std::endl;

  int num_sent = send(client_fd, response_message.c_str(), response_message.length(), 0);
  if (num_sent == -1) {
    // Handle error
    return;
  }

  // ID: Responding "RESPONSE"
  std::cout << request_id << ": Responding \"" << res_return.getStatus() << "\""
            << std::endl;

  // add it to cache
  pthread_mutex_lock(&mutex);
  my_cache.addToCache(req, res_return);
  pthread_mutex_unlock(&mutex);

  close(my_client_fd);
}

void Proxy::handlePOST(Request req, int client_fd) {
  Client my_client(req.getHost().c_str(), req.getPort().c_str());
  // connect to the remote server
  int my_client_fd = my_client.createConnection();

  const char * request_message = req.getContent().c_str();
  int client_len = send(my_client_fd, request_message, strlen(request_message), 0);

  // ID: Requesting "REQUEST" from SERVER
  std::cout << req.getRequestID() << ": Requesting \"" << req.getFirstLine() << "\" from "
            << req.getHost() << std::endl;

  const size_t buffer_size = 1024;
  char buffer[buffer_size] = {0};

  int total_length = 0;
  string response_message;
  while (1) {
    int server_len = recv(my_client_fd, buffer, buffer_size, 0);
    if (server_len == -1) {
      std::cerr << "Error" << endl;
      return;
    }
    else if (server_len == 0) {
      break;
    }
    else {
      response_message.append(buffer, server_len);
    }
  }

  Response res_return(response_message);
  // ID: Received "RESPONSE" from	SERVER
  std::cout << req.getRequestID() << ": Received \"" << res_return.getStatus()
            << "\" from	" << req.getHost() << std::endl;

  int num_sent = send(client_fd, response_message.c_str(), response_message.length(), 0);
  if (num_sent == -1) {
    // Handle error
    return;
  }

  // ID: Responding "RESPONSE"
  std::cout << req.getRequestID() << ": Responding \"" << res_return.getStatus() << "\""
            << std::endl;

  close(my_client_fd);
}

void Proxy::handleCONNECT(Request req, int client_fd) {
  Client my_client(req.getHost().c_str(), req.getPort().c_str());
  int my_client_fd = my_client.createConnection();

  std::string response = "HTTP/1.1 200 OK\r\n\r\n";
  send(client_fd, response.c_str(), response.length(), 0);

  // ID: Responding "RESPONSE"
  std::cout << req.getRequestID() << ": Responding "
            << "\"HTTP/1.1 200 OK\"" << std::endl;

  fd_set readfds;
  int maxfd = std::max(my_client_fd, client_fd);

  while (1) {
    FD_ZERO(&readfds);
    FD_SET(client_fd, &readfds);
    FD_SET(my_client_fd, &readfds);

    int status = select(maxfd + 1, &readfds, NULL, NULL, NULL);
    if (status == -1) {
      // error
      exit(EXIT_FAILURE);
    }

    if (FD_ISSET(client_fd, &readfds)) {
      char request_message[65536] = {0};
      int len_recv = recv(client_fd, request_message, sizeof(request_message), 0);
      // cout << "Receive length: " << len_recv << endl;
      // std::cout << "Receive from client: " << request_message << std::endl;
      if (len_recv <= 0) {
        // error or end?
        return;
      }

      int len_send = send(my_client_fd, request_message, len_recv, 0);
      if (len_send <= 0) {
        // I think this is an error?
        return;
      }
    }

    if (FD_ISSET(my_client_fd, &readfds)) {
      char response_message[65536] = {0};
      int len_recv = recv(my_client_fd, response_message, sizeof(response_message), 0);
      // cout << "Receive length: " << len_recv << endl;
      // std::cout << "Receive from remote server: " << response_message << std::endl;
      if (len_recv <= 0) {
        // error or end?
        return;
      }
      int len_send = send(client_fd, response_message, len_recv, 0);
      if (len_send <= 0) {
        // I think this is an error?
        return;
      }
    }
  }
}
