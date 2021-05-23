#include "device.hpp"
#include "registry.hpp"
#include "dvsticksession.hpp"
#include "protocol.hpp"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <termios.h>

namespace DvStick {

using namespace Protocol;

Device::Device(std::string tty, unsigned int baudRate) {
    open(tty, baudRate);
    init();
}

std::vector<std::string> Device::getCodecs() {
    return { "ambe" };
}

CodecServer::Session* Device::startSession(CodecServer::Request* request) {
    for (int i = 0; i < channelState.size(); i++) {
        // TODO lock?
        if (!channelState[i]) {
            std::cerr << "starting new session on channel " << i << "\n";
            channelState[i] = true;
            return new DvStickSession(this, i);
        }
    }
    return nullptr;
}

void Device::releaseChannel(unsigned char channel) {
    channelState[channel] = false;
}

void Device::open(std::string ttyname, unsigned int baudRate) {
    fd = ::open(ttyname.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
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
    channelState = {false, false, false};

    (new RateTRequest(0, 33))->writeTo(fd);

    response = Packet::receiveFrom(fd);
    if (response == nullptr) {
        std::cerr << "no response\n";
        return;
    }

    RateTResponse* rateT = dynamic_cast<RateTResponse*>(response);
    if (rateT == nullptr) {
        std::cerr << "unexpected response from stick\n";
        return;
    }

    if (rateT->getResult() != 0) {
        std::cerr << "error setting rate: " << rateT->getResult() << "\n";
        return;
    }

}

size_t Device::decode(unsigned char channel, char* input, char* output, size_t size) {
    int processed = 0;
    int collected = 0;
    while (processed < size / 9) {
        (new ChannelPacket(channel, input + processed * 9, 9))->writeTo(fd);
        Packet* response = Packet::receiveFrom(fd);
        if (response == nullptr) {
            std::cerr << "no response\n";
            return 0;
        }

        SpeechPacket* speech = dynamic_cast<SpeechPacket*>(response);
        if (speech == nullptr) {
            std::cerr << "response is not speech\n";
            return 0;
        }
        collected += speech->getSpeechData(output + processed * 320);

        processed += 1;
    }
    return collected;
}

}