#include "client.hpp"

void Client::init_addrinfo() {
  std::memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  int status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }
}

void Client::createSocket() {
  fd = socket(host_info_list->ai_family,
              host_info_list->ai_socktype,
              host_info_list->ai_protocol);
  if (fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    // cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }
}

int Client::createConnection() {
  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "Error: cannot connect to socket" << endl;
    // cerr << "  (" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }
  return fd;
}

void Client::createClient(){
    init_addrinfo();
    createSocket();
}