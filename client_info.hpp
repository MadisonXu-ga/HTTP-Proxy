#ifndef CLIENTINFO_HPP
#define CLIENTINFO_HPP

#include <string>

typedef struct ClientInfo_t {
  int client_fd;
  int request_id;
  std::string client_ip;
} ClientInfo;

#endif