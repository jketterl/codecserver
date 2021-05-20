#pragma once

namespace CodecServer {

    class Connection {
        public:
            Connection(int sock);
        private:
            void handshake();
            void loop();
            void close();
            bool run = true;
            int sock;
    };

}