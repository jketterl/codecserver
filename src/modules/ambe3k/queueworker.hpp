#pragma once

#include "ambe3kdevice.hpp"
#include "protocol.hpp"

namespace Ambe3K {

    // forward declaration since those two classes are interdependent
    class Device;

    class QueueWorker {
        public:
            QueueWorker(Device* device, int fd, BlockingQueue<Ambe3K::Protocol::Packet*>* queue);
            ~QueueWorker();
        private:
            void run(int fd);
            Device* device;
            BlockingQueue<Ambe3K::Protocol::Packet*>* queue;
            bool dorun = true;
    };

}