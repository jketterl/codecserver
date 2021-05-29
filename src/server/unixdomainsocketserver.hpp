#pragma once

#include "socketserver.hpp"
#include <sys/socket.h>

namespace CodecServer {

    class UnixDomainSocketServer: public SocketServer {
        public:
            UnixDomainSocketServer(): SocketServer() {}
            void readConfig(std::map<std::string, std::string> config) override;
            int getSocket() override;
            int bind() override;
            void run() override;
        private:
            std::string socket_path = "/tmp/codecserver.sock";
    };

}