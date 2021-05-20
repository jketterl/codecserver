#include "config.h"
#include "cli.hpp"
#include "handshake.pb.h"
#include "request.pb.h"
#include "response.pb.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>

using namespace CodecServer;

int Cli::main(int argc, char** argv) {
    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    const char* socket_path = "/tmp/codecserver.sock";
    strncpy(addr.sun_path, socket_path, strlen(socket_path));

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "socket error\n";
        return 1;
    }

    if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        std::cerr << "connection failure\n";
        return 1;
    }

    void* buffer = malloc(BUFFER_SIZE);
    int rc = recv(sock, buffer, BUFFER_SIZE, 0);
    if (rc == -1) {
        std::cerr << "handshake receive failure\n";
        return 1;
    }

    CodecServer::proto::Handshake handshake;
    handshake.ParseFromArray(buffer, rc);

    std::cerr << "received handshake from " << handshake.servername() << "\n";

    if (handshake.serverversion() != VERSION) {
        std::cerr << "server version mismatch\n";
        return 1;
    }
    free(buffer);

    CodecServer::proto::Request request;
    request.set_codec("ambe");
    request.set_direction(CodecServer::proto::Request_Direction_DECODE);

    size_t size = request.ByteSizeLong();
    buffer = malloc(size);
    request.SerializeToArray(buffer, size);
    send(sock, buffer, size, MSG_NOSIGNAL);

    free(buffer);

    buffer = malloc(BUFFER_SIZE);
    rc = recv(sock, buffer, BUFFER_SIZE, 0);
    if (rc == -1) {
        std::cerr << "response error\n";
        return 1;
    }

    CodecServer::proto::Response response;
    response.ParseFromArray(buffer, rc);

    if (response.result() != CodecServer::proto::Response_Status_OK) {
        std::cerr << "server replied with error, message: " << response.message() << "\n";
        return 1;
    }

    std::cerr << "server response OK, start decoding!\n";

    ::close(sock);

    return 0;
}