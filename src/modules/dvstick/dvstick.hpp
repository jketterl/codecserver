#pragma once

#include <string>

#define BUFFER_SIZE 1024

namespace DvStick {

    class DvStick {
        public:
            DvStick(std::string tty, unsigned int baudRate);
            size_t decode(char* input, char* output, size_t size);
        private:
            void open(std::string tty, unsigned int baudRate);
            void init();
            int fd;
    };

}