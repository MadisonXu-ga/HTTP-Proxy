#include "proxy.cpp"

int main(int argc, char *argv[]){
    const char * port = "1237";
    cout << "before proxy" << endl;
    Proxy myProxy(port);
    myProxy.makeDaemon();

    return 0;
}