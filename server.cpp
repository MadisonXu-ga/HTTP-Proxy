#include "server.hpp"

int Server::createServer() {
  int status = init_addrinfo();  // -1
  status = createSocket();       // -2
  status = listenToSocket();     // -3 + -4

  return status;
}

int Server::init_addrinfo() {
  std::memset(&my_host_info, 0, sizeof(my_host_info));

  my_host_info.ai_family = AF_UNSPEC;
  my_host_info.ai_socktype = SOCK_STREAM;
  my_host_info.ai_flags = AI_PASSIVE;

  int status = getaddrinfo(hostname, port, &my_host_info, &my_host_info_list);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    // exit(EXIT_FAILURE);
    return -1;
  }
  // assign port randomly
  // struct sockaddr_in * addr_in = (struct sockaddr_in *)(my_host_info_list->ai_addr);
  // addr_in->sin_port = 0;
  return 1;
}

int Server::createSocket() {
  // create socket
  fd = socket(my_host_info_list->ai_family,
              my_host_info_list->ai_socktype,
              my_host_info_list->ai_protocol);
  if (fd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    // exit(EXIT_FAILURE);
    return -2;
  }
  return 1;
}

int Server::listenToSocket() {
  // bind socket
  int yes = 1;
  int status = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(fd, my_host_info_list->ai_addr, my_host_info_list->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error: cannot bind socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    // exit(EXIT_FAILURE);
    return -3;
  }

  // listen to socket
  status = listen(fd, 100);
  if (status == -1) {
    std::cerr << "Error: cannot listen on socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    // exit(EXIT_FAILURE);
    return -4;
  }
  return 1;
}

std::pair<int, std::string> Server::acceptConnection() {
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);

  int connect_fd = accept(fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (connect_fd == -1) {
    std::cerr << "Error: cannot accept connection on socket" << std::endl;
    // exit(-1);
    return std::make_pair(-1, std::string("ERROR"));
  }

  // get ip
  char ip[NI_MAXHOST];
  int status = getnameinfo(
      (struct sockaddr *)&socket_addr, socket_addr_len, ip, NI_MAXHOST, NULL, 0, 0);
  if (status != 0) {
    std::cerr << "Error: getnameinfo error" << std::endl;
    // exit(-1);
    return std::make_pair(-2, std::string("ERROR"));
  }

  std::string client_ip(ip);

  std::pair<int, std::string> result = std::make_pair(connect_fd, client_ip);
  return result;
}