#include "socketserver.hpp"
#include "clientconnection.hpp"
#include <thread>
#include <sys/socket.h>

using namespace CodecServer;

SocketServer::~SocketServer() {
    stop();
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