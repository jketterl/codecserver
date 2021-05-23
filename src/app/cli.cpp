#include "config.h"
#include "cli.hpp"
#include "handshake.pb.h"
#include "request.pb.h"
#include "response.pb.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <google/protobuf/any.pb.h>

using namespace CodecServer;

int Cli::main(int argc, char** argv) {
    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    const char* socket_path = "/tmp/codecserver.sock";
    strncpy(addr.sun_path, socket_path, strlen(socket_path));

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "socket error\n";
        return 1;
    }

    if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        std::cerr << "connection failure\n";
        return 1;
    }

    Connection* connection = new Connection(sock);
    google::protobuf::Any* message = connection->receiveMessage();
    if (!message->Is<CodecServer::proto::Handshake>()) {
        std::cerr << "unexpected message\n";
        return 1;
    }

    CodecServer::proto::Handshake handshake;
    message->UnpackTo(&handshake);

    std::cerr << "received handshake from " << handshake.servername() << "\n";

    if (handshake.serverversion() != VERSION) {
        std::cerr << "server version mismatch\n";
        return 1;
    }

    CodecServer::proto::Request request;
    request.set_codec("ambe");
    request.set_direction(CodecServer::proto::Request_Direction_DECODE);
    connection->sendMessage(&request);

    message = connection->receiveMessage();
    if (!message->Is<CodecServer::proto::Response>()) {
        std::cerr << "response error\n";
        return 1;
    }

    CodecServer::proto::Response response;
    message->UnpackTo(&response);

    if (response.result() != CodecServer::proto::Response_Status_OK) {
        std::cerr << "server replied with error, message: " << response.message() << "\n";
        return 1;
    }

    std::cerr << "server response OK, start decoding!\n";

    void* in_buf = malloc(9);
    void* buffer = malloc(BUFFER_SIZE);

    while (run) {
        size_t size = fread(in_buf, sizeof(char), 9, stdin);
        if (size <= 0) {
            run = false;
            break;
        }
        send(sock, in_buf, size, MSG_NOSIGNAL);
        size = recv(sock, buffer, 320, 0);
        if (size <= 0) {
            run = false;
            break;
        }
        fwrite(buffer, sizeof(char), size, stdout);
        fflush(stdout);
    }

    free(in_buf);
    free(buffer);

    ::close(sock);

    return 0;
}