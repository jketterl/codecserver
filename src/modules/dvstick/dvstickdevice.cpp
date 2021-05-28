#include "device.hpp"
#include "registry.hpp"
#include "dvsticksession.hpp"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <thread>
#include <stdexcept>

namespace DvStick {

using namespace Protocol;

Device::Device(std::string tty, unsigned int baudRate) {
    queue = new BlockingQueue<DvStick::Protocol::Packet*>(10);
    worker = new QueueWorker(this, queue);
    open(tty, baudRate);
    init();
}

Device::~Device() {
    delete worker;
    delete queue;
    for (Channel* c: channels) {
        delete c;
    }
}

std::vector<std::string> Device::getCodecs() {
    return { "ambe" };
}

CodecServer::Session* Device::startSession(CodecServer::proto::Request* request) {
    for (Channel* channel: channels) {
        // TODO lock?
        if (!channel->isBusy()) {
            std::cerr << "starting new session on channel " << +channel->getIndex() << "\n";
            channel->reserve();
            DvStickSession* session = new DvStickSession(channel);
            try {
                session->renegotiate(request->settings());
                return session;
            } catch (std::invalid_argument) {
                std::cerr << "invalid or unsupported channel index\n";
                return nullptr;
            }
            session->end();
            delete session;
        }
    }
    return nullptr;
}

void Device::open(std::string ttyname, unsigned int baudRate) {
    speed_t baud = convertBaudrate(baudRate);

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

    if (cfsetospeed(&tty, baud) != 0) {
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

speed_t Device::convertBaudrate(unsigned int baud) {
    switch (baud) {
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
        case 460800:
            return B460800;
        case 500000:
            return B500000;
        case 576000:
            return B576000;
        case 921600:
            return B921600;
        case 1000000:
            return B1000000;
        case 1152000:
            return B1152000;
        case 1500000:
            return B1500000;
        case 2000000:
            return B2000000;
        case 2500000:
            return B2500000;
        case 3000000:
            return B3000000;
        case 3500000:
            return B3500000;
        case 4000000:
            return B4000000;
        default:
            throw std::runtime_error("invalid baud rate");
    }
}

void Device::init() {
    ResetPacket reset;
    reset.writeTo(fd);

    Packet* response = Packet::receiveFrom(fd);
    if (response == nullptr) {
        std::cerr << "no response\n";
        return;
    }

    ReadyPacket* ready = dynamic_cast<ReadyPacket*>(response);
    if (ready == nullptr) {
        delete response;
        std::cerr << "unexpected response from stick\n";
        return;
    }
    delete ready;

    ProdIdRequest prodIdRequest;
    prodIdRequest.writeTo(fd);

    response = Packet::receiveFrom(fd);
    if (response == nullptr) {
        std::cerr << "no response\n";
        return;
    }

    ProdIdResponse* prodid = dynamic_cast<ProdIdResponse*>(response);
    if (prodid == nullptr) {
        delete response;
        std::cerr << "unexpected response from stick\n";
        return;
    }

    VersionStringRequest versionStringRequest;
    versionStringRequest.writeTo(fd);

    response = Packet::receiveFrom(fd);
    if (response == nullptr) {
        std::cerr << "no response\n";
        return;
    }

    VersionStringResponse* versionString = dynamic_cast<VersionStringResponse*>(response);
    if (versionString == nullptr) {
        delete response;
        std::cerr << "unexpected response from stick\n";
        return;
    }

    std::cerr << "Product id: " << prodid->getProductId() << "; Version: " << versionString->getVersionString() << "\n";

    createChannels(prodid->getProductId());

    delete prodid;
    delete versionString;
}

void Device::createChannels(std::string prodId) {
    if (prodId.substr(0, 8) == "AMBE3000") {
        std::cerr << "detected AMBE3000, creating one channel\n";
        createChannels(1);
        return;
    }

    if (prodId.substr(0, 8) == "AMBE3003") {
        std::cerr << "detected AMBE3003, creating three channels\n";
        createChannels(3);
        return;
    }

    std::cerr << "unknown product id, cannot create channels\n";
}

void Device::createChannels(unsigned int num_channels) {
    for (unsigned char i = 0; i < num_channels; i++) {
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
    RateTResponse* tResponse = dynamic_cast<RateTResponse*>(packet);
    if (tResponse != nullptr) {
        delete tResponse;
        std::cerr << "setup response received, result = " << +tResponse->getResult() << "\n";
        return;
    }
    RatePResponse* pResponse = dynamic_cast<RatePResponse*>(packet);
    if (pResponse != nullptr) {
        std::cerr << "setup response received, result = " << +pResponse->getResult() << "\n";
        delete pResponse;
        return;
    }
    delete packet;
    std::cerr << "unexpected packet received from stick\n";
}

Channel::Channel(Device* device, unsigned char index) {
    this->device = device;
    this->index = index;
}

Channel::~Channel() {
    release();
}

unsigned char Channel::getFramingBits() {
    switch (getCodecIndex()) {
        case 33:
            return 72;
        case 34:
            return 49;
        case 59:
            return 144;
    }
    return 0;
}

void Channel::process(char* input, size_t size) {
    device->writePacket(new ChannelPacket(index, input, getFramingBits()));
}

void Channel::receive(SpeechPacket* packet) {
    // TODO lock to make sure that queue doesn't go away after...
    if (queue == nullptr) {
        delete packet;
        std::cerr << "received packet while channel is not active. recent shutdown?\n";
        return;
    }
    try {
        queue->push(packet, false);
    } catch (QueueFullException) {
        std::cerr << "channel queue full. shutting down queue...\n";
        delete packet;
        delete queue;
        queue = nullptr;
    }
}

size_t Channel::read(char* output) {
    SpeechPacket* packet = queue->pop();
    if (packet == nullptr) {
        return 0;
    }
    size_t size = packet->getSpeechData(output);
    delete packet;
    return size;
}

unsigned char Channel::getIndex() {
    return index;
}

bool Channel::isBusy() {
    return busy;
}

void Channel::reserve() {
    busy = true;
    queue = new BlockingQueue<SpeechPacket*>(10);
}

void Channel::release() {
    if (queue != nullptr) {
        delete queue;
        queue=nullptr;
    }
    busy = false;
}

void Channel::setup(unsigned char codecIndex, unsigned char direction) {
    this->codecIndex = codecIndex;
    device->writePacket(new SetupRequest(index, codecIndex, direction));
}

void Channel::setup(short* cwds, unsigned char direction) {
    codecIndex = 0;
    device->writePacket(new SetupRequest(index, cwds, direction));
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

QueueWorker::~QueueWorker() {
    dorun = false;
}

void QueueWorker::run() {
    size_t in_progress = 0;
    while (dorun) {
        while ((!queue->empty() && in_progress < AMBE3K_FIFO_MAX_PENDING) || in_progress == 0) {
            Packet* packet = queue->pop();
            packet->writeTo(device->fd);
            delete packet;
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