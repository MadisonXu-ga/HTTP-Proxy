#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
using namespace std;

class Response {
 public:
  string response_content = "";
  string status = "";
  string date = "";
  string expire = "";
  string etag = "";
  string last_modify = "";
  string code = "";
  size_t content_length = 0;
  size_t max_age = 0;
  size_t max_stale = 0;
  bool must_revalidate = false;
  bool no_cache = false;
  bool no_store = false;
  bool Private = false;
  bool Public = false;
  bool chunked = false;
  bool has_Maxage = false;

  Response() {}
  Response(string init_response) : response_content(init_response) {
    parseStatus();
    parseEtag();
    parseDate();
    parseExpire();
    parseLastModify();
    parseContent_len();
    parseCache_control();
  }
  void parseStatus();
  void parseEtag();
  void parseDate();
  void parseExpire();
  void parseLastModify();
  void parseContent_len();
  void parseCache_control();
  void parseChunked();
  string getDate();
  string getExpires();
  string getContent();
  string getEtag();
  string getStatus();
  string getLastModify();
  string getCode();
  size_t getMaxage();
  size_t getMaxstale();
  size_t getContentlen();
};

#endif
