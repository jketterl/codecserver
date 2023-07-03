#include "server.hpp"

int main (int argc, char** argv) {
    auto server = new CodecServer::Server();
    int rc = server->main(argc, argv);
    delete server;
    return rc;
}