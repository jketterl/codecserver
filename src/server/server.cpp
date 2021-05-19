#include "server.hpp"
#include "scanner.hpp"
#include <iostream>

using namespace CodecServer;

int Server::main(int argc, char** argv) {
    std::cout << "Hello, I'm the codecserver.\n";

    Scanner* scanner = new Scanner();
    scanner->scanModules();

    return 0;
}