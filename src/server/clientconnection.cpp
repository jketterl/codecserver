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

void ClientConnection::close() {
    ::close(sock);
}