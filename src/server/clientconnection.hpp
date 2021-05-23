#pragma once

#include "connection.hpp"
#include "session.hpp"
#include <thread>
#include <google/protobuf/message.h>

namespace CodecServer {

    class ClientConnection: public Connection {
        public:
            ClientConnection(int sock): Connection(sock) {
                std::thread thread = std::thread( [this] {
                    handshake();
                    if (session != nullptr) {
                        loop();
                    }
                    close();
                });
                thread.detach();
            }
        private:
            void handshake();
            void loop();
            void close();
            bool run = true;
            Session* session;
    };

}