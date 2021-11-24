#include "ambe3kdevice.hpp"
#include "registry.hpp"
#include "ambe3ksession.hpp"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>

namespace Ambe3K {

using namespace Protocol;

using CodecServer::DeviceException;

Device::Device(std::string tty, unsigned int baudRate) {
    open(tty, baudRate);
    init();
    queue = new BlockingQueue<Ambe3K::Protocol::Packet>(10);
    worker = new QueueWorker(this, fd, queue);
}

Device::~Device() {
    delete worker;
    delete queue;
    for (Channel* c: channels) {
        delete c;
    }
    ::close(fd);
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
            auto* session = new Ambe3KSession(channel);
            try {
                session->renegotiate(request->settings());
                return session;
            } catch (std::invalid_argument&) {
                std::cerr << "invalid or unsupported channel index\n";
            }
            session->end();
            delete session;
            return nullptr;
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

    auto* ready = dynamic_cast<ControlPacket*>(response);
    if (ready == nullptr) {
        delete response;
        throw DeviceException("unexpected response from stick");
    }
    if (!ready->hasReadyField()) {
        delete response;
        throw DeviceException("device is not ready after reset");
    }
    delete ready;

    ProdIdRequest prodIdRequest;
    prodIdRequest.writeTo(fd);

    response = Packet::receiveFrom(fd);
    if (response == nullptr) {
        throw DeviceException("device did not respond to product id request");
    }

    auto* prodid = dynamic_cast<ControlPacket*>(response);
    if (prodid == nullptr) {
        delete response;
        throw DeviceException("unexpected response from stick");
    }
    if (!prodid->hasProductId()) {
        delete response;
        throw DeviceException("device did not respond with product id as expected");
    }

    VersionStringRequest versionStringRequest;
    versionStringRequest.writeTo(fd);

    response = Packet::receiveFrom(fd);
    if (response == nullptr) {
        throw DeviceException("device did not respond to version string request");
    }

    auto* versionString = dynamic_cast<ControlPacket*>(response);
    if (versionString == nullptr) {
        delete response;
        throw DeviceException("unexpected response from stick");
    }
    if (!versionString->hasVersionString()) {
        throw DeviceException("device did not response with version string as expected");
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
    auto speech = dynamic_cast<SpeechPacket*>(packet);
    if (speech != nullptr) {
        // fallback for single-channel chips that don't respond with a channel number
        unsigned int channelNo = 0;
        if (speech->hasChannel()) {
            channelNo = speech->getChannel();
            assert(channelNo < channels.size());
        }
        channels[channelNo]->receive(speech);
        return;
    }
    auto control = dynamic_cast<ControlPacket*>(packet);
    if (control != nullptr) {
        if (control->hasChannel()) {
            std::cerr << "channel " << +control->getChannel() << ": ";
        }
        if (control->hasInitResponse()) {
            std::cerr << "init response received; ";
        }
        if (control->hasRateTResponse()) {
            std::cerr << "RateT response received; ";
        }
        if (control->hasRatePResponse()) {
            std::cerr << "RateP response received; ";
        }
        std::cerr << "\n";
        delete control;
        return;
    }
    delete packet;
    std::cerr << "unexpected packet received from stick\n";
}

void Device::onQueueError(std::string message) {
    std::cerr << "ambe3k queue worker reported error: " << message << "\n";
    CodecServer::Registry::get()->unregisterDevice(this);
}

}