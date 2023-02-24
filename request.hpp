#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <cstring>
#include <iostream>
#include <string>

using namespace std;

class Request {
  string request_content;
  string method;
  string host;
  string port;
  string URI;
  string Fline;
  int request_id;
  size_t max_stale = 0;
  bool has_MaxStale = false;

 public:
  Request(string init_request, int request_id) : request_content(init_request), request_id(request_id) {
    parseFirstLine();
    parseMethod();
    parseURI();
    parseHost();
    parseMaxStale();
  }
  string getContent();
  string getFirstLine();
  string getMethod();
  string getHost();
  string getPort();
  string getURI();
  size_t getMaxStale();
  int getRequestID();
  void parseMethod();
  void parseHost();
  void parseURI();
  void parseFirstLine();
  void parseMaxStale();
};

#endif