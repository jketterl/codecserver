#pragma once

#include "socketserver.hpp"

namespace CodecServer {

    class TcpServer: public SocketServer {
        public:
            TcpServer(std::map<std::string, std::string> config);
        private:
            unsigned short port = 1073;
            std::string bindAddr = "0.0.0.0";
    };

}