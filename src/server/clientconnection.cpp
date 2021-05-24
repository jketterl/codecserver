#include "config.h"
#include "clientconnection.hpp"
#include "handshake.pb.h"
#include "request.pb.h"
#include "response.pb.h"
#include "request.hpp"
#include "registry.hpp"
#include <iostream>
#include <netinet/in.h>
#include <google/protobuf/any.pb.h>

using namespace CodecServer;

void ClientConnection::handshake() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    CodecServer::proto::Handshake handshake;
    handshake.set_servername("codecserver");
    handshake.set_serverversion(VERSION);
    sendMessage(&handshake);

    google::protobuf::Any* message = receiveMessage();
    if (!message->Is<CodecServer::proto::Request>()) {
        std::cerr << "invalid message\n";
        return;
    }
    CodecServer::proto::Request request;
    message->UnpackTo(&request);
    std::cout << "client requests codec " << request.codec() << " and direction(s): ";
    std::map<std::string, std::string> args(request.args().begin(), request.args().end());
    unsigned char dir = 0;
    for (int direction: request.direction()) {
        if (direction == CodecServer::proto::Request_Direction_ENCODE) {
            dir |= DIRECTION_ENCODE;
            std::cout << "enccode ";
        } else if (direction == CodecServer::proto::Request_Direction_DECODE) {
            dir |= DIRECTION_DECODE;
            std::cout << "decode ";
        }
    }
    std::cout << "\n";
    Request* codecRequest = new Request(dir, args);

    for (Device* device: Registry::get()->findDevices(request.codec())) {
        session = device->startSession(codecRequest);
        if (session != nullptr) break;
    }

    CodecServer::proto::Response response;
    if (session != nullptr) {
        response.set_result(CodecServer::proto::Response_Status_OK);
    } else {
        response.set_result(CodecServer::proto::Response_Status_ERROR);
        response.set_message("no device available");
    }

    sendMessage(&response);
}

void ClientConnection::loop() {
    char* input = (char*) malloc(BUFFER_SIZE);
    session->start();

    std::thread reader = std::thread( [this] {
        read();
    });

    while (run) {
        size_t size = recv(sock, input, BUFFER_SIZE, 0);
        if (size <= 0) {
            run = false;
        }
        session->process(input, size);
    }


    session->end();
    reader.join();
}

void ClientConnection::read() {
    size_t size;
    char* output = (char*) malloc(BUFFER_SIZE);
    while (run) {
        size = session->read(output);
        send(sock, output, size, MSG_NOSIGNAL);
    }
}

void ClientConnection::close() {
    ::close(sock);
}