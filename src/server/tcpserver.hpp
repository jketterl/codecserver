#pragma once

#include "socketserver.hpp"
#include <sys/socket.h>

namespace CodecServer {

    class TcpServer: public SocketServer {
        public:
            TcpServer(): SocketServer() {}
            void readConfig(const std::map<std::string, std::string>& config) override;
        protected:
            std::string bindAddr;
            unsigned short port = 1073;
    };

    class Tcp4Server: public TcpServer {
        public:
            Tcp4Server(): TcpServer() {
                bindAddr = "0.0.0.0";
            }
            int bind() override;
            int getSocket() override;
    };

    class Tcp6Server: public TcpServer {
        public:
            Tcp6Server(): TcpServer() {
                bindAddr = "::";
            }
            int bind() override;
            int getSocket() override;
    };

}