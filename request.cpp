#include "request.hpp"

#include <iostream>
#include <string>

void Request::parseMethod() {
  if (request_content.find("GET") != string::npos) {
    method = "GET";
  }
  else if (request_content.find("POST") != string::npos) {
    method = "POST";
  }
  else if (request_content.find("CONNECT") != string::npos) {
    method = "CONNECT";
  }
  else {
    size_t endpoint = request_content.find(' ');
    method = request_content.substr(0, endpoint);
  }
}
void Request::parseHost() {
  size_t host_begin = request_content.find("Host: ");
  size_t host_end = request_content.find("\r\n", host_begin);
  host_begin += 6;
  string temp = request_content.substr(host_begin, host_end - host_begin);
  size_t comma = temp.find(":");
  // cout << "comma is:" << comma << endl;
  host = temp.substr(0, comma);
  // cout << "host is:" << host << endl;
  if (method == "GET") {
    port = "80";
  }
  else if (method == "CONNECT") {
    port = temp.substr(comma + 1, host_end - comma);
  }
  else {
    port = "80";
  }
}

void Request::parseURI() {
  if (request_content.find(" ") != string::npos) {
    size_t URI_begin = request_content.find(" ");
    URI_begin += 1;
    size_t URI_end = request_content.find(" ", URI_begin);

    URI = request_content.substr(URI_begin, URI_end - URI_begin);
  }
}

void Request::parseFirstLine() {
  size_t end = request_content.find("\r\n");
  Fline = request_content.substr(0, end);
}

string Request::getContent() {
  return request_content;
}
string Request::getMethod() {
  return method;
}
string Request::getHost() {
  return host;
}
string Request::getPort() {
  return port;
}
string Request::getURI() {
  return URI;
}
string Request::getFirstLine() {
  return Fline;
}