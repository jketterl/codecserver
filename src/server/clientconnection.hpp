#pragma once

#include "connection.hpp"
#include "session.hpp"
#include "proto/data.pb.h"
#include "proto/request.pb.h"
#include "proto/check.pb.h"
#include <thread>
#include <google/protobuf/message.h>
#include <google/protobuf/any.pb.h>

namespace CodecServer {

    class ClientConnection: public Connection {
        public:
            explicit ClientConnection(int sock);
        private:
            void handshake();
            void loop();
            void read();
            template <typename T> bool checkMessageType(google::protobuf::Any* message);
            void processMessage(ChannelData* data);
            void processMessage(SpeechData* data);
            void processMessage(Renegotiation* data);
            void processMessage(Request* request);
            void processMessage(Check* check);
            void startSession();
            void stopSession();
            bool run = true;
            Session* session = nullptr;
            std::thread* reader = nullptr;
    };

}