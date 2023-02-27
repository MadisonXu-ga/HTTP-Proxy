#include "proxy.hpp"

#include <fcntl.h>

#include <fstream>
#include <vector>

std::ofstream proxy_log("./proxy.log");

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

Cache my_cache(100, proxy_log);

void Proxy::makeDaemon() {
  pid_t pid = fork();
  if (pid < 0) {
    pthread_mutex_lock(&mutex);
    proxy_log << "(no-id): ERROR fork() failed" << std::endl;
    pthread_mutex_unlock(&mutex);
    exit(EXIT_FAILURE);
  }
  if (pid > 0) {
    // Exit the parent process
    exit(EXIT_SUCCESS);
  }
  // create a new session and become the session leader
  if (setsid() < 0) {
    pthread_mutex_lock(&mutex);
    proxy_log << "(no-id): ERROR setsid() failed" << std::endl;
    pthread_mutex_unlock(&mutex);
    exit(EXIT_FAILURE);
  }

  // Change current working directory to the root directory
  if (chdir("/") < 0) {
    pthread_mutex_lock(&mutex);
    proxy_log << "(no-id): ERROR chdir() failed" << std::endl;
    pthread_mutex_unlock(&mutex);
    exit(EXIT_FAILURE);
  }

  // Close standard file descriptors and reopen them as /dev/null
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  open("/dev/null", O_RDONLY);
  open("/dev/null", O_WRONLY);
  open("/dev/null", O_RDWR);

  // Clear umask
  umask(0);

  // start the proxy server
  run();
}

void Proxy::run() {
  Server proxy_server(port);
  int status = proxy_server.createServer();
  if (status == -1) {
    pthread_mutex_lock(&mutex);
    proxy_log << "(no-id): ERROR Cannot get address info for host" << std::endl;
    pthread_mutex_unlock(&mutex);
    return;
  }
  else if (status == -2) {
    pthread_mutex_lock(&mutex);
    proxy_log << "(no-id): ERROR Cannot create socket" << std::endl;
    pthread_mutex_unlock(&mutex);
    return;
  }
  else if (status == -3) {
    pthread_mutex_lock(&mutex);
    proxy_log << "(no-id): ERROR Cannot bind socket" << std::endl;
    pthread_mutex_unlock(&mutex);
    return;
  }
  else if (status == -4) {
    pthread_mutex_lock(&mutex);
    proxy_log << "(no-id): ERROR Cannot listen on socket" << std::endl;
    pthread_mutex_unlock(&mutex);
    return;
  }

  // need assign unique id to each request.
  int request_id = -1;
  while (true) {
    std::pair<int, std::string> result = proxy_server.acceptConnection();
    int client_fd = result.first;
    std::string client_ip = result.second;

    if (client_fd == -1) {
      pthread_mutex_lock(&mutex);
      proxy_log << "(no-id): ERROR Cannot accept connection on socket" << std::endl;
      pthread_mutex_unlock(&mutex);
      continue;
    }
    else if (client_fd == -2) {
      pthread_mutex_lock(&mutex);
      proxy_log << "(no-id): ERROR Getnameinfo error" << std::endl;
      pthread_mutex_unlock(&mutex);
      continue;
    }

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
  }
}

