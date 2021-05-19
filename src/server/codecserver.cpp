#include "server.hpp"

int main (int argc, char** argv) {
    CodecServer::Server* server = new CodecServer::Server();
    return server->main(argc, argv);
}