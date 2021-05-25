#include "connection.hpp"
#include <unistd.h>
#include <netinet/in.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#define BUFFER_SIZE 65536

using namespace CodecServer;
using namespace google::protobuf::io;

Connection::Connection(int sock) {
    this->sock = sock;
}

void Connection::sendMessage(google::protobuf::Message* message) {
    google::protobuf::Any* any = new google::protobuf::Any();
    any->PackFrom(*message);
    FileOutputStream* fos = new FileOutputStream(sock);
    CodedOutputStream* os = new CodedOutputStream(fos);
    size_t size = any->ByteSizeLong();
    os->WriteVarint64(size);
    any->SerializeToCodedStream(os);
    delete any;
    delete os;
    delete fos;
}

google::protobuf::Any* Connection::receiveMessage() {
    FileInputStream* fis = new FileInputStream(sock);
    CodedInputStream* is = new CodedInputStream(fis);
    size_t size;
    is->ReadVarint64(&size);
    CodedInputStream::Limit l = is->PushLimit(size);
    google::protobuf::Any* any = new google::protobuf::Any();
    any->ParseFromCodedStream(is);
    is->PopLimit(l);
    delete is;
    delete fis;
    return any;
}