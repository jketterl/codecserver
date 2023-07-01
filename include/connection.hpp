#pragma once

#include <google/protobuf/message.h>
#include <google/protobuf/any.pb.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <stdexcept>

using namespace google::protobuf::io;

namespace CodecServer {

    class Connection {
        public:
            explicit Connection(int sock);
            virtual ~Connection();
            bool sendMessage(google::protobuf::Message* message);
            google::protobuf::Any* receiveMessage();
            bool sendChannelData(char* buffer, size_t size);
            bool sendSpeechData(char* buffer, size_t size);
            bool isCompatible(uint32_t protocolVersion);
        private:
            int sock;
            FileInputStream* inStream;
    };

    class ConnectionException: public std::runtime_error {
        public:
            explicit ConnectionException(const std::string msg): std::runtime_error(msg) {}
    };

    class HandshakeException: public ConnectionException {
        public:
            explicit HandshakeException(const std::string msg): ConnectionException(msg) {}
    };

}