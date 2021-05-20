#include "connection.hpp"
#include <thread>
#include <iostream>
#include <unistd.h>

using namespace CodecServer;

Connection::Connection(int sock) {
    this->sock = sock;
    std::thread thread = std::thread( [this] {
        handshake();
        loop();
        close();
    });
    thread.detach();
}

void Connection::handshake() {
    std::cout << "SIM handshake\n";
}

void Connection::loop() {
    std::cout << "SIM connection loop\n";
}

void Connection::close() {
    ::close(sock);
}