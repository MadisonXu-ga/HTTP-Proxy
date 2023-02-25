#include "response.hpp"

void Response::parseStatus() {
  size_t code_begin = response_content.find(" ");
  code_begin += 1;
  size_t code_end = response_content.find(" ", code_begin);
  code = response_content.substr(code_begin, code_end - code_begin);
  size_t Status_end = response_content.find("\r\n");
  status = response_content.substr(0, Status_end);
}

void Response::parseDate() {
  if (response_content.find("Date: ") != string ::npos) {
    size_t Date_begin = response_content.find("Date: ");
    size_t Date_end = response_content.find("\r\n", Date_begin);
    Date_begin += 6;
    date = response_content.substr(Date_begin, Date_end - Date_begin);
  }
}

void Response::parseExpire() {
  if (response_content.find("Expires: ") != string ::npos) {
    size_t Expire_begin = response_content.find("Expires: ");
    size_t Expire_end = response_content.find("\r\n", Expire_begin);
    Expire_begin += 9;
    date = response_content.substr(Expire_begin, Expire_end - Expire_begin);
  }
}

void Response::parseEtag() {
  if (response_content.find("ETag: ") != string ::npos) {
    size_t Etag_begin = response_content.find("ETag: ");
    size_t Etag_end = response_content.find("\r\n", Etag_begin);
    Etag_begin += 6;
    etag = response_content.substr(Etag_begin, Etag_end - Etag_begin);
  }
}

void Response::parseLastModify() {
  if (response_content.find("Last-Modified: ") != string ::npos) {
    size_t LastModify_begin = response_content.find("Last-Modified: ");
    size_t LastModify_end = response_content.find("\r\n", LastModify_begin);
    LastModify_begin += 15;
    last_modify =
        response_content.substr(LastModify_begin, LastModify_end - LastModify_begin);
  }
}
void Response::parsehead_len() {
  head_length = response_content.find("\r\n\r\n");
}
void Response::parseContent_len() {
  if (response_content.find("Content-Length: ") != string ::npos) {
    size_t Contentlen_begin = response_content.find("Content-Length: ");
    size_t Contentlen_end = response_content.find("\r\n", Contentlen_begin);
    Contentlen_begin += 16;
    content_length = stoul(
        response_content.substr(Contentlen_begin, Contentlen_end - Contentlen_begin));
  }
}
void Response::parseChunked() {
  if (response_content.find("chunked") != string ::npos) {
    chunked = true;
  }
}

void Response::parseCache_control() {
  if (response_content.find("Cache-Control: ") != string ::npos) {
    size_t Cache_begin = response_content.find("Cache-Control: ");
    size_t Cache_end = response_content.find("\r\n", Cache_begin);
    string Cache_str = response_content.substr(Cache_begin, Cache_end - Cache_begin);
    size_t length = Cache_str.size();
    size_t Cache_str_end = Cache_str.find("\r\n");
    // max-age
    //size_t Maxage_begin = Cache_str.find("max-age=");
    //size_t Maxage_end = Cache_str.find(",",Maxage_begin);
    if (Cache_str.find("max-age=") != string ::npos) {
      size_t Maxage_begin = Cache_str.find("max-age=");
      has_Maxage = true;
      if (Cache_str.find(",", Maxage_begin) != string ::npos) {
        size_t Maxage_end = Cache_str.find(",", Maxage_begin);
        max_age = stoul(Cache_str.substr(Maxage_begin, Maxage_end - Maxage_begin));
      }
      else {
        max_age = stoul(Cache_str.substr(Maxage_begin, Cache_str_end - Maxage_begin));
      }
    }
    //max_stale
    size_t Maxstale_begin = Cache_str.find("max-stale=");
    size_t Maxstale_end = Cache_str.find(",", Maxstale_begin);
    if (Maxstale_begin < length && Maxstale_end < length) {
      max_stale = stoul(Cache_str.substr(Maxstale_begin, Maxstale_end - Maxstale_begin));
    }
    else if (Maxstale_begin < length && Maxstale_end > length) {
      max_stale = stoul(Cache_str.substr(Maxstale_begin, Cache_str_end - Maxstale_begin));
    }
    // revalidate
    if (Cache_str.find("must-revalidate") != string ::npos) {
      must_revalidate = true;
    }
    if (Cache_str.find("no-cache") != string ::npos) {
      no_cache = true;
    }
    if (Cache_str.find("no-store") != string ::npos) {
      no_store = true;
    }
    if (Cache_str.find("private") != string ::npos) {
      Private = true;
    }
    if (Cache_str.find("public") != string ::npos) {
      Public = true;
    }
  }
}

string Response::getContent() {
  return response_content;
}
string Response::getStatus() {
  return status;
}
string Response::getDate() {
  return date;
}
string Response::getExpires() {
  return expire;
}
string Response::getEtag() {
  return etag;
}
string Response::getLastModify() {
  return last_modify;
}
string Response::getCode() {
  return code;
}
size_t Response::getMaxage() {
  return max_age;
}
size_t Response::getMaxstale() {
  return max_stale;
}
size_t Response::getContentlen() {
  return content_length;
}
size_t Response::getHeadLen() {
  return head_length;
}