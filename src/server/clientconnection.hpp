#pragma once

#include "connection.hpp"
#include "session.hpp"
#include <thread>
#include <google/protobuf/message.h>

namespace CodecServer {

    class ClientConnection: public Connection {
        public:
            ClientConnection(int sock);
        private:
            bool handshake();
            void loop();
            void read();
            bool run = true;
            Session* session = nullptr;
    };

}