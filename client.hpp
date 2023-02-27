#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

class Client {
  const char * hostname;
  const char * port;
  int fd;

  struct addrinfo host_info;
  struct addrinfo * host_info_list;

 public:
  Client() : hostname(NULL), port(NULL) {}
  Client(const char * hostname, const char * port) : hostname(hostname), port(port) {
    int status = createClient();
  }
  ~Client() {
    free(host_info_list);
    close(fd);
  }

  int createClient();
  int init_addrinfo();
  int createSocket();
  int createConnection();
};

#endif