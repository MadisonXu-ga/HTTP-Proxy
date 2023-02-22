#include "proxy.hpp"
<<<<<<< HEAD

#include "client.hpp"

=======
#include "request.hpp"
>>>>>>> test
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
    // pthread_t thread;
    // pthread_create(&thread, NULL, handleRequest, &client_fd);
    handleRequest(&client_fd);
    close(client_fd);
  }
}

void * Proxy::handleRequest(void * fd) {
  std::cout << "handle request begin" << std::endl;
  
  int client_fd = *((int *)fd);
  char request_message[INT16_MAX] = {0};
  int len = recv(client_fd, &request_message, sizeof(request_message), 0);
  if (len <= 0){
    return NULL;
  }
  string request_str(request_message);
  Request initial_request(request_str);
  std::cout << "Request content is:" << initial_request.request_content << endl;
  initial_request.parseMethod();
  std::cout << "Method is:" << initial_request.method << endl;
  initial_request.parseHost();
  std::cout << "Host is:" << initial_request.host << endl;
  std::cout << "Port is:" << initial_request.port << endl;
  //initial_request.parseMaxage();
  //std::cout << "max_age is:" << initial_request.max_age << endl;
  //initial_request.parseURI();
  //std::cout << "URI is:" << initial_request.URI << endl;
  //std::cout << "len is: " << len << std::endl;
  //std::cout << request_message << std::endl;

  if (len <= 0) {
    //error to logfile, now I just print to std::cout
    return NULL;
  }
  /*
  Request req(request_message);
  if (req.method == "GET") {
    handleGET(req, client_fd);
  }
  else if (req.method == "POST") {
    handlePOST(req, client_fd);
  }
  else if (req.method == "CONNECT") {
    handleCONNECT(req, client_fd);
  }
  else {
    // error print to logfile
    return NULL;
  }
  */

  std::cout << "handle request end" << std::endl;

  return NULL;
}

void Proxy::handleGET() {
  // Need to consider cache
}

void Proxy::handlePOST() {
}

<<<<<<< HEAD
void Proxy::handleCONNECT(Request req, int client_fd) {
  // Client my_client(req.getHost(), req.getPost());
  Client my_client("www.google.com", "443");
  my_client.createClient();
  int my_client_fd = my_client.createConnection();

  fd_set readfds;

  while (1) {
    char message[INT16_MAX];
    FD_ZERO(&readfds);

    FD_SET(client_fd, &readfds);
    FD_SET(my_client_fd, &readfds);

    int status = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
    if(status==-1){
      // error
      exit(EXIT_FAILURE);
    }

    if(FD_ISSET(client_fd, &readfds)){
      recv(client_fd, )
    }
  }
}
=======
void Proxy::handePOST(){}
>>>>>>> test
