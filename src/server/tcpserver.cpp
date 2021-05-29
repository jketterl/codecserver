#include "tcpserver.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

using namespace CodecServer;

void TcpServer::readConfig(std::map<std::string, std::string> config) {
    if (config.find("port") != config.end()) {
        port = stoul(config["port"]);
    }

    if (config.find("bind") != config.end()) {
        bindAddr = config["bind"];
    }
}

int Tcp4Server::bind() {
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(bindAddr.c_str());
    return ::bind(sock, (sockaddr*) &addr, sizeof(addr));
}

int Tcp6Server::bind() {
    sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port);
    inet_pton(AF_INET6, bindAddr.c_str(), &addr.sin6_addr);
    return ::bind(sock, (sockaddr*) &addr, sizeof(addr));
}

int Tcp4Server::getSocket() {
    return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

int Tcp6Server::getSocket() {
    return socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
}
