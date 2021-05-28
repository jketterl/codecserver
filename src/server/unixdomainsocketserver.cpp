#include "unixdomainsocketserver.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using namespace CodecServer;

UnixDomainSocketServer::UnixDomainSocketServer(std::map<std::string, std::string> config): SocketServer(config) {
    if (config.find("socket") != config.end()) {
        socket_path = config["socket"];
    }

    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), socket_path.length());

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "socket error\n";
        return;
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

void UnixDomainSocketServer::run() {
    SocketServer::run();
    unlink(socket_path.c_str());
}