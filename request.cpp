#include "request.hpp"
#include <iostream>
#include <string>

void Request:: parseMethod()
{
    if(request_content.find("GET") != string::npos)
    {
        method = "GET";
    }
    else if(request_content.find("POST") != string::npos)
    {
        method = "POST";
    }
    else if(request_content.find("CONNECT") != string::npos)
    {
        method = "CONNECT";
    }
    else
    {
        size_t endpoint = request_content.find(' ');
        method = request_content.substr(0, endpoint);
    }
}
void Request:: parseHost()
{
    size_t host_begin = request_content.find("Host: ");
    size_t host_end = request_content.find("\r\n", host_begin);
    host_begin += 6;
    host = request_content.substr(host_begin, host_begin - host_end);
    if(method == "GET")
    {
        port = "80";
    }
    else if(method == "CONNECT")
    {
        size_t comma = host.find(":");
        port = host.substr(comma + 1, host_end - comma);
    }
    else
    {
        port = "80";
    }
}

string Request:: getContent() {return request_content;}
string Request:: getMethod() {return method;}
string Request:: getHost() {return host;}