void * Proxy::handleRequest(void * args) {
  // get client info
  ClientInfo * client_info = static_cast<ClientInfo *>(args);
  int client_fd = client_info->client_fd;
  int request_id = client_info->request_id;
  std::string client_ip = client_info->client_ip;

  // receive a request from a client
  vector<char> request_message(1024 * 1024);
  int len = recv(client_fd, &request_message.data()[0], 1000 * 1000, 0);
  if (len <= 0) {
    // Send400Error(client_fd);
    close(client_fd);
    return NULL;
  }

  request_message.data()[len] = '\0';
  // covert request to string
  string request_str(request_message.begin(), request_message.end());
  // make a Request object
  Request req(request_str, request_id);

  // ID: "REQUEST" from IPFROM @ TIME
  // ****************change it to log file later***********************//
  std::time_t now = std::time(nullptr);
  std::tm * utc = std::gmtime(&now);
  pthread_mutex_lock(&mutex);
  proxy_log << request_id << ": \"" << req.getFirstLine() << "\" from " << client_ip
            << " @ " << std::asctime(utc);
  pthread_mutex_unlock(&mutex);

  if (req.getMethod() == "GET") {
    handleGET(req, client_fd, request_id);
  }
  else if (req.getMethod() == "POST") {
    handlePOST(req, client_fd);
  }
  else if (req.getMethod() == "CONNECT") {
    // ID: Requesting "REQUEST" from SERVER
    pthread_mutex_lock(&mutex);
    proxy_log << request_id << ": Requesting \"" << req.getFirstLine() << "\" from "
              << req.getHost() << std::endl;
    pthread_mutex_unlock(&mutex);

    handleCONNECT(req, client_fd);

    // ID: Tunnel closed
    pthread_mutex_lock(&mutex);
    proxy_log << request_id << ": Tunnel closed" << std::endl;
    pthread_mutex_unlock(&mutex);
  }
  else {
    // error print to logfile
    Send400Error(client_fd, request_id);
  }

  // close the socket and the thread
  close(client_fd);
  pthread_exit(NULL);

  return NULL;
}

void Proxy::handleGET(Request req, int client_fd, int request_id) {
  // Need to consider cache
  Client my_client(req.getHost().c_str(), req.getPort().c_str());
  // connect to the remote server
  int my_client_fd = my_client.createConnection();
  if (my_client_fd == -1) {
    proxy_log << req.getRequestID() << ": ERROR Cannot connect to socket" << std::endl;
    return;
  }

  // if in cache
  if (my_cache.isInCache(req)) {
    // response
    Response res_in_cache = *(my_cache.getCacheResonse(req, my_client_fd));
    int num_sent = send(client_fd,
                        res_in_cache.getContent().c_str(),
                        res_in_cache.getContent().length(),
                        0);
    if (num_sent == -1) {
      pthread_mutex_lock(&mutex);
      proxy_log << request_id << ": ERROR Sending response to client failed" << std::endl;
      pthread_mutex_unlock(&mutex);
      return;
    }
    // ID: Responding "RESPONSE"
    pthread_mutex_lock(&mutex);
    proxy_log << request_id << ": Responding \"" << res_in_cache.getStatus() << "\""
              << std::endl;
    pthread_mutex_unlock(&mutex);

    close(my_client_fd);
    return;
  }

  // if not in cache
  pthread_mutex_lock(&mutex);
  proxy_log << request_id << ": not in cache" << std::endl;
  pthread_mutex_unlock(&mutex);

  // try \0
  // const char * request_message = req.getContent().c_str();
  char request_message[req.getContent().length() + 1];
  strcpy(request_message, req.getContent().c_str());
  request_message[req.getContent().length()] = '\0';

  int client_len = send(my_client_fd, request_message, strlen(request_message), 0);
  // ID: Requesting "REQUEST" from SERVER
  pthread_mutex_lock(&mutex);
  proxy_log << request_id << ": Requesting \"" << req.getFirstLine() << "\" from "
            << req.getHost() << std::endl;
  pthread_mutex_unlock(&mutex);

  // const size_t first_buffer_size = BUFSIZ;
  char first_buffer[1024];

  // receive first response, contains head and part of body
  int server_len = recv(my_client_fd, first_buffer, 1024, 0);
  if (server_len == -1) {
    Send502Error(client_fd, request_id);
    close(my_client_fd);
    return;
  }

  std::string first_response_message;
  first_response_message.append(first_buffer, server_len);

  Response first_res(first_response_message);

  int remain_len = first_res.getHeadLen() + first_res.getContentlen() - server_len;

  const size_t buffer_size = BUFSIZ;
  char buffer[BUFSIZ];

  int total_length = 0;
  std::string response_message;
  bool response_complete = false;
  while (!response_complete) {
    if (first_res.getContentlen() != 0 && remain_len <= 0) {
      response_complete = true;
      break;
    }
    int server_len = recv(my_client_fd, buffer, BUFSIZ, 0);
    remain_len -= server_len;
    if (server_len < 0) {
      Send502Error(client_fd, request_id);
      close(my_client_fd);
      return;
    }
    else if (server_len == 0) {
      response_complete = true;
      break;
    }
    else {
      first_response_message.append(buffer, server_len);
    }
    // Check if the last chunk has been received
    if (first_response_message.find("\r\n0\r\n") != std::string::npos) {
      response_complete = true;
    }
  }

  // Response res_return(response_str);
  Response res_return(first_response_message);

  // ID: Received "RESPONSE" from	SERVER
  pthread_mutex_lock(&mutex);
  proxy_log << request_id << ": Received \"" << res_return.getStatus() << "\" from	"
            << req.getHost() << std::endl;
  pthread_mutex_unlock(&mutex);

  int num_sent =
      send(client_fd, first_response_message.c_str(), first_response_message.length(), 0);
  if (num_sent == -1) {
    pthread_mutex_lock(&mutex);
    proxy_log << request_id << ": ERROR Sending response to client failed" << std::endl;
    pthread_mutex_unlock(&mutex);
    close(my_client_fd);
    return;
  }

  // ID: Responding "RESPONSE"
  pthread_mutex_lock(&mutex);
  proxy_log << request_id << ": Responding \"" << res_return.getStatus() << "\""
            << std::endl;
  pthread_mutex_unlock(&mutex);

  // if response is not chunked, add it to the cache
  if (!res_return.chunked) {
    pthread_mutex_lock(&mutex);
    my_cache.addToCache(req, res_return);
    pthread_mutex_unlock(&mutex);
  }
  else {
    // ID: not cacheable because chunked
    pthread_mutex_lock(&mutex);
    proxy_log << request_id << ": not cacheable because "
              << "response is chunked" << std::endl;
    pthread_mutex_unlock(&mutex);
  }

  close(my_client_fd);
}

