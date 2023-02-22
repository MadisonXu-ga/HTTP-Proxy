#include "proxy.hpp"

#include <vector>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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
  int request_id = 0;
  while (true) {
    //
    std::cout << "??" << std::endl;
    int client_fd = proxy_server.acceptConnection();
    pthread_t thread;
    pthread_create(&thread, NULL, handleRequest, &client_fd);
    // handleRequest(&client_fd);
    // close(client_fd);   // ?????? but how can I close this fd???
  }
}

void * Proxy::handleRequest(void * fd) {
  std::cout << "handle request begin" << std::endl;

  int client_fd = *((int *)fd);
  //pthread_mutex_lock(&mutex);
  // char request_message[1024 * 1024] = {0};
  vector<char> request_message(1024 * 1024);
  int len = recv(client_fd, &request_message.data()[0], 1000 * 1000, 0);
  //pthread_mutex_unlock(&mutex);make
  std::cout << "len: " << len << std::endl;
  std::cout << "sizeof(request_message): " << sizeof(request_message) << std::endl;
  // makestd::cout << "Let me see see: " << request_message << std::endl;
  if (len <= 0) {
    return NULL;
  }

  request_message.data()[len] = '\0';

  string request_str(request_message.begin(), request_message.end());

  std::cout << "et me see see: request_str: " << request_str << std::endl;

  Request req(request_str);
  // Request initial_request(request_str);
  // std::cout << "Request content is:" << initial_request.request_content << endl;
  // initial_request.parseMethod();
  std::cout << "Method is:" << req.getMethod() << endl;
  std::cout << "Host is:" << req.getHost() << endl;
  std::cout << "Port is:" << req.getPort() << endl;

  if (len <= 0) {
    //error to logfile, now I just print to std::cout
    return NULL;
  }

  if (req.getMethod() == "GET") {
    handleGET(req, client_fd);
    return NULL;
  }
  else if (req.getMethod() == "POST") {
    // handlePOST(req, client_fd);
    // handleCONNECT(req, client_fd);
    return NULL;
  }
  else if (req.getMethod() == "CONNECT") {
    handleCONNECT(req, client_fd);
  }
  else {
    // error print to logfile
    return NULL;
  }

  std::cout << "handle request end" << std::endl;

  return NULL;
}

void Proxy::handleGET(Request req, int client_fd) {
  // Need to consider cache
  // cout << "handleGet begin  HOST: " << req.getHost().c_str() << " "
  //      << req.getPort().c_str() << endl;
  Client my_client(req.getHost().c_str(), req.getPort().c_str());

  // connect to the remote server
  int my_client_fd = my_client.createConnection();
  cout << "Connect to remote server successfully!" << endl;

  // send request to remote server
  // char request_message[65536] = {0};
  const char * request_message = req.getContent().c_str();
  int client_len = send(my_client_fd, request_message, strlen(request_message), 0);
  std::cout << "client_len" << client_len << std::endl;
  std::cout << "GET request to remote server: " << request_message << std::endl;

  // receive respond from remote server
  char response_message[65536] = {0};
  int server_len = recv(my_client_fd, response_message, sizeof(response_message), 0);
  std::cout << "server_len" << server_len << std::endl;
  std::cout << "GET receive: " << response_message << std::endl;
  if (server_len <= 0) {
    return;
  }

  // send resond to client
  int send_len = send(client_fd, response_message, sizeof(response_message), 0);
  std::cout << "send_len" << server_len << std::endl;
  std::cout << "client GET receive: " << response_message << std::endl;
  // int _len = send(my_client_fd, req.getContent(), );
}

void Proxy::handlePOST() {
}

void Proxy::handleCONNECT(Request req, int client_fd) {
  cout << "handleConnect begin  HOST: " << req.getHost().c_str() << " "
       << req.getPort().c_str() << endl;
  // pthread_mutex_lock(&mutex);
  Client my_client(req.getHost().c_str(), req.getPort().c_str());
  // Client my_client("www.google.com", "443");
  // my_client.createClient();
  int my_client_fd = my_client.createConnection();
  // pthread_mutex_unlock(&mutex);
  cout << "Connect to remote server successfully!" << endl;

  std::string response = "HTTP/1.1 200 OK\r\n\r\n";
  send(client_fd, response.c_str(), response.length(), 0);

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
      cout << "Receive length: " << len_recv << endl;
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
      cout << "Receive length: " << len_recv << endl;
      std::cout << "Receive from remote server: " << response_message << std::endl;
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