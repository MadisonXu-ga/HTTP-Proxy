#ifndef SERVER_HPP
#define SERVER_HPP

#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <cstring>

class Server {
  const char * hostname;
  const char * port;
  int fd;

  struct addrinfo my_host_info;
  struct addrinfo * my_host_info_list;

 public:
  Server() : hostname(NULL), port(NULL) {}
  Server(const char * port) : hostname(NULL), port(port) {}
  ~Server() {
    free(my_host_info_list);
    close(fd);
  }
  void createServer();
  void init_addrinfo();
  void createSocket();
  void listenToSocket();
  int acceptConnection();
};
#endif // SERVER_HPP