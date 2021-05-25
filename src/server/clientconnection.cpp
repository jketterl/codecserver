#include "config.h"
#include "clientconnection.hpp"
#include "handshake.pb.h"
#include "request.pb.h"
#include "response.pb.h"
#include "data.pb.h"
#include "registry.hpp"
#include <iostream>
#include <netinet/in.h>
#include <google/protobuf/any.pb.h>

using namespace CodecServer;
using namespace CodecServer::proto;

void ClientConnection::handshake() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    Handshake handshake;
    handshake.set_servername("codecserver");
    handshake.set_serverversion(VERSION);
    sendMessage(&handshake);

    google::protobuf::Any* message = receiveMessage();
    if (!message->Is<Request>()) {
        std::cerr << "invalid message\n";
        return;
    }
    Request request;
    message->UnpackTo(&request);
    std::cout << "client requests codec " << request.codec() << "\n";

    for (Device* device: Registry::get()->findDevices(request.codec())) {
        session = device->startSession(&request);
        if (session != nullptr) break;
    }

    Response response;
    if (session == nullptr) {
        response.set_result(Response_Status_ERROR);
        response.set_message("no device available");
        sendMessage(&response);
        return;
    }

    response.set_result(Response_Status_OK);
    FramingHint* framing = session->getFraming();
    if (framing != nullptr) {
        response.set_allocated_framing(framing);
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
        google::protobuf::Any* message = receiveMessage();
        if (message == nullptr) {
            run = false;
            close();
        } else if (message->Is<ChannelData>()) {
            ChannelData* data = new ChannelData();
            message->UnpackTo(data);
            std::string input = data->data();
            session->process((char*) input.c_str(), input.length());
            delete data;
        } else if (message->Is<Renegotiation>()) {
            Renegotiation* reneg = new Renegotiation();
            message->UnpackTo(reneg);
            Response* response = new Response();
            try {
                session->renegotiate(reneg->settings());
                response->set_result(Response_Status_OK);
            } catch (const std::exception&) {
                response->set_result(Response_Status_ERROR);
            }
            sendMessage(response);
            delete response;
            delete reneg;
        } else {
            std::cerr << "received unexpected message type\n";
        }

        delete message;

        // TODO SpeechData
    }


    session->end();
    reader.join();
}

void ClientConnection::read() {
    size_t size;
    char* output = (char*) malloc(BUFFER_SIZE);
    while (run) {
        size = session->read(output);
        SpeechData* data = new SpeechData();
        data->set_data(std::string(output, size));
        sendMessage(data);
        delete data;
    }
}