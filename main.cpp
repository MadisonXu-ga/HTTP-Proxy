#include "proxy.hpp"
#include "server.hpp"

int main(int argc, char * argv[]) {
  const char * port = "12345";
  std::cout << "before proxy" << std::endl;
  Proxy myProxy(port);
  myProxy.makeDaemon();

  return 0;
}