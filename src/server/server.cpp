#include "server.hpp"
#include "scanner.hpp"
#include "serverconfig.hpp"
#include "registry.hpp"
#include "unixdomainsocketserver.hpp"
#include "tcpserver.hpp"
#include <iostream>
#include <csignal>
#include <getopt.h>
#include <functional>

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
    std::signal(SIGPIPE, &signal_callback_function);

    ServerConfig config(configFile);

    Scanner scanner;
    scanner.scanModules();

    for (const std::string& driver: config.getDrivers()) {
        std::map<std::string, std::string> args = config.getDriverConfig(driver);
        Registry::get()->configureDriver(driver, args);
    }

    std::cout << "loading devices from configuration...\n";
    for (const std::string& device: config.getDevices()) {
        std::map<std::string, std::string> args = config.getDeviceConfig(device);
        Registry::get()->loadDeviceFromConfig(args);
    }

    std::cout << "auto-detecing devices...\n";
    Registry::get()->autoDetectDevices();
    std::cout << "device scan complete.\n";

    for (const std::string& type: config.getServers()) {
        std::map<std::string, std::string> args = config.getServerConfig(type);
        SocketServer* server = nullptr;
        if (type == "unixdomainsockets") {
            server = new UnixDomainSocketServer();
        } else if (type == "tcp4") {
            server = new Tcp4Server();
        } else if (type == "tcp" or type == "tcp6") {
            server = new Tcp6Server();
        }

        if (server == nullptr) {
            std::cerr << "unknown server type: \"" << type << "\"\n";
        } else {
            server->readConfig(args);
            server->setupSocket();
            servers.push_back(server);
        }
    }

    for (SocketServer* server: servers) {
        server->start();
    }

    for (SocketServer* server: servers) {
        server->join();
        delete server;
    }

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
                configFile = std::string(optarg);
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

void Server::handle_signal(int signal) {
    if (signal == SIGPIPE) {
        return;
    }
    std::cerr << "received signal: " << signal << "\n";
    stop();
}

void Server::stop() {
    for (SocketServer* server: servers) {
        server->stop();
    }
}