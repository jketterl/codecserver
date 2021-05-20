#include "config.h"
#include "connection.hpp"
#include "handshake.pb.h"
#include "request.pb.h"
#include "response.pb.h"
#include "request.hpp"
#include "registry.hpp"
#include <thread>
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>

using namespace CodecServer;

Connection::Connection(int sock) {
    this->sock = sock;
    std::thread thread = std::thread( [this] {
        handshake();
        if (session != nullptr) {
            loop();
        }
        close();
    });
    thread.detach();
}

void Connection::handshake() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    CodecServer::proto::Handshake handshake;
    handshake.set_servername("codecserver");
    handshake.set_serverversion(VERSION);

    size_t size = handshake.ByteSizeLong();
    void* buffer = malloc(size);
    handshake.SerializeToArray(buffer, size);
    send(sock, buffer, size, MSG_NOSIGNAL);
    free(buffer);

    buffer = malloc(BUFFER_SIZE);
    CodecServer::proto::Request request;
    size = recv(sock, buffer, BUFFER_SIZE, 0);
    if (size == -1) {
        std::cerr << "request failure: " << errno << "\n";
        // TODO throw an execption
        return;
    }
    request.ParseFromArray(buffer, size);
    free(buffer);

    std::cout << "client requests codec " << request.codec() << " and direction " << request.direction() << "\n";
    std::map<std::string, std::string> args(request.args().begin(), request.args().end());
    Direction dir;
    switch (request.direction()) {
        case CodecServer::proto::Request_Direction_ENCODE:
            dir = ENCODE;
            break;
        case CodecServer::proto::Request_Direction_DECODE:
            dir = DECODE;
            break;
    }
    Request* codecRequest = new Request(dir, args);

    for (Codec* codec: Registry::get()->findCodecs(request.codec())) {
        session = codec->startSession(codecRequest);
        if (session != nullptr) break;
    }

    CodecServer::proto::Response response;
    if (session != nullptr) {
        response.set_result(CodecServer::proto::Response_Status_OK);
    } else {
        response.set_result(CodecServer::proto::Response_Status_ERROR);
        response.set_message("no suitable codec found");
    }

    size = response.ByteSizeLong();
    buffer = malloc(size);
    response.SerializeToArray(buffer, size);
    send(sock, buffer, size, MSG_NOSIGNAL);
    free(buffer);
}

void Connection::loop() {
    char* input = (char*) malloc(BUFFER_SIZE);
    char* output = (char*) malloc(BUFFER_SIZE);
    std::cout << "entering loop\n";

    while (run) {
        size_t size = recv(sock, input, BUFFER_SIZE, 0);
        if (size <= 0) {
            run = false;
        }
        size = session->process(input, output, size);
        send(sock, output, size, MSG_NOSIGNAL);
    }
}

void Connection::close() {
    ::close(sock);
}