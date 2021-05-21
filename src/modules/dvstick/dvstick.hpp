#pragma once

#include <string>

#define BUFFER_SIZE 1024

namespace DvStick {

    class DvStick {
        public:
            DvStick(std::string tty, unsigned int baudRate);
        private:
            void open(std::string tty, unsigned int baudRate);
            void init();
            int fd;
    };

}