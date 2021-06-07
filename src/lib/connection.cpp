#include "connection.hpp"
#include "proto/data.pb.h"
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

Connection::~Connection() {
    delete inStream;
}

bool Connection::sendMessage(google::protobuf::Message* message) {
    google::protobuf::Any* any = new google::protobuf::Any();
    any->PackFrom(*message);
    FileOutputStream* fos = new FileOutputStream(sock);
    CodedOutputStream* os = new CodedOutputStream(fos);
#if GOOGLE_PROTOBUF_VERSION < 3006001
    uint64_t size = any->ByteSize();
#else
    uint64_t size = any->ByteSizeLong();
#endif
    os->WriteVarint64(size);
    bool rc = any->SerializeToCodedStream(os);
    delete any;
    delete os;
    delete fos;
    return rc;
}

google::protobuf::Any* Connection::receiveMessage() {
    CodedInputStream* is = new CodedInputStream(inStream);
    uint64_t size;
    is->ReadVarint64(&size);
    if (size == 0) {
        return nullptr;
    }
    CodedInputStream::Limit l = is->PushLimit(size);
    google::protobuf::Any* any = new google::protobuf::Any();
    bool rc = any->ParseFromCodedStream(is);
    while (!rc && !is->ConsumedEntireMessage()) {
        rc = any->MergeFromCodedStream(is);
    }
    is->PopLimit(l);
    delete is;
    if (!rc) {
        return nullptr;
    }
    return any;
}

bool Connection::sendChannelData(char* bytes, size_t size) {
    ChannelData* data = new ChannelData();
    data->set_data(std::string(bytes, size));
    bool rc = sendMessage(data);
    delete data;
    return rc;
}

bool Connection::sendSpeechData(char* bytes, size_t size) {
    SpeechData* data = new SpeechData();
    data->set_data(std::string(bytes, size));
    bool rc = sendMessage(data);
    delete data;
    return rc;
}

void Connection::close() {
    ::close(sock);
}

bool Connection::isCompatible(std::string version) {
    return version == VERSION;
}