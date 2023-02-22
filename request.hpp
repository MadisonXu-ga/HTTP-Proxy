#include <iostream>
#include <string>
#include <cstring>
using namespace std;

class Request{
public:
    string request_content;
    string method;
    string host;
    string port;

    string getContent();
    string getMethod();
    string getHost();
    void parseMethod();
    void parseHost();
    Request(string init_request) : request_content(init_request){}
};