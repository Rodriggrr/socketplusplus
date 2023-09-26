#pragma once

#define ANY_ADDR  "0.0.0.0"
#define LOCALHOST "127.0.0.1"

#ifndef __linux__
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <iphlpapi.h>
    #define SO_WINDOWS
    typedef SOCKET sock_t;
#else 
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #define INVALID_SOCKET -1
    typedef int sock_t;
#endif

#include <string>
#include <stdexcept>

////////////////////////////////////////////////////
///
/// AUTHOR: Rodrigo Farinon; github.com/rodriggrr
/// CONTACT: rfarinon@alu.ufc.br
///
/// A simple TCP socket library for C++.
/// 
/// TOC:
/// skt - Namespace
/// skt::Node - Class
/// skt::Socket - Class
///
/// skt: getLastError() - Function
/// 
/// SOCKET METHODS:
/// skt::Socket::Socket() -----> Constructor
/// skt::Socket::accept() -----> skt::Node*   - Method
/// skt::Socket::connect() ----> void         - Method
/// skt::Socket::connectRef() -> skt::Node*   - Method
/// skt::Socket::send() -------> int          - Method
/// skt::Socket::recv() -------> std::string  - Method
/// skt::Socket::close() ------> void         - Method
/// skt::Socket::getSocket() --> sock_t       - Method
/// skt::Socket::getAddr() ----> sockaddr_in* - Method
///
/// NODE METHODS:
/// skt::Node::Node() ---------> Constructor
/// skt::Node::send() ---------> void         - Method
/// skt::Node::recv() ---------> std::string  - Method
/// skt::Node::getSock() ------> sock_t       - Method
/// skt::Node::getIp() --------> std::string  - Method
/// skt::Node::getIpStr() -----> std::string  - Method
/// skt::Node::getPort() ------> int          - Method
/// skt::Node::getAddr() ------> sockaddr_in* - Method
/// skt::Node::getAddrLen() ---> socklen_t*   - Method
///
/// FUNCTIONS:
/// skt::getLastError() -------> std::string  - Function
///
///////////////////////////////////////////////////

// Socket namespace;
/**
 * 
 * @brief Main namespace of the library.
 * 
 * @param LOCALHOST The localhost ip macro: 127.0.0.1, there's no better place than home.
 * @param ANY_ADDR  The any address ip macro: 0.0.0.0, used to bind to any address. 
 * @param sock_t    The socket file descriptor type. It is a int on linux, and a SOCKET on windows.
 * 
 * @note All classes and functions are inside this namespace.
 * 
 */
namespace skt {

// Node class.
/**
 * 
 * @brief ## `skt::Node`
 * 
 * 
 * @param sock_fd   The socket file descriptor of the node.
 * @param ip        The ip of the node.
 * @param port      The port of the node.
 * 
 * @note - Used to store data about a client: socket file descriptor, ip, etc.
 * @note - No parameters are required to create a node, but you will have to set them later.
 * @note - The destructor will close the socket file descriptor, this behavior can be changed by passing true to the constructor.
 * @note #### Examples:
 * @note `skt::Node node;` - Creates a node with no parameters.
 * @note `skt::Node node(true);` - Creates a node with no parameters, but the destructor will not close the socket file descriptor.
 * @note `skt::Node node(sock_fd, ip, port);` - Creates a node with the given parameters.
 *
 */
class Node {
    int port{};
    sock_t sock_fd{};
    std::string ip;
    sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    bool noCloseOnDestruct = false;
    char buffer[4096];

public:

    Node() = default;

    Node(bool noCloseOnDestruct) {
        this->noCloseOnDestruct = noCloseOnDestruct;
    }

    Node(sock_t sock_fd, std::string ip, int port) {
        this->sock_fd = sock_fd;
        this->ip = ip;
        this->port = port;
    }

    void setAddr() {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
    }

    ~Node() {
        if(noCloseOnDestruct) return;
        close(sock_fd);
    }

    operator sock_t() const { return sock_fd; }

    void setSock(int sock_fd) {
        this->sock_fd = sock_fd;
    }

    void setIp(std::string ip) {
        this->ip = ip;
    }

    void setPort(int port) {
        this->port = port;
    }

    void setAddrLen(size_t addrLen) {
        this->addrLen = addrLen;
    }

    sock_t getSock() {
        return sock_fd;
    }

    std::string getIp() {
        return ip;
    }

    std::string getIpStr() {
        return inet_ntoa(addr.sin_addr);
    }

    int getPort() {
        return port;
    }

    sockaddr_in* getAddr() {
        return &addr;
    }

    socklen_t* getAddrLen() {
        return &addrLen;
    }

    // Send data to the connected server.
    /**
     * 
     * @brief Every node is a client, so this method sends data to the server it is connected to.
     * 
     * @param data The data to be sent.
     * 
     * @throw `std::runtime_error()` if the data can't be sent.
     * 
     */
    void send(std::string data) {
        int sent = ::send(sock_fd, data.c_str(), data.size(), 0);
        if(sent < 0) {
            throw std::runtime_error("Error sending data");
        }
    }

