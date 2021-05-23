#include "connection.hpp"
#include <unistd.h>
#include <netinet/in.h>

#define BUFFER_SIZE 65536

using namespace CodecServer;

Connection::Connection(int sock) {
    this->sock = sock;
}

void Connection::sendMessage(google::protobuf::Message* message) {
    google::protobuf::Any* any = new google::protobuf::Any();
    any->PackFrom(*message);
    size_t size = any->ByteSizeLong();
    void* buffer = malloc(size);
    any->SerializeToArray(buffer, size);
    send(sock, buffer, size, MSG_NOSIGNAL);
    free(buffer);
}

google::protobuf::Any* Connection::receiveMessage() {
    void* buffer = malloc(BUFFER_SIZE);
    google::protobuf::Any* any = new google::protobuf::Any();
    size_t size = recv(sock, buffer, BUFFER_SIZE, 0);
    if (size == -1) {
        std::cerr << "request failure: " << errno << "\n";
        // TODO throw an execption
        return nullptr;
    }
    any->ParseFromArray(buffer, size);
    free(buffer);
    return any;
}