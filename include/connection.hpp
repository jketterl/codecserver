#pragma once

#include <google/protobuf/message.h>
#include <google/protobuf/any.pb.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace google::protobuf::io;

namespace CodecServer {

    class Connection {
        public:
            Connection(int sock);
            void sendMessage(google::protobuf::Message* message);
            google::protobuf::Any* receiveMessage();
            void sendChannelData(char* buffer, size_t size);
            void sendSpeechData(char* buffer, size_t size);
            void close();
        private:
            int sock;
            FileInputStream* inStream;
    };


}