    // Receives data from the connected server.
    /**
     * 
     * @brief As a client, this method receives data from the server, and stores it in the buffer.
     * 
     * @param buffer[] The buffer to store the data. If omitted, it will use the internal buffer.
     * 
     * @returns The data received, as a std::string. If the data bytes is 0, it will return an empty string.
     * 
     * @throw `std::runtime_error()` if the data can't be received.
     * 
     */
    std::string recv(char buffer[]=nullptr) {
        if(buffer == nullptr) { buffer = this->buffer; }

        int received = ::recv(sock_fd, buffer, 4096, 0);
        if(received < 0) {
            throw std::runtime_error("Error receiving data");
        }
        std::string data = std::string(buffer, received);
        if(received == 0) data = "";

        return data;
    }

};

// Socket class.
/**
 * 
 * @brief ## `skt::Socket`
 * 
 * @param port      The port that the socket will connect or bind. No default value, must be set.
 * @param ip        The ip that the socket will be bind, or to connected to. If not set, fallback to 0.0.0.0
 * @param isClient  Tell the socket if it is a client or not. If not set, fallback to false. Check note below.
 * @param reuseAddr Tell the socket if it should reuse the address or not. If not set, fallback to true.
 * @param queued    Tell the socket how many connections it should queue until droping requisitions. If not set, fallback to 10.
 * 
 * @throw `std::runtime_error()` if the socket can't be created. Sometimes it can be fixed, so you should try to treat it. Ex: bad port.
 * 
 * @note If the socket is a client, the ip and port will be used to connect to the server.
 * 
 * @note ##### As a Client:
 * @note - If client, MUST call `connect()` or `connectRef()` to connect to a server. `client.connect()`
 * @note - As a client, if connecting to localhost, `LOCALHOST` should be used instead of `ANY_ADDR` in windows.
 * @note ##### As a Server:
 * @note - After a unexpected error, the socket may be in a `TIME_WAIT` state, and even with `SO_REUSEADDR`, it may not be able to bind to the same port. So, if your server can't bind to the same port after a crash, or a Ctrl + C, you should wait a few seconds before trying to bind again, or change port.
 * @note #### Overloads:
 * @note `skt::Socket sock(49110);` - Creates a server socket, listening on port 49110, binded to 0.0.0.0
 * @note `skt::Socket sock(49110, true)` - Creates a client socket, connecting to 127.0.0.0 on port 49110.
 * @note `skt::Socket sock(49110, LOCALHOST, true);` - Creates a client socket, connecting to localhost on port 49110.
 * @note `skt::Socket sock(49110, ANY_ADDR, false, true, 10);` - Creates a server socket, listening on port 49110, with reuseAddr set to true, and queued set to 10.
 * 
 *
 */
class Socket {
    
    sock_t socket;
    std::string ip;
    sockaddr_in addr;
    socklen_t addrLen = sizeof(addr);
    bool reuseAddr;
    char buffer[4096];
    int port{}, queued{};
    bool isClient;

    sock_t createSocket(){
        sock_t sock;
    #ifdef SO_WINDOWS
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
    #endif
        sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            throw std::runtime_error("Error creating socket");
        }
        return sock;
    }

    void setAddr() {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        if(isClient)
            addr.sin_addr.s_addr = inet_addr(ip.c_str());
        else
            addr.sin_addr.s_addr = INADDR_ANY;
    }


    void bindSocket() {
        if(reuseAddr) {
            int opt = 1;

        #ifdef SO_WINDOWS
            if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt)) < 0) 
        #else
            if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        #endif
                throw std::runtime_error("Error setting socket options");
        }

        if(::bind(socket, (struct sockaddr *)&addr, addrLen) < 0) {
            throw std::runtime_error("Error binding socket to IP/Port");
        }
    }

    void listenSocket() {
        if(::listen(socket, queued) < 0) {
            throw std::runtime_error("Error listening to socket");
        }
    }

public:
    
    Socket(int port, bool isClient){
        this->ip = LOCALHOST;
        this->port = port;
        this->isClient = isClient;
        this->reuseAddr = true;
        this->queued = 10;
        this->socket = createSocket();
        setAddr();

        if(!isClient) {
            bindSocket();
            listenSocket();
        }

    }

    Socket(int port, std::string ip=ANY_ADDR, bool isClient=false, bool reuseAddr=true, int queued=10){
        this->ip = ip;
        this->port = port;
        this->isClient = isClient;
        this->socket = createSocket();
        this->queued = queued;
        setAddr();

        if(!isClient){
            bindSocket();
            listenSocket();         
        }
    }

    // Accepts a new connection.
    /**
     * 
     * @brief ## Accepts a new connection.
     * 
     * @note Hangs until a new connection is made.
     * 
     * @throw `std::runtime_error()` if the socket can't be accepted, or if it is a client socket.
     * 
     * @returns A pointer to a `skt::Node` object, containing the new socket file descriptor, ip and port. See skt::Node class.
     * 
     * @warning Put the returned pointer in a smart pointer to avoid memory leaks, or delete it manually.
     * 
     */
    Node* accept() {
        if(isClient) {
            throw std::runtime_error("Can't accept connections on a client socket");
        }

        Node* node = new Node();

        node->setSock(::accept(socket, (struct sockaddr *)node->getAddr(), node->getAddrLen()));
        node->setIp(inet_ntoa(node->getAddr()->sin_addr));
        node->setPort(port);

        if(node->getSock() < 0) {
            throw std::runtime_error("Error accepting connection");
        }
        return node;
    }

    // Connects to a server.
    /**
     * 
     * @brief ## Connects to a server.
     * 
     * @throw `std::runtime_error()` if the socket can't be connected, or it it is a server socket.
     * 
     * @returns A pointer to a `skt::Node` object, containing the new socket file descriptor, ip and port. See skt::Node class.
     * 
     * @warning - Put the returned pointer in a smart pointer to avoid memory leaks, or delete it manually.
     * @warning - IMPORTANT: on windows, as a client, if connecting to localhost, `ANY_ADDR` 0.0.0.0 will fail. Use `LOCALHOST` macro instead.
     * 
     */
    Node* connectRef(){
        if(!isClient) {
            throw std::runtime_error("Can't connect on a server socket");
        }

        Node* node = new Node();
        auto addr = node->getAddr();
        auto addrLen = node->getAddrLen();

        node->setSock(socket);
        node->setIp(ip);
        node->setPort(port);

        if(::connect(socket, (struct sockaddr *)addr, *addrLen) < 0) {
            throw std::runtime_error("Error connecting to server");
        }

        return node;
    }

    // Connects to a server.
    /**
     * 
     * @brief ## Connects to a server. 
     * 
     * @note Stores the socket file descriptor, ip and port in the socket object.
     * 
     * @throw `std::runtime_error()` if the socket can't be connected, or if it is a server socket.
     * 
     * @warning IMPORTANT: on windows, as a client, if connecting to localhost, `ANY_ADDR` 0.0.0.0 will fail. Use `LOCALHOST` macro instead.
     * 
     */
    void connect(){
        if(!isClient) {
            throw std::runtime_error("Can't connect on a server socket");
        }

        if(::connect(socket, (struct sockaddr *)&addr, addrLen) < 0) {
            throw std::runtime_error("Error connecting to server");
        }
    }

    // Sends data to the socket.
    /**
     * 
     * @brief ## Sends data to a socket.
     * 
     * @param data The data to be sent.
     * @param socket The socket to send the data. If a client, it can be omitted.
     * 
     * @return The number of bytes sent.
     * 
     * @throw `std::runtime_error()` if the data can't be sent.
     * 
     */
    int send(std::string data, sock_t socket=INVALID_SOCKET) {
        if(socket == INVALID_SOCKET) { socket = this->socket; }

        int sent = ::send(socket, data.c_str(), data.size(), 0);
        if(sent < 0) {
            throw std::runtime_error("Error sending data");
        }
        return sent;
    }

    // Receives data from the socket.
    /**
     * 
     * @brief ## Receives data from a socket.
     * 
     * 
     * @param socket The socket to receive the data. If a client, it can be omitted.
     * @param buffer[] The buffer to store the data. If omitted, it will use the internal buffer.
     * 
     * @return The data received.
     * 
     * @throw `std::runtime_error()` if the data can't be received.
     * 
     * @note Hangs until data is received.
     */
    std::string recv(sock_t socket=INVALID_SOCKET, char buffer[]=nullptr) {
        if(socket == INVALID_SOCKET) { socket = this->socket; }
        if(buffer == nullptr) { buffer = this->buffer; }

        int received = ::recv(socket, buffer, 4096, 0);
        if(received < 0) {
            throw std::runtime_error("Error receiving data");
        }
        return std::string(buffer, received);
    }

    // Closes the socket.
    /**
     * 
     * @brief ## Closes the socket.
     * 
     * @throw `std::runtime_error()` if the socket can't be closed.
     * 
     */
    void close() {

    #ifdef SO_WINDOWS
        if(closesocket(socket) < 0)
    #else
        if(::close(socket) < 0)
    #endif
            throw std::runtime_error("Error closing socket");
    
    }

    // Returns the socket file descriptor.
    sock_t getSocket() {
        return socket;
    }

    // Returns the address.
    sockaddr_in* getAddr() {
        return &addr;
    }
};

// Returns the last error.
std::string getLastError() {
#ifdef SO_WINDOWS
    return std::to_string(WSAGetLastError());
#else
    return std::to_string(errno);
#endif
}

}