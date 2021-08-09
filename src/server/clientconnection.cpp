#include "clientconnection.hpp"
#include "proto/handshake.pb.h"
#include "proto/response.pb.h"
#include "registry.hpp"
#include <iostream>
#include <netinet/in.h>

#define BUFFER_SIZE 65536

using namespace CodecServer;
using namespace CodecServer::proto;

ClientConnection::ClientConnection(int sock): Connection(sock) {
    try {
        handshake();
        loop();
    } catch (ConnectionException e) {
        std::cerr << "connection error: " << e.what() << "\n";
    }
    close();
    delete this;
}

void ClientConnection::handshake() {
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    Handshake handshake;
    handshake.set_servername("codecserver");
    handshake.set_serverversion(VERSION);
    if (!sendMessage(&handshake)) {
        throw ConnectionException("sending handshake failed");
    }
}

void ClientConnection::startSession() {
    if (session == nullptr) {
        return;
    }

    session->start();

    reader = new std::thread( [this] {
        read();
    });
}

void ClientConnection::stopSession() {
    if (session == nullptr) {
        return;
    }

    session->end();
    reader->join();

    delete session;
    session = nullptr;
    delete reader;
    reader = nullptr;
}

void ClientConnection::loop() {
    while (run) {
        google::protobuf::Any* message = receiveMessage();
        if (message == nullptr) {
            run = false;
            close();
        } else if (
            checkMessageType<Request>(message) ||
            checkMessageType<Check>(message) ||
            checkMessageType<ChannelData>(message) ||
            checkMessageType<SpeechData>(message) ||
            checkMessageType<Renegotiation>(message)
        ) {
            // we're good
        } else {
            std::cerr << "received unexpected message type\n";
        }

        delete message;
    }

    stopSession();
}

template <typename T>
bool ClientConnection::checkMessageType(google::protobuf::Any* message) {
    if (message->Is<T>()) {
        T* content = new T();
        message->UnpackTo(content);
        processMessage(content);
        return true;
    }
    return false;
}

void ClientConnection::processMessage(ChannelData* data) {
    if (session == nullptr) {
        std::cerr << "dropping incoming channel data since we don't have a decoding session\n";
    } else {
        std::string input = data->data();
        session->decode((char*) input.c_str(), input.length());
    }
    delete data;
}

void ClientConnection::processMessage(SpeechData* data) {
    if (session == nullptr) {
        std::cerr << "dropping incoming speech data since we don't have an encoding session\n";
    } else {
        std::string input = data->data();
        session->encode((char*) input.c_str(), input.length());
    }
    delete data;
}

void ClientConnection::processMessage(Renegotiation* reneg) {
    Response* response = new Response();
    try {
        session->renegotiate(reneg->settings());
        response->set_result(Response_Status_OK);
        FramingHint* framing = session->getFraming();
        if (framing != nullptr) {
            response->set_allocated_framing(framing);
        }
    } catch (const std::exception&) {
        response->set_result(Response_Status_ERROR);
    }
    bool sent = sendMessage(response);
    delete response;
    delete reneg;
    if (!sent) {
        throw ConnectionException("sending response failed");
    }
}

void ClientConnection::processMessage(Request* request) {
    std::cout << "client requests codec " << request->codec() << "\n";

    // stop existing session, if any
    stopSession();

    for (Device* device: Registry::get()->findDevices(request->codec())) {
        session = device->startSession(request);
        if (session != nullptr) break;
    }

    Response* response = new Response();

    if (session == nullptr) {
        response->set_result(Response_Status_ERROR);
        response->set_message("no device available");
    } else {
        response->set_result(Response_Status_OK);
        FramingHint* framing = session->getFraming();
        if (framing != nullptr) {
            response->set_allocated_framing(framing);
        }
        startSession();
    }

    bool sent = sendMessage(response);
    delete response;
    delete request;
    if (!sent) {
        throw ConnectionException("sending response failed");
    }
}

void ClientConnection::processMessage(Check* check) {
    std::cout << "check for codec: " << check->codec() << "\n";
    Response* response = new Response();

    if (!Registry::get()->findDevices(check->codec()).size()) {
        response->set_result(Response_Status_ERROR);
        response->set_message("no device available");
    } else {
        response->set_result(Response_Status_OK);
    }

    bool sent = sendMessage(response);
    delete response;
    delete check;
    if (!sent) {
        throw ConnectionException("sending response failed");
    }
}

void ClientConnection::read() {
    size_t size;
    char* output = (char*) malloc(BUFFER_SIZE);
    while (run) {
        size = session->read(output);
        if (size == 0) {
            run = false;
            break;
        }
        // TODO: we don't know if it's actually speech data, typing is lost in Session::read()
        SpeechData* data = new SpeechData();
        data->set_data(std::string(output, size));
        if (!sendMessage(data)) {
            run = false;
        }
        delete data;
    }
    free(output);
}