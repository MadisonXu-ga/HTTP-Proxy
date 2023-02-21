class Client {
  const char * hostname;
  const char * port;
  int fd;

  struct addrinfo host_info;
  struct addrinfo * host_info_list;

 public:
  Client() : hostname(NULL), port(NULL) {}
  Client(const char * hostname, const char * port) : hostname(NULL), port(port) {}
  ~Client() {
    free(host_info_list);
    close(fd);
  }
}