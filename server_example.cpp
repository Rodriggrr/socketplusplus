#include "tcpsock.hpp"
#include <iostream>

int main(){

    skt::Node client;
    skt::Socket sock(49110, ANY_ADDR, false);
    try {
        client = *sock.accept();
    } catch (std::runtime_error& e) {
        std::cout << e.what() << ": " << skt::getLastError() << std::endl;
        return 1;
    }
    std::cout << "Client connected: " << client.getIpStr() << ":" << client.getPort() << std::endl;
    sock.send("Hello", client);
}