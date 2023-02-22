#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "client.hpp"
#include "request.hpp"
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
  static void handleGET(Request req, int fd);
  static void handlePOST();
  static void handleCONNECT(Request req, int fd);
};