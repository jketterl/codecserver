#include "server.hpp"
#include "scanner.hpp"
#include "connection.hpp"
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <functional>
#include <csignal>

using namespace CodecServer;

std::function<void(int)> signal_callback_wrapper;
void signal_callback_function(int value) {
    signal_callback_wrapper(value);
}

int Server::main(int argc, char** argv) {
    std::cout << "Hello, I'm the codecserver.\n";

    signal_callback_wrapper = std::bind(&Server::handle_signal, this, std::placeholders::_1);
    std::signal(SIGINT, &signal_callback_function);
    std::signal(SIGTERM, &signal_callback_function);
    std::signal(SIGQUIT, &signal_callback_function);

    Scanner* scanner = new Scanner();
    scanner->scanModules();

    serve();

    return 0;
}

void Server::serve() {
    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    const char* socket_path = "/tmp/codecserver.sock";
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

    unlink(socket_path);
}

void Server::handle_signal(int signal) {
    std::cerr << "received signal: " << signal << "\n";
    stop();
}

void Server::stop() {
    run = false;
    ::close(sock);
}