void Proxy::handlePOST(Request req, int client_fd) {
  Client my_client(req.getHost().c_str(), req.getPort().c_str());
  // connect to the remote server
  int my_client_fd = my_client.createConnection();
  if (my_client_fd == -1) {
    proxy_log << req.getRequestID() << ": ERROR Cannot connect to socket" << std::endl;
    return;
  }

  // try \0
  // const char * request_message = req.getContent().c_str();
  char request_message[req.getContent().length() + 1];
  strcpy(request_message, req.getContent().c_str());
  request_message[req.getContent().length()] = '\0';

  int client_len = send(my_client_fd, request_message, strlen(request_message), 0);
  // ID: Requesting "REQUEST" from SERVER
  pthread_mutex_lock(&mutex);
  proxy_log << req.getRequestID() << ": Requesting \"" << req.getFirstLine() << "\" from "
            << req.getHost() << std::endl;
  pthread_mutex_unlock(&mutex);

  // const size_t first_buffer_size = BUFSIZ;
  char first_buffer[1024];

  // receive first response, contains head and part of body
  int server_len = recv(my_client_fd, first_buffer, 1024, 0);
  if (server_len == -1) {
    Send502Error(client_fd, req.getRequestID());
    close(my_client_fd);
    return;
  }

  std::string first_response_message;
  first_response_message.append(first_buffer, server_len);

  Response first_res(first_response_message);

  int remain_len = first_res.getHeadLen() + first_res.getContentlen() - server_len;

  const size_t buffer_size = BUFSIZ;
  char buffer[BUFSIZ];

  int total_length = 0;
  std::string response_message;
  bool response_complete = false;
  while (!response_complete) {
    if (first_res.getContentlen() != 0 && remain_len <= 0) {
      response_complete = true;
      break;
    }
    int server_len = recv(my_client_fd, buffer, BUFSIZ, 0);
    if (server_len < 0) {
      Send502Error(client_fd, req.getRequestID());
      close(my_client_fd);
      return;
    }
    else if (server_len == 0) {
      response_complete = true;
      break;
    }
    else {
      first_response_message.append(buffer, server_len);
    }
    // Check if the last chunk has been received
    if (first_response_message.find("\r\n0\r\n") != std::string::npos) {
      response_complete = true;
    }
  }

  // Response res_return(response_str);
  Response res_return(first_response_message);

  // ID: Received "RESPONSE" from	SERVER
  pthread_mutex_lock(&mutex);
  proxy_log << req.getRequestID() << ": Received \"" << res_return.getStatus()
            << "\" from	" << req.getHost() << std::endl;
  pthread_mutex_unlock(&mutex);

  int num_sent =
      send(client_fd, first_response_message.c_str(), first_response_message.length(), 0);
  if (num_sent == -1) {
    pthread_mutex_lock(&mutex);
    proxy_log << "ERROR Sending response to client failed" << std::endl;
    pthread_mutex_unlock(&mutex);
    close(my_client_fd);
    return;
  }

  // ID: Responding "RESPONSE"
  pthread_mutex_lock(&mutex);
  proxy_log << req.getRequestID() << ": Responding \"" << res_return.getStatus() << "\""
            << std::endl;
  pthread_mutex_unlock(&mutex);

  close(my_client_fd);
}

void Proxy::handleCONNECT(Request req, int client_fd) {
  Client my_client(req.getHost().c_str(), req.getPort().c_str());
  int my_client_fd = my_client.createConnection();
  if (my_client_fd == -1) {
    proxy_log << req.getRequestID() << ": ERROR Cannot connect to socket" << std::endl;
    return;
  }

  std::string response = "HTTP/1.1 200 OK\r\n\r\n";
  send(client_fd, response.c_str(), response.length(), 0);

  // ID: Responding "RESPONSE"
  pthread_mutex_lock(&mutex);
  proxy_log << req.getRequestID() << ": Responding "
            << "\"HTTP/1.1 200 OK\"" << std::endl;
  pthread_mutex_unlock(&mutex);

  fd_set readfds;
  int maxfd = std::max(my_client_fd, client_fd);

  while (1) {
    FD_ZERO(&readfds);
    FD_SET(client_fd, &readfds);
    FD_SET(my_client_fd, &readfds);

    int status = select(maxfd + 1, &readfds, NULL, NULL, NULL);
    if (status == -1) {
      pthread_mutex_lock(&mutex);
      proxy_log << req.getRequestID() << ": ERROR Select failed!" << std::endl;
      pthread_mutex_unlock(&mutex);
      return;
    }

    if (FD_ISSET(client_fd, &readfds)) {
      char request_message[65536] = {0};
      int len_recv = recv(client_fd, request_message, sizeof(request_message), 0);
      if (len_recv <= 0) {
        return;
      }

      int len_send = send(my_client_fd, request_message, len_recv, 0);
      if (len_send <= 0) {
        pthread_mutex_lock(&mutex);
        proxy_log << req.getRequestID() << ": ERROR Forwarding response to client failed"
                  << std::endl;
        pthread_mutex_unlock(&mutex);
        return;
      }
    }

    if (FD_ISSET(my_client_fd, &readfds)) {
      char response_message[65536] = {0};
      int len_recv = recv(my_client_fd, response_message, sizeof(response_message), 0);
      if (len_recv <= 0) {
        return;
      }
      int len_send = send(client_fd, response_message, len_recv, 0);
      if (len_send <= 0) {
        pthread_mutex_lock(&mutex);
        proxy_log << req.getRequestID() << ": ERROR Forwarding response to client failed"
                  << std::endl;
        pthread_mutex_unlock(&mutex);
        return;
      }
    }
  }
}

void Proxy::Send502Error(int client_fd, int id) {
  string response = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
  if (send(client_fd, response.c_str(), response.size(), 0) == -1) {
    pthread_mutex_lock(&mutex);
    proxy_log << id << ": ERROR Send 502 failed!" << std::endl;
    pthread_mutex_unlock(&mutex);
    return;
  }
}

void Proxy::Send400Error(int client_fd, int id) {
  string response = "HTTP/1.1 400 Bad Request\r\n\r\n";
  if (send(client_fd, response.c_str(), response.size(), 0) == -1) {
    pthread_mutex_lock(&mutex);
    proxy_log << id << ": ERROR Send 400 failed!" << std::endl;
    pthread_mutex_unlock(&mutex);
    return;
  }
}