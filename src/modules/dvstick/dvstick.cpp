#include "dvstick.hpp"
#include "protocol.hpp"
#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <termios.h>

namespace DvStick {

using namespace Protocol;

DvStick::DvStick(std::string tty, unsigned int baudRate) {
    open(tty, baudRate);
    init();
}

void DvStick::open(std::string ttyname, unsigned int baudRate) {
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

void DvStick::init() {
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

}

}