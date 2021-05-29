#include "unixdomainsocketserver.hpp"
#include <iostream>
#include <cstring>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace CodecServer;

void UnixDomainSocketServer::readConfig(std::map<std::string, std::string> config) {
    if (config.find("socket") != config.end()) {
        socket_path = config["socket"];
    }
}

int UnixDomainSocketServer::clearSocket() {
    struct stat sb;
    if (lstat(socket_path.c_str(), &sb) == -1) {
        // if the file doesn't exist, we don't need to clear it. everything's fine.
        if (errno == ENOENT) {
            return 0;
        }
        std::cerr << "could not query socket status: " << strerror(errno) << "\n";
        return -1;
    }

    if ((sb.st_mode & S_IFMT) != S_IFSOCK) {
        std::cerr << "file \"" << socket_path << "\" exists but is not a socket\n";
        return -2;
    }

    if (unlink(socket_path.c_str()) == -1) {
        std::cerr << "failed to clear existing socket: " << strerror(errno) << "\n";
        return -1;
    }

    return 0;
}

int UnixDomainSocketServer::bind() {
    int rc = clearSocket();
    if (rc != 0) {
        return rc;
    }

    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), socket_path.length());
    rc = ::bind(sock, (sockaddr*) &addr, sizeof(addr));

    // change permissions so that others can use the socket, too
    if (chmod(socket_path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
        std::cerr << "error setting socket permissions: " << strerror(errno) << "\n";
    };

    return rc;
}

int UnixDomainSocketServer::getSocket() {
    return socket(AF_UNIX, SOCK_STREAM, 0);
}

void UnixDomainSocketServer::run() {
    SocketServer::run();
    unlink(socket_path.c_str());
}