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

 public:
  Request(string init_request) : request_content(init_request) {
    parseFirstLine();
    parseMethod();
    parseURI();
    parseHost();
  }
  string getContent();
  string getFirstLine();
  string getMethod();
  string getHost();
  string getPort();
  string getURI();
  void parseMethod();
  void parseHost();
  void parseURI();
  void parseFirstLine();
};

#endif