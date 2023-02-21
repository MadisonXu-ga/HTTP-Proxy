#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

using namespace std;

class Server {
  const char * hostname;
  const char * port;
  int fd;

  struct addrinfo my_host_info;
  struct addrinfo * my_host_info_list;

 public:
  Server() : hostname(NULL), port(NULL) {}
  Server(const char * port) : hostname(NULL), port(port) {}
  ~Server(){
    free(my_host_info_list);
    close(fd);
  }

  void createServer() {
    init_addrinfo();
    createSocket();
    listenToSocket();
  }

  void init_addrinfo() {
    memset(&my_host_info, 0, sizeof(my_host_info));

    my_host_info.ai_family = AF_UNSPEC;
    my_host_info.ai_socktype = SOCK_STREAM;
    my_host_info.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(hostname, port, &my_host_info, &my_host_info_list);
    if (status != 0) {
      cerr << "Error: cannot get address info for host" << endl;
      exit(EXIT_FAILURE);
    }
    // assign port randomly
    // struct sockaddr_in * addr_in = (struct sockaddr_in *)(my_host_info_list->ai_addr);
    // addr_in->sin_port = 0;
  }

  void createSocket() {
    // create socket
    fd = socket(my_host_info_list->ai_family,
                my_host_info_list->ai_socktype,
                my_host_info_list->ai_protocol);
    if (fd == -1) {
      cerr << "Error: cannot create socket" << endl;
      cerr << "  (" << hostname << "," << port << ")" << endl;
      exit(EXIT_FAILURE);
    }
  }

  void listenToSocket() {
    // bind socket
    int yes = 1;
    int status = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(fd, my_host_info_list->ai_addr, my_host_info_list->ai_addrlen);
    if (status == -1) {
      cerr << "Error: cannot bind socket" << endl;
      cerr << "  (" << hostname << "," << port << ")" << endl;
      exit(EXIT_FAILURE);
    }

    // listen to socket
    status = listen(fd, 100);
    if (status == -1) {
      cerr << "Error: cannot listen on socket" << endl;
      cerr << "  (" << hostname << "," << port << ")" << endl;
      exit(EXIT_FAILURE);
    }
  }

  int acceptConnection() {
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);

    int connect_fd = accept(fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (connect_fd == -1) {
      cerr << "Error: cannot accept connection on socket" << endl;
      exit(-1);
    }

    char request_message[INT16_MAX] = {0};
    int len = recv(connect_fd, &request_message, sizeof(request_message), 0);
    cout << len << endl;
    cout << request_message << endl;

    close(connect_fd);

    return connect_fd;
  }

  //   void get_port() {
  //     struct sockaddr_in addr;
  //     socklen_t addr_len = sizeof(addr);
  //     status = getsockname(left_fd_initial, (struct sockaddr *)&addr, &addr_len);
  //     if (status == -1) {
  //       cerr << "getsockname failed" << endl;
  //       return -1;
  //     }

  //     port = to_string(ntohs(addr.sin_port)).c_str();
  //   }
};

class Proxy {
  const char * hostname;
  const char * port;
  // int server_fd;
  // int client_fd;

 public:
  Proxy() : hostname(NULL), port(NULL) {}
  Proxy(const char * port) : hostname(NULL), port(port) {}

  void makeDaemon();
  static void * handleRequest(void * fd);
};

void Proxy::makeDaemon() {
  // pid_t pid = fork();
  // if (pid < 0) {
  //   cerr << "Error: fork() failed" << endl;
  //   exit(EXIT_FAILURE);
  // }
  // if (pid > 0) {
  //   // Exit the parent process
  //   exit(EXIT_SUCCESS);
  // }
  // cout << pid << endl;

  // // create a new session and become the session leader
  // if (setsid() < 0) {
  //   cerr << "Error: setsid() failed" << endl;
  //   exit(EXIT_FAILURE);
  // }

  // // Close stdin/stderr/stdout, open them to /dev/null
  // // freopen("/dev/null", "r", stdin);
  // // freopen("/dev/null", "w", stdout);
  // // freopen("/dev/null", "w", stderr);

  // // cout << "4" << endl;
  // // Change current working directory to the root directory
  // if (chdir("/") < 0) {
  //   cerr << "Error: chdir() failed" << endl;
  //   exit(EXIT_FAILURE);
  // }

  // // cout << "5" << endl;
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

  Server proxy_server(port);
  proxy_server.createServer();

  while (true) {
    //
    cout << "??" << endl;
    int client_fd = proxy_server.acceptConnection();
    char request_message[INT16_MAX] = {0};
    int len = recv(client_fd, &request_message, sizeof(request_message), 0);
    cout << len << endl;
    cout << request_message << endl;
    // pthread_t thread;
    // pthread_create(&thread, NULL, handleRequest, &client_fd);
    close(client_fd);
  }
}

void * Proxy::handleRequest(void * fd) {
  cout << "handle request begin" << endl;
  int client_fd = *((int *)fd);
  char request_message[INT8_MAX] = {0};
  int len = recv(client_fd, &request_message, sizeof(request_message), MSG_WAITALL);
  cout << len << endl;
  cout << request_message << endl;

  cout << "handle request end" << endl;

  return NULL;
}