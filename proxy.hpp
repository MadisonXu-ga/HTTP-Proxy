#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "server.hpp"

class Proxy {
  const char * hostname;
  const char * port;

 public:
  Proxy() : hostname(NULL), port(NULL) {}
  Proxy(const char * port) : hostname(NULL), port(port) {}

  void makeDaemon();
  void run();
  static void * handleRequest(void * fd);
  static void handleGET();
  static void handePOST();
  static void handleConnect();
};