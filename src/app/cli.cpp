#include "cli.hpp"
#include "handshake.pb.h"
#include <sys/socket.h>
#include <sys/un.h>
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

    void* buffer = malloc(1024);
    int rc = recv(sock, buffer, 1024, 0);
    if (rc == -1) {
        std::cerr << "handshake receive failure\n";
        return 1;
    }

    Handshake handshake;
    handshake.ParseFromArray(buffer, rc);

    std::cout << "received handshake from " << handshake.servername() << "\n";

    if (handshake.serverversion() != VERSION) {
        std::cerr << "server version mismatch\n";
        return 1;
    }

    return 0;
}