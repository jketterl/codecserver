#include "connection.hpp"
#include "handshake.pb.h"
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
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    codecserver::Handshake handshake;
    handshake.set_servername("this is a test.");
    handshake.set_serverversion(VERSION);
    std::string message;
    handshake.SerializeToString(&message);
    std::cout << message << "\n";
    std::cout << "SIM handshake\n";
}

void Connection::loop() {
    std::cout << "SIM connection loop\n";
}

void Connection::close() {
    ::close(sock);
}