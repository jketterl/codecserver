#include "connection.hpp"
#include "data.pb.h"
#include <unistd.h>
#include <netinet/in.h>
#include <google/protobuf/io/coded_stream.h>

#define BUFFER_SIZE 65536

using namespace CodecServer;
using namespace CodecServer::proto;
using namespace google::protobuf::io;

Connection::Connection(int sock) {
    this->sock = sock;
    inStream = new FileInputStream(sock);
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
    CodedInputStream* is = new CodedInputStream(inStream);
    size_t size;
    is->ReadVarint64(&size);
    if (size == 0) {
        return nullptr;
    }
    CodedInputStream::Limit l = is->PushLimit(size);
    google::protobuf::Any* any = new google::protobuf::Any();
    any->ParseFromCodedStream(is);
    is->PopLimit(l);
    delete is;
    return any;
}

void Connection::sendChannelData(char* bytes, size_t size) {
    ChannelData* data = new ChannelData();
    data->set_data(std::string(bytes, size));
    sendMessage(data);
}

void Connection::sendSpeechData(char* bytes, size_t size) {
    SpeechData* data = new SpeechData();
    data->set_data(std::string(bytes, size));
    sendMessage(data);
}

void Connection::close() {
    ::close(sock);
}