#pragma once

#include "session.hpp"

namespace CodecServer {

    class Connection {
        public:
            Connection(int sock);
        private:
            void handshake();
            void loop();
            void close();
            bool run = true;
            Session* session;
            int sock;
    };

}