#include "tcpserver.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

using namespace CodecServer;

TcpServer::TcpServer(std::map<std::string, std::string> config): SocketServer(config) {
    if (config.find("port") != config.end()) {
        port = stoul(config["port"]);
    }

    if (config.find("bind") != config.end()) {
        bindAddr = config["bind"];
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(bindAddr.c_str());

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == -1) {
        std::cerr << "socket error\n";
        return;
    }

    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
        std::cerr << "error setting socket options\n";
    }

    if (bind(sock, (sockaddr*) &addr, sizeof(addr)) == -1) {
        std::cerr << "bind error\n";
        return;
    }

    if (listen(sock, 1) == -1) {
        std::cerr << "listen error\n";
        return;
    }
}
