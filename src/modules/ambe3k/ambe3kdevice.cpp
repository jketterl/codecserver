#include "ambe3kdevice.hpp"
#include "registry.hpp"
#include "ambe3ksession.hpp"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <thread>
#include <stdexcept>

namespace Ambe3K {

using namespace Protocol;

using CodecServer::DeviceException;

Device::Device(std::string tty, unsigned int baudRate) {
    open(tty, baudRate);
    init();
    queue = new BlockingQueue<Ambe3K::Protocol::Packet*>(10);
    worker = new QueueWorker(this, fd, queue);
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
            Ambe3KSession* session = new Ambe3KSession(channel);
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
        throw DeviceException("could not open TTY", errno);
    }

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        throw DeviceException("tcgetattr error", errno);
    }

    if (cfsetispeed(&tty, 0) != 0) {
        throw DeviceException("cfsetispeed error", errno);
    }

    if (cfsetospeed(&tty, baud) != 0) {
        throw DeviceException("cfsetospeed error", errno);
    }

    tty.c_lflag &= ~(ECHO | ECHOE | ICANON | IEXTEN | ISIG );
    tty.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON | IXOFF | IXANY);
    tty.c_cflag &= ~(CSIZE | CSTOPB | PARENB );
    tty.c_cflag |= CS8;
    tty.c_oflag &= ~(OPOST);
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 10;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        throw DeviceException("tcsetattr error", errno);
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
        throw DeviceException("device did not respond to reset");
    }

    ReadyPacket* ready = dynamic_cast<ReadyPacket*>(response);
    if (ready == nullptr) {
        delete response;
        throw DeviceException("unexpected response from stick");
    }
    delete ready;

    ProdIdRequest prodIdRequest;
    prodIdRequest.writeTo(fd);

    response = Packet::receiveFrom(fd);
    if (response == nullptr) {
        throw DeviceException("device did not respond to product id request");
    }

    ProdIdResponse* prodid = dynamic_cast<ProdIdResponse*>(response);
    if (prodid == nullptr) {
        delete response;
        throw DeviceException("unexpected response from stick");
    }

    VersionStringRequest versionStringRequest;
    versionStringRequest.writeTo(fd);

    response = Packet::receiveFrom(fd);
    if (response == nullptr) {
        throw DeviceException("device did not respond to version string request");
    }

    VersionStringResponse* versionString = dynamic_cast<VersionStringResponse*>(response);
    if (versionString == nullptr) {
        delete response;
        throw DeviceException("unexpected response from stick");
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

void Device::onQueueError(std::string message) {
    std::cerr << "ambe3k queue worker reported error: " << message << "\n";
    CodecServer::Registry::get()->unregisterDevice(this);
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
    short* cwds = getRateP();
    if (cwds != nullptr) {
        short dstar[] = {0x0130, 0x0763, 0x4000, 0x0000, 0x0000, 0x0048};
        if (std::memcmp(cwds, &dstar, sizeof(short) * 6) == 0) {
            return 72;
        }
    }
    return 0;
}

void Channel::decode(char* input, size_t size) {
    device->writePacket(new ChannelPacket(index, input, size * 8));
}

void Channel::encode(char* input, size_t size) {
    device->writePacket(new SpeechPacket(index, input, size / 2));
}

void Channel::receive(SpeechPacket* packet) {
    // TODO lock to make sure that queue doesn't go away after...
    if (queue == nullptr) {
        delete packet;
        std::cerr << "received packet while channel is inactive. recent shutdown?\n";
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

void Channel::receive(ChannelPacket* packet) {
    // TODO lock to make sure that queue doesn't go away after...
    if (queue == nullptr) {
        delete packet;
        std::cerr << "received packet while channel is inactive. recent shutdown?\n";
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
    while (true) {
        Packet* packet = queue->pop();
        if (packet == nullptr) {
            return 0;
        }

        // TODO: this loses the typing. callers of read() will not know if the response is channel or speech data.

        SpeechPacket* speech = dynamic_cast<SpeechPacket*>(packet);
        if (speech != nullptr) {
            size_t size = speech->getSpeechData(output);
            delete speech;
            return size;
        }

        ChannelPacket* channel = dynamic_cast<ChannelPacket*>(packet);
        if (channel != nullptr) {
            size_t size = channel->getChannelData(output);
            delete channel;
            return size;
        }

        std::cerr << "dropping one unexpected packet from channel queue\n";
        delete packet;
    }
}

unsigned char Channel::getIndex() {
    return index;
}

bool Channel::isBusy() {
    return busy;
}

void Channel::reserve() {
    busy = true;
    queue = new BlockingQueue<Packet*>(10);
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
    if (ratep != nullptr) delete(ratep);
    ratep = nullptr;
    device->writePacket(new SetupRequest(index, codecIndex, direction));
}

void Channel::setup(short* cwds, unsigned char direction) {
    codecIndex = 0;
    if (ratep != nullptr && ratep != cwds) delete(ratep);
    ratep = cwds;
    device->writePacket(new SetupRequest(index, cwds, direction));
}

unsigned char Channel::getCodecIndex() {
    return codecIndex;
}

short* Channel::getRateP() {
    return ratep;
}

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

}