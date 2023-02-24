#ifndef PROXY_HPP
#define PROXY_HPP

#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "cache.hpp"
#include "client.hpp"
#include "client_info.hpp"
#include "request.hpp"
#include "response.hpp"
#include "server.hpp"

class Proxy {
  const char * hostname;
  const char * port;

 public:
  Proxy() : hostname(NULL), port(NULL) {}
  Proxy(const char * port) : hostname(NULL), port(port) {}

  void makeDaemon();
  void run();
  static void * handleRequest(void * args);
  static void handleGET(Request req, int fd);
  static void handlePOST(Request req, int fd);
  static void handleCONNECT(Request req, int fd);
};

#endif
