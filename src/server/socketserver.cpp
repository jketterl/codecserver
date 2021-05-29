#include "socketserver.hpp"
#include "clientconnection.hpp"
#include <thread>

using namespace CodecServer;

SocketServer::~SocketServer() {
    stop();
}

void SocketServer::setupSocket() {
    sock = getSocket();
    if (sock == -1) {
        std::cerr << "socket error: " << strerror(errno) << "\n";
        return;
    }

    int reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
        std::cerr << "error setting socket options: " << strerror(errno) << "\n";
        return;
    }

    if (bind() == -1) {
        std::cerr << "bind error: " << strerror(errno) << "\n";
        return;
    }

    if (listen(sock, 1) == -1) {
        std::cerr << "listen error: " << strerror(errno) << "\n";
        return;
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