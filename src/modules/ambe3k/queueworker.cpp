#include "queueworker.hpp"
#include <thread>

using namespace Ambe3K;
using namespace Ambe3K::Protocol;

QueueWorker::QueueWorker(Device* device, int fd, BlockingQueue<Packet*>* queue) {
    this->device = device;
    this->queue = queue;
    std::thread thread = std::thread( [this, fd] {
        run(fd);
    });
    thread.detach();
}

QueueWorker::~QueueWorker() {
    dorun = false;
}

void QueueWorker::run(int fd) {
    size_t in_progress = 0;
    while (dorun) {
        while ((!queue->empty() && in_progress < AMBE3K_FIFO_MAX_PENDING) || in_progress == 0) {
            Packet* packet = queue->pop();
            if (packet == nullptr) {
                device->onQueueError("queue returned a null pointer, so assuming queue was shut down. shutting down worker\n");
                dorun = false;
                return;
            }
            packet->writeTo(fd);
            delete packet;
            in_progress += 1;
            //std::cerr << "  sent one packet, in_progress is now: " << in_progress << "\n";
        }

        do {
            Packet* response = Packet::receiveFrom(fd);
            if (response == nullptr) {
                device->onQueueError("no response from device");
                dorun = false;
                return;
            }
            device->receivePacket(response);
            in_progress -= 1;
            //std::cerr << "  received one packet, in_progress is now: " << in_progress << "\n";
        } while (in_progress > 0 && queue->empty());
    }
}