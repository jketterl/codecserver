#include "server.hpp"
#include "scanner.hpp"
#include "connection.hpp"
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>

using namespace CodecServer;

int Server::main(int argc, char** argv) {
    std::cout << "Hello, I'm the codecserver.\n";

    Scanner* scanner = new Scanner();
    scanner->scanModules();

    serve();

    return 0;
}

void Server::serve() {
    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    const char* socket_path = "codecserver.sock";
    strncpy(addr.sun_path, socket_path, strlen(socket_path));

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cout << "socket error\n";
        return;
    }

    if (bind(sock, (sockaddr*) &addr, sizeof(addr)) == -1) {
        std::cout << "bind error\n";
        return;
    }

    if (listen(sock, 1) == -1) {
        std::cout << "listen error\n";
        return;
    }

    while (run) {
        std::cout << "starting accept...\n";
        int client_sock = accept(sock, NULL, NULL);
        if (client_sock > 0) {
            new Connection(client_sock);
        }
    }
}