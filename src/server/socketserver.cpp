#include "socketserver.hpp"
#include "clientconnection.hpp"
#include <thread>
#include <iostream>

using namespace CodecServer;

SocketServer::~SocketServer() {
    stop();
}

void SocketServer::setupSocket() {
    sock = getSocket();
    if (sock == -1) {
        throw std::runtime_error("socket error: " + std::string(strerror(errno)));
    }

    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
        throw std::runtime_error("error setting socket options: " + std::string(strerror(errno)));
    }

    bind();

    if (listen(sock, 1) == -1) {
        throw std::runtime_error("listen error: " + std::string(strerror(errno)));
    }
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