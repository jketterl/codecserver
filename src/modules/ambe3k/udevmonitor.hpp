#pragma once

#include <libudev.h>

namespace Ambe3K::Udev {
    class Monitor {
        public:
            explicit Monitor();
        private:
            void run();
    };
}