#pragma once

#include <map>
#include <string>
#include <thread>
#include <sys/socket.h>

namespace CodecServer {

    class SocketServer {
        public:
            virtual ~SocketServer();
            virtual void readConfig(const std::map<std::string, std::string>& config) {};
            virtual int getSocket() = 0;
            virtual void bind() = 0;
            virtual void setupSocket();
            virtual void start();
            void stop();
            virtual void run();
            virtual void join();
        protected:
            int sock = -1;
            bool dorun = true;
        private:
            std::thread thread;
    };

}