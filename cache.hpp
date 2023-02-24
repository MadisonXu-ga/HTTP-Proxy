#ifndef CACHE_HPP
#define CACHE_HPP

#include <netdb.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>

#include "request.hpp"
#include "response.hpp"

class Cache {
  int capacity;
  std::unordered_map<std::string, Response> cachePipe;
  std::queue<std::string> cacheQueue;

 public:
  Cache(int capacity) : capacity(capacity){};
  void addToCache(Request req, Response res);
  bool checkValidate(Response res);
  Response * getCacheResonse(Request req, int fd);
  bool isInCache(Request req);
};

#endif
