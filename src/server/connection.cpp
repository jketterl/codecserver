#include "connection.hpp"
#include "handshake.pb.h"
#include <thread>
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>

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
    Handshake handshake;
    handshake.set_servername("codecserver");
    handshake.set_serverversion(VERSION);
    size_t size = handshake.ByteSizeLong();
    void* buffer = malloc(size);
    handshake.SerializeToArray(buffer, size);
    send(sock, buffer, size, MSG_NOSIGNAL);
    free(buffer);
}

void Connection::loop() {
    std::cout << "SIM connection loop\n";
}

void Connection::close() {
    ::close(sock);
}