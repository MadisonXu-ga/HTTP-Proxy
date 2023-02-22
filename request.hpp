#include <cstring>
#include <iostream>
#include <string>

using namespace std;

class Request {
  string request_content;
  string method;
  string host;
  string port;

 public:
  Request(string init_request) : request_content(init_request) {
    parseMethod();
    parseHost();
  }
  string getContent();
  string getMethod();
  string getHost();
  string getPort();
  void parseMethod();
  void parseHost();
};