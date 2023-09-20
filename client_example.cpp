#include "tcpsock.hpp"

skt::Socket client(49110, LOCALHOST, true);

int main(){
    client.connect();
    std::cout << "Connected to server" << std::endl;
    std::cout << "Server says: " << client.recv() << std::endl;
}