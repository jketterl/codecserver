#pragma once

#include <google/protobuf/message.h>
#include <google/protobuf/any.pb.h>

using namespace google::protobuf::io;

namespace CodecServer {

    class Connection {
        public:
            Connection(int sock);
            void sendMessage(google::protobuf::Message* message);
            google::protobuf::Any* receiveMessage();
        protected:
            int sock;
    };


}