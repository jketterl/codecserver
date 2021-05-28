#pragma once

#include "socketserver.hpp"

namespace CodecServer {

    class UnixDomainSocketServer: public SocketServer {
        public:
            UnixDomainSocketServer(std::map<std::string, std::string> config);
            void run() override;
        private:
            std::string socket_path = "/tmp/codecserver.sock";
    };

}