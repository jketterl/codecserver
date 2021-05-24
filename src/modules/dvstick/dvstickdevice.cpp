#include "device.hpp"
#include "registry.hpp"
#include "dvsticksession.hpp"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <thread>

namespace DvStick {

using namespace Protocol;

Device::Device(std::string tty, unsigned int baudRate) {
    queue = new BlockingQueue<DvStick::Protocol::Packet*>(10);
    worker = new QueueWorker(this, queue);
    open(tty, baudRate);
    init();
}

std::vector<std::string> Device::getCodecs() {
    return { "ambe" };
}

CodecServer::Session* Device::startSession(CodecServer::Request* request) {
    for (Channel* channel: channels) {
        // TODO lock?
        if (!channel->isBusy()) {
            std::cerr << "starting new session on channel " << +channel->getIndex() << "\n";
            channel->reserve();
            try {
                std::string indexStr = request->getArg("index");
                std::cerr << "requested index: " << indexStr << "\n";
                unsigned char index = std::stoi(indexStr);
                channel->setup(index, request->getDirection());
                return new DvStickSession(channel);
            } catch (std::invalid_argument) {
                std::cerr << "invalid or unsupported channel index\n";
                return nullptr;
            }
        }
    }
    return nullptr;
}

void Device::open(std::string ttyname, unsigned int baudRate) {
    fd = ::open(ttyname.c_str(), O_RDWR | O_NOCTTY /*| O_SYNC*/);
    if (fd < 0) {
        std::cerr << "could not open TTY: " << strerror(errno) << "\n";
        return;
    }

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        std::cerr << "tcgetattr error: " << strerror(errno) << "\n";
        return;
    }

    if (cfsetispeed(&tty, 0) != 0) {
        std::cerr << "cfsetispeed error: " << strerror(errno) << "\n";
        return;
    }

    if (cfsetospeed(&tty, B921600) != 0) {
        std::cerr << "cfsetospeed error: " << strerror(errno) << "\n";
        return;
    }

    tty.c_lflag &= ~(ECHO | ECHOE | ICANON | IEXTEN | ISIG );
    tty.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON | IXOFF | IXANY);
    tty.c_cflag &= ~(CSIZE | CSTOPB | PARENB );
    tty.c_cflag |= CS8;
    tty.c_oflag &= ~(OPOST);
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 10;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        std::cerr << "tcsetattr error: " << strerror(errno) << "\n";
        return;
    }
}

void Device::init() {
    (new ResetPacket())->writeTo(fd);

    Packet* response = Packet::receiveFrom(fd);
    if (response == nullptr) {
        std::cerr << "no response\n";
        return;
    }

    ReadyPacket* ready = dynamic_cast<ReadyPacket*>(response);
    if (ready == nullptr) {
        std::cerr << "unexpected response from stick\n";
        return;
    }

    (new ProdIdRequest())->writeTo(fd);

    response = Packet::receiveFrom(fd);
    if (response == nullptr) {
        std::cerr << "no response\n";
        return;
    }

    ProdIdResponse* prodid = dynamic_cast<ProdIdResponse*>(response);
    if (prodid == nullptr) {
        std::cerr << "unexpected response from stick\n";
        return;
    }

    (new VersionStringRequest())->writeTo(fd);

    response = Packet::receiveFrom(fd);
    if (response == nullptr) {
        std::cerr << "no response\n";
        return;
    }

    VersionStringResponse* versionString = dynamic_cast<VersionStringResponse*>(response);
    if (versionString == nullptr) {
        std::cerr << "unexpected response from stick\n";
        return;
    }

    std::cerr << "Product id: " << prodid->getProductId() << "; Version: " << versionString->getVersionString() << "\n";

    // TODO check product id and initialize number of channels
    for (unsigned char i = 0; i < 3; i++) {
        channels.push_back(new Channel(this, i));
    }

}

void Device::writePacket(Packet* packet) {
    queue->push(packet);
}

void Device::receivePacket(Packet* packet) {
    SpeechPacket* speech = dynamic_cast<SpeechPacket*>(packet);
    if (speech != nullptr) {
        channels[speech->getChannel()]->receive(speech);
        return;
    }
    RateTResponse* response = dynamic_cast<RateTResponse*>(packet);
    if (response != nullptr) {
        std::cerr << "setup response received\n";
        return;
    }
    std::cerr << "response is not speech\n";
}

Channel::Channel(Device* device, unsigned char index) {
    this->device = device;
    this->index = index;
}

void Channel::process(char* input, size_t size) {
    int processed = 0;
    while (processed < size / 9) {
        device->writePacket(new ChannelPacket(index, input + processed * 9, 9));
        processed += 1;
    }
}

void Channel::receive(SpeechPacket* packet) {
    queue->push(packet);
}

size_t Channel::read(char* output) {
    SpeechPacket* packet = queue->pop();
    if (packet == nullptr) {
        return 0;
    }
    return packet->getSpeechData(output);
}

unsigned char Channel::getIndex() {
    return index;
}

bool Channel::isBusy() {
    return busy;
}

void Channel::reserve() {
    queue = new BlockingQueue<SpeechPacket*>(10);
    busy = true;
}

void Channel::release() {
    queue->shutdown();
    busy = false;
}

void Channel::setup(unsigned char codecIndex, unsigned char direction) {
    this->codecIndex = codecIndex;
    device->writePacket(new SetupRequest(index, codecIndex, direction));
}

unsigned char Channel::getCodecIndex() {
    return codecIndex;
}

QueueWorker::QueueWorker(Device* device, BlockingQueue<Packet*>* queue) {
    this->device = device;
    this->queue = queue;
    std::thread thread = std::thread( [this] {
        run();
    });
    thread.detach();
}

void QueueWorker::run() {
    size_t in_progress = 0;
    while (dorun) {
        while ((!queue->empty() && in_progress < DV3K_FIFO_MAX_PENDING) || in_progress == 0) {
            Packet* packet = queue->pop();
            packet->writeTo(device->fd);
            in_progress += 1;
            //std::cerr << "  sent one packet, in_progress is now: " << in_progress << "\n";
        }

        do {
            Packet* response = Packet::receiveFrom(device->fd);
            if (response == nullptr) {
                std::cerr << "no response\n";
                dorun = false;
                return;
            }
            device->receivePacket(response);
            in_progress -= 1;
            //std::cerr << "  received one packet, in_progress is now: " << in_progress << "\n";
        } while (in_progress > 0 && queue->empty());
    }
}

}