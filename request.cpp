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
    if(temp.find(":") != string :: npos)
    {
      port = temp.substr(comma + 1, host_end - comma);
    }
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
void Request::parseMaxStale()
{
  if(request_content.find("Cache-Control: ") != string :: npos)
  {
    size_t Cache_begin = request_content.find("Cache-Control: ");
    size_t Cache_end = request_content.find("\r\n", Cache_begin);
    string Cache_str = request_content.substr(Cache_begin, Cache_end - Cache_begin);
    size_t Cache_str_end = Cache_str.find("\r\n");
    //max_stale
    size_t Maxstale_begin = Cache_str.find("max-stale=");
    size_t Maxstale_end = Cache_str.find(",",Maxstale_begin);
    if(Cache_str.find("max-stale=") != string :: npos)
    {
      size_t Maxstale_begin = Cache_str.find("max-stale=")
      has_MaxStale = true;
      if(Cache_str.find(",",Maxstale_begin) != string :: npos)
      {
        size_t Maxstale_end = Cache_str.find(",",Maxstale_begin);
        max_stale = stoul(Cache_str.substr(Maxstale_begin, Maxstale_end - Maxstale_begin));
      }
      else
      {
        max_stale = stoul(Cache_str.substr(Maxstale_begin, Cache_str_end - Maxstale_begin));
      }
    }
  }
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
int Request::getRequestID(){
  return request_id;
}
size_t Request::getMaxStale()
{
  return max_stale;
}