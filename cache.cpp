#include "cache.hpp"

// latter need to add to response maybe
// Parse a string representation of time in an HTTP response and return it as a std::time_t value
std::time_t parseHttpResponseTime(const std::string & time_str) {
  std::tm tm = {};
  std::istringstream ss(time_str);
  ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
  return std::mktime(&tm);
}

void Cache::addToCache(Request req, Response res) {
  // if no-store or private, do not store this response in http proxy
  if (res.no_store || res.Private) {
    // do not store
    return;
  }

  // check if it is out of capacity
  if (cacheQueue.size() == capacity) {
    std::string isGoingToPop = cacheQueue.front();
    cacheQueue.pop();
    cachePipe.erase(isGoingToPop);
  }

  // add to cache
  cachePipe[req.getURI()] = res;
  // to record the order of requests
  cacheQueue.push(req.getURI());
}

bool Cache::checkValidate(Response res, int request_id) {
  // should I check at request??
  std::time_t now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::time_t response_receive_time = parseHttpResponseTime(res.getDate());

  // ????? do i need max-age=0 for must-revalidate???
  // if no-cache, need to send a validation request before using any stored response
  if (res.no_cache || (res.must_revalidate && res.has_Maxage && res.getMaxage() == 0)) {
    // ID: in cache, requires validation
    std::cout << request_id << " in cache, requires validation" << std::endl;
    return false;
  }

  // this is a problem!!! max-age could not exist!!!!!!!!!! Fix this!!!!!!
  if (res.getExpires().empty() && !res.has_Maxage) {
    // ID: in cache, valid
    std::cout << request_id << ": in cache, valid" << std::endl;
    return true;
  }

  std::time_t expires_time;
  // no max-age, according to expires_time
  // Fix this!!!!!!!!!
  if (!res.has_Maxage) {
    expires_time = parseHttpResponseTime(res.getExpires());
  }
  // according to max-age (+ max-stale)
  else {
    if(req){
      ss
    }
    expires_time = response_receive_time + res.getMaxage() + res.getMaxstale();
  }
  if (expires_time < now) {
    // ID: in cache, but expired at EXPIREDTIME
    std::tm * utc = std::gmtime(&expires_time);
    std::cout << request_id << ": in cache, but expired at " << std::asctime(utc);
    return false;
  }

  std::cout << request_id << ": in cache, valid" << std::endl;
  return true;
}

Response * Cache::getCacheResonse(Request req, int fd) {
  // update->return 200    not update return 304
  Response * res_in_cache = &cachePipe[req.getURI()];
  // validate, just return
  if (checkValidate(*res_in_cache, req.getRequestID())) {
    return res_in_cache;
  }
  // invalidate, need to ask server
  // create request
  std::string validate_request = "GET " + req.getURI() + " HTTP/1.1\r\n";
  validate_request += "Host: " + req.getHost() + "\r\n";
  validate_request += "If-None-Match: " + res_in_cache->getEtag() + "\r\n";
  validate_request += "If-Modified-Since: " + res_in_cache->getLastModify() + "\r\n";
  validate_request += "Connection: close\r\n\r\n";

  // send request to server
  // ID: Requesting "REQUEST" from SERVER
  std::cout << req.getRequestID() << ": Requesting \"" << req.getFirstLine() << "\" from " << req.getHost() << std::endl;
  if (send(fd, validate_request.c_str(), validate_request.length(), 0) < 0) {
    std::cerr << "Error sending validate_request." << std::endl;
    // error
    exit(EXIT_FAILURE);
  }

  // receive response from the server
  std::vector<char> response_buffer(1024);
  std::string response;
  int num_bytes;
  while ((num_bytes = recv(fd, response_buffer.data(), response_buffer.size(), 0)) > 0) {
    response.append(response_buffer.begin(), response_buffer.begin() + num_bytes);
  }

  Response * validate_response = new Response(response);
  // ID: Received "RESPONSE" from	SERVER
  std::cout << req.getRequestID() << ": Received \"" << validate_response->getStatus() << "\"" << std::endl;

  // 304 not modified
  if (validate_response->getCode() == "304") {
    // do i need to change max-age or something?
    return res_in_cache;
  }
  else if (validate_response->getCode() == "200") {
    // just update?
    cachePipe[req.getURI()] = *validate_response;
    return validate_response;
  }
  else {
    // error
    std::cerr << "validate failed!" << endl;
    exit(EXIT_FAILURE);
  }
}

bool Cache::isInCache(Request req) {
  std::unordered_map<std::string, Response>::const_iterator it =
      cachePipe.find(req.getURI());
  if (it == cachePipe.end()) {
    return false;
  }
  return true;
}
