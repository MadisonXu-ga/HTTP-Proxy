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
  if (res.no_store) {
    // do not store
    proxy_log << req.getRequestID() << ": not cacheable because "
              << "response is no-store" << std::endl;
    return;
  }
  if (res.Private) {
    proxy_log << req.getRequestID() << ": not cacheable because "
              << "response is private" << std::endl;
    return;
  }
  if (res.getEtag().empty() && res.getLastModify().empty()) {
    proxy_log << req.getRequestID() << ": not cacheable because "
              << "response does not have Etag or Last Modified Time" << std::endl;
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

bool Cache::checkValidate(Request req, Response res, int request_id) {
  // should I check at request??
  std::time_t now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::time_t response_receive_time = parseHttpResponseTime(res.getDate());

  // if no-cache, need to send a validation request before using any stored response
  if (res.no_cache) {
    // ID: in cache, requires validation
    proxy_log << request_id << ": in cache, requires validation" << std::endl;
    return false;
  }

  std::time_t expires_time;
  // if has max-age
  if (res.has_Maxage) {
    // max_age + date
    expires_time = response_receive_time + res.getMaxage();
  }
  // do not have max-age but has expire time
  else if (!res.getExpires().empty()) {
    expires_time = parseHttpResponseTime(res.getExpires());
  }
  // has nothing
  else {
    proxy_log << request_id << ": in cache, valid" << std::endl;
    return true;
  }

  // has max-age or expire time
  // if do not have must_revalidate
  if (!res.must_revalidate) {
    expires_time += req.getMaxStale();
  }

  // check the time
  if (expires_time < now) {
    // ID: in cache, but expired at EXPIREDTIME
    std::tm * utc = std::gmtime(&expires_time);
    proxy_log << request_id << ": in cache, but expired at " << std::asctime(utc);
    return false;
  }

  proxy_log << request_id << ": in cache, valid" << std::endl;
  return true;
}

Response * Cache::getCacheResonse(Request req, int fd) {
  // update->return 200    not update return 304
  Response * res_in_cache = &cachePipe[req.getURI()];
  // validate, just return
  if (checkValidate(req, *res_in_cache, req.getRequestID())) {
    return res_in_cache;
  }
  // invalidate, need to ask server
  // create request
  std::string validate_request = "GET " + req.getURI() + " HTTP/1.1\r\n";
  validate_request += "Host: " + req.getHost() + "\r\n";
  if (!(res_in_cache->getEtag()).empty()) {
    validate_request += "If-None-Match: " + res_in_cache->getEtag() + "\r\n";
  }
  if (!(res_in_cache->getLastModify()).empty()) {
    validate_request += "If-Modified-Since: " + res_in_cache->getLastModify() + "\r\n";
  }
  validate_request += "Connection: close\r\n\r\n";

  // send request to server
  // ID: Requesting "REQUEST" from SERVER
  proxy_log << req.getRequestID() << ": Requesting \"" << req.getFirstLine() << "\" from "
            << req.getHost() << std::endl;
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
  proxy_log << req.getRequestID() << ": Received \"" << validate_response->getStatus()
            << "\"" << std::endl;

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
