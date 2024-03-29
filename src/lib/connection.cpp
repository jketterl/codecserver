#include "connection.hpp"
#include "protocol.hpp"
#include "proto/data.pb.h"
#include <unistd.h>
#include <poll.h>
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
    close();
}

void Connection::close() {
    int s = sock;
    sock = -1;
    auto old = inStream;
    inStream = nullptr;
    if (s != -1) {
        ::shutdown(s, SHUT_RDWR);
        ::close(s);
    }
    delete old;
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
    // this whole block is mostly just about making sure that data is actually
    // available on the socket, while exiting early when any conditions arise on
    // the socket. Normally, this shouldn't be necessary, but external
    // observation shows that the CodecInputStream does not always seem to
    // handle socket errors and / or EOF events correctly.
    int s;
    while ((s = sock) != -1) {
        pollfd pfd = {
            .fd = s,
            .events = POLLIN
        };

        int rc = poll(&pfd, 1, 10000);
        if (rc == -1) {
            // poll failed
            return nullptr;
        }
        if (pfd.revents & POLLERR) {
            // something wrong about the socket
            return nullptr;
        }
        if (pfd.revents & POLLIN) {
            // we actually have some data
            break;
        }
    }

    auto stream = inStream;

    if (stream == nullptr) {
        return nullptr;
    }

    CodedInputStream* is = new CodedInputStream(stream);
    uint64_t size;
    if (!is->ReadVarint64(&size)) {
        delete is;
        return nullptr;
    }
    if (size == 0) {
        delete is;
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

bool Connection::isCompatible(uint32_t protocolVersion) {
    // this method should model any upwards / downwards compatibility in protocol versions
    // precompiler condition is for protection only, it's not necessary to list previous versions here.
#if PROTOCOL_VERSION == 1
    // version 0 did not explicitly exist, but protobuf fills the field with 0, which can be used to handle older versions.
    // version 1 introduced the protocol version field and is otherwise identical to the previous protocol.
    if (protocolVersion == 0 || protocolVersion == 1) return true;
#endif
    // an exact match should always pass
    return protocolVersion == PROTOCOL_VERSION;
}