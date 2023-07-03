#include "unixdomainsocketserver.hpp"
#include <iostream>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace CodecServer;

void UnixDomainSocketServer::readConfig(const std::map<std::string, std::string>& config) {
    if (config.find("socket") != config.end()) {
        socket_path = config.at("socket");
    }
}

void UnixDomainSocketServer::clearSocket() {
    struct stat sb {};
    if (lstat(socket_path.c_str(), &sb) == -1) {
        // if the file doesn't exist, we don't need to clear it. everything's fine.
        if (errno == ENOENT) {
            return;
        }
        throw std::runtime_error("could not query socket status: " + std::string(strerror(errno)));
    }

    if ((sb.st_mode & S_IFMT) != S_IFSOCK) {
        throw std::runtime_error("file \"" + socket_path + "\" exists but is not a socket");
    }

    if (unlink(socket_path.c_str()) == -1) {
        throw std::runtime_error("failed to clear existing socket: " + std::string(strerror(errno)));
    }
}

void UnixDomainSocketServer::bind() {
    clearSocket();

    sockaddr_un addr {};
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), socket_path.length());
    if (::bind(sock, (sockaddr*) &addr, sizeof(addr)) != 0) {
        throw std::runtime_error("bind error: " + std::string(strerror(errno)));
    }

    // change permissions so that others can use the socket, too
    if (chmod(socket_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
        std::cerr << "error setting socket permissions: " << strerror(errno) << "\n";
    }
}

int UnixDomainSocketServer::getSocket() {
    return socket(AF_UNIX, SOCK_STREAM, 0);
}

void UnixDomainSocketServer::run() {
    SocketServer::run();
    unlink(socket_path.c_str());
}