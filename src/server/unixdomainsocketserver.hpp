#pragma once

#include "socketserver.hpp"
#include <sys/socket.h>

namespace CodecServer {

    class UnixDomainSocketServer: public SocketServer {
        public:
            UnixDomainSocketServer(): SocketServer() {}
            void readConfig(const std::map<std::string, std::string>& config) override;
            void clearSocket();
            int getSocket() override;
            void bind() override;
            void run() override;
        private:
            std::string socket_path = "/tmp/codecserver.sock";
    };

}