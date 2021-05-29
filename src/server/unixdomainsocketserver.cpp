#include "unixdomainsocketserver.hpp"
#include <iostream>
#include <cstring>
#include <sys/un.h>
#include <unistd.h>

using namespace CodecServer;

void UnixDomainSocketServer::readConfig(std::map<std::string, std::string> config) {
    if (config.find("socket") != config.end()) {
        socket_path = config["socket"];
    }
}

int UnixDomainSocketServer::bind() {
    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), socket_path.length());
    return ::bind(sock, (sockaddr*) &addr, sizeof(addr));
}

int UnixDomainSocketServer::getSocket() {
    return socket(AF_UNIX, SOCK_STREAM, 0);
}

void UnixDomainSocketServer::run() {
    SocketServer::run();
    unlink(socket_path.c_str());
}