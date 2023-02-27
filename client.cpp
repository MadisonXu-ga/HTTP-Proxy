#include "client.hpp"

int Client::init_addrinfo() {
  std::memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  int status = getaddrinfo(this->hostname, this->port, &host_info, &host_info_list);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    std::cerr << "  (" << this->hostname << "," << this->port << ")" << std::endl;
    // exit(EXIT_FAILURE);
    return -1;
  }
  return 1;
}

int Client::createSocket() {
  fd = socket(host_info_list->ai_family,
              host_info_list->ai_socktype,
              host_info_list->ai_protocol);
  if (fd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    // std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    // exit(EXIT_FAILURE);
    return -2;
  }
  return 1;
}

int Client::createConnection() {
  int status = connect(fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    // std::cerr << "Error: cannot connect to socket" << std::endl;
    // std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    // exit(EXIT_FAILURE);
    return -1;
  }
  return fd;
}

int Client::createClient(){
    int status = init_addrinfo();
    status = createSocket();

    return status;
}