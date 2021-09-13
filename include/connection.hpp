#pragma once

#include <google/protobuf/message.h>
#include <google/protobuf/any.pb.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <stdexcept>

using namespace google::protobuf::io;

namespace CodecServer {

    class Connection {
        public:
            Connection(int sock);
            ~Connection();
            bool sendMessage(google::protobuf::Message* message);
            google::protobuf::Any* receiveMessage();
            bool sendChannelData(char* buffer, size_t size);
            bool sendSpeechData(char* buffer, size_t size);
            bool isCompatible(std::string version);
        private:
            int sock;
            FileInputStream* inStream;
    };

    class ConnectionException: public std::runtime_error {
        public:
            ConnectionException(const std::string msg): std::runtime_error(msg) {}
    };

    class HandshakeException: public ConnectionException {
        public:
            HandshakeException(const std::string msg): ConnectionException(msg) {}
    };

}