#include "tcpserver.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

using namespace CodecServer;

void TcpServer::readConfig(const std::map<std::string, std::string>& config) {
    if (config.find("port") != config.end()) {
        port = stoul(config.at("port"));
    }

    if (config.find("bind") != config.end()) {
        bindAddr = config.at("bind");
    }
}

void Tcp4Server::bind() {
    sockaddr_in addr {};
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(bindAddr.c_str());
    if (::bind(sock, (sockaddr*) &addr, sizeof(addr)) != 0) {
        throw std::runtime_error("bind error: " + std::string(strerror(errno)));
    }
}

void Tcp6Server::bind() {
    sockaddr_in6 addr {};
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port);
    inet_pton(AF_INET6, bindAddr.c_str(), &addr.sin6_addr);
    if (::bind(sock, (sockaddr*) &addr, sizeof(addr)) != 0) {
        throw std::runtime_error("bind error: " + std::string(strerror(errno)));
    }
}

int Tcp4Server::getSocket() {
    return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

int Tcp6Server::getSocket() {
    return socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
}
