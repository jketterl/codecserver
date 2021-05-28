#include "server.hpp"
#include "scanner.hpp"
#include "clientconnection.hpp"
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <functional>
#include <csignal>
#include <getopt.h>

using namespace CodecServer;

std::function<void(int)> signal_callback_wrapper;
void signal_callback_function(int value) {
    signal_callback_wrapper(value);
}

int Server::main(int argc, char** argv) {
    if (!parseOptions(argc, argv)) {
        return 0;
    }

    std::cout << "Hello, I'm the codecserver.\n";

    signal_callback_wrapper = std::bind(&Server::handle_signal, this, std::placeholders::_1);
    std::signal(SIGINT, &signal_callback_function);
    std::signal(SIGTERM, &signal_callback_function);
    std::signal(SIGQUIT, &signal_callback_function);

    Scanner scanner;
    scanner.scanModules();

    serve();

    return 0;
}

bool Server::parseOptions(int argc, char** argv) {
    int c;
    static struct option long_options[] = {
        {"version", no_argument, NULL, 'v'},
        {"help", no_argument, NULL, 'h'},
        {"config", required_argument, NULL, 'c'},
        { NULL, 0, NULL, 0 }
    };
    while ((c = getopt_long(argc, argv, "vhc:", long_options, NULL)) != -1 ) {
        switch (c) {
            case 'v':
                printVersion();
                return false;
            case 'h':
                printUsage();
                return false;
            case 'c':
                config = std::string(optarg);
                break;
        }
    }

    return true;
}

void Server::printUsage() {
    std::cerr <<
        "codecserver version " << VERSION << "\n\n" <<
        "Usage: codecserver [options]\n\n" <<
        "Available options:\n" <<
        " -h, --help              show this message\n" <<
        " -v, --version           print version and exit\n" <<
        " -c, --config            path to config file (default: \"" << CONFIG << "\")\n";
}

void Server::printVersion() {
    std::cout << "codecserver version " << VERSION << "\n";
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
        int client_sock = accept(sock, NULL, NULL);
        if (client_sock > 0) {
            std::thread t([client_sock] {
                new ClientConnection(client_sock);
            });
            t.detach();
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