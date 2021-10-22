#pragma once

#include <map>
#include <string>
#include <thread>
#include <sys/socket.h>

namespace CodecServer {

    class SocketServer {
        public:
            SocketServer() {}
            virtual ~SocketServer();
            virtual void readConfig(std::map<std::string, std::string> config) {};
            virtual int getSocket() = 0;
            virtual int bind() = 0;
            virtual void setupSocket();
            virtual void start();
            virtual void stop();
            virtual void run();
            virtual void join();
        protected:
            int sock;
            bool dorun = true;
        private:
            std::thread thread;
    };

}
