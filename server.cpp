#include "server.hpp"

void Server::createServer() {
  init_addrinfo();
  createSocket();
  listenToSocket();
}

void Server::init_addrinfo() {
  std::memset(&my_host_info, 0, sizeof(my_host_info));

  my_host_info.ai_family = AF_UNSPEC;
  my_host_info.ai_socktype = SOCK_STREAM;
  my_host_info.ai_flags = AI_PASSIVE;

  int status = getaddrinfo(hostname, port, &my_host_info, &my_host_info_list);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    exit(EXIT_FAILURE);
  }
  // assign port randomly
  // struct sockaddr_in * addr_in = (struct sockaddr_in *)(my_host_info_list->ai_addr);
  // addr_in->sin_port = 0;
}

void Server::createSocket() {
  // create socket
  fd = socket(my_host_info_list->ai_family,
              my_host_info_list->ai_socktype,
              my_host_info_list->ai_protocol);
  if (fd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    exit(EXIT_FAILURE);
  }
}

void Server::listenToSocket() {
  // bind socket
  int yes = 1;
  int status = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(fd, my_host_info_list->ai_addr, my_host_info_list->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error: cannot bind socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    exit(EXIT_FAILURE);
  }

  // listen to socket
  status = listen(fd, 100);
  if (status == -1) {
    std::cerr << "Error: cannot listen on socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    exit(EXIT_FAILURE);
  }
}

int Server::acceptConnection() {
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);

  int connect_fd = accept(fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (connect_fd == -1) {
    std::cerr << "Error: cannot accept connection on socket" << std::endl;
    exit(-1);
  }

  return connect_fd;
}

//   void get_port() {
//     struct sockaddr_in addr;
//     socklen_t addr_len = sizeof(addr);
//     status = getsockname(left_fd_initial, (struct sockaddr *)&addr, &addr_len);
//     if (status == -1) {
//       std::cerr << "getsockname failed" << std::endl;
//       return -1;
//     }

//     port = to_string(ntohs(addr.sin_port)).c_str();
//   }