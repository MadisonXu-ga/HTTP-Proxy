#include "proxy.hpp"

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
    close(client_fd);
  }
}

void * Proxy::handleRequest(void * fd) {
  std::cout << "handle request begin" << std::endl;
  int client_fd = *((int *)fd);
  char request_message[INT16_MAX] = {0};
  int len = recv(client_fd, &request_message, sizeof(request_message), 0);
  std::cout << len << std::endl;
  std::cout << request_message << std::endl;

  if (len <= 0) {
    //error to logfile, now I just print to std::cout
    return NULL;
  }

  Request req(request_message);
  if (req.method == "GET") {
    handleGET(./);
  }
  else if (req.method == "POST") {
  }
  else if (req.method == "CONNECT") {
  }
  else {
    // error print to logfile
    return NULL;
  }

  std::cout << "handle request end" << std::endl;

  return NULL;
}

void Proxy::handleGET(){
  // Need to consider cache

  
}

void Proxy::handePOST(){}

void Proxy::handleGET(){}