#include "server.hpp"

int main (int argc, char** argv) {
    CodecServer::Server* server = new CodecServer::Server();
    int rc = server->main(argc, argv);
    delete server;
    return rc;
}