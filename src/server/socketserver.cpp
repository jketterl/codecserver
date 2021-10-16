#include "socketserver.hpp"
#include "clientconnection.hpp"
#include <thread>
#include <iostream>

using namespace CodecServer;

SocketServer::~SocketServer() {
    stop();
}

int SocketServer::setupSocket() {

    int result = 0;

    this->sock = getSocket();

    if (this->sock == -1) {
        std::cerr << "socket error: " << strerror(errno) << std::endl;
        result = -1;
    }

    if (result == 0) {
        int reuse = 1;
        if (setsockopt(this->sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
            std::cerr << "error setting socket options: " << strerror(errno) << std::endl;
            result = -1;
        }
    }

    if (result == 0) {
        int rc = this->bind();
        if (rc < 0) {
            if (rc == -1) {
                std::cerr << "bind error: " << strerror(errno) << std::endl;
            } else {
                std::cerr << "bind error: unknown" << std::endl;
            }
            result = -1;
        }
    }

    if (result == 0) {
        if (listen(this->sock, 1) == -1) {
            std::cerr << "listen error: " << strerror(errno) << std::endl;
            result = -1;
        }
    }

    return result;
}

void SocketServer::start() {
    thread = std::thread([this] {
        run();
    });
}

void SocketServer::run() {
    while (dorun) {
        int client_sock = accept(sock, NULL, NULL);
        if (client_sock > 0) {
            std::thread t([client_sock] {
                new ClientConnection(client_sock);
            });
            t.detach();
        }
    }
}

void SocketServer::stop() {
    dorun = false;
    // unlocks the call to accept();
    shutdown(sock, SHUT_RDWR);
    ::close(sock);
}

void SocketServer::join() {
    thread.join();
}
