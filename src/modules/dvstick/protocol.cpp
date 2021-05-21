#include "protocol.hpp"
#include <cstring>
#include <cassert>
#include <arpa/inet.h> // htons
#include <iostream>

using namespace DvStick::Protocol;

Packet::Packet(char* newData, size_t size) {
    // minimum size with parity
    assert(size >= 6);
    data = newData;
    payload = data + 4;
    dataSize = size;

    // start byte is constant
    data[0] = DV3K_START_BYTE;

    // need to fix endianness
    *(short*)(&data[1]) = htons(dataSize - 4);
}

Packet* Packet::parse(char* data, size_t size) {
    Packet* p = new Packet(data, size);
    if (p->verifyChecksum() != 0) {
        return nullptr;
    }
    if (p->getType() == DV3K_TYPE_CONTROL) {
        switch(p->payload[0]) {
            case DV3K_CONTROL_READY:
                return new ReadyPacket(data, size);
            case DV3K_CONTROL_PRODID:
                return new ProdIdResponse(data, size);
            case DV3K_CONTROL_VERSTRING:
                return new VersionStringResponse(data, size);
        }
    }
    return p;
}

void Packet::setType(char type) {
    data[3] = type;
}

char Packet::getType() {
    return data[3];
}

char Packet::getChecksum() {
    char parity = 0;
    for (int i = 0; i < dataSize - 2; i++) {
        parity ^= data[i + 1];
    }
    return parity;
}

void Packet::updateChecksum() {
    data[dataSize - 2] = DV3K_PARITY_BYTE;
    data[dataSize - 1] = getChecksum();
}

char Packet::verifyChecksum() {
    return getChecksum() - data[dataSize - 1];
}

void Packet::writeTo(int fd) {
    updateChecksum();
    write(fd, data, dataSize);
}

Packet* Packet::receiveFrom(int fd) {
    char* buf = (char*) malloc(1024);
    for (int i = 0; i < 100; i++) {
        char start_byte;
        read(fd, buf, 1);
        if (buf[0] == DV3K_START_BYTE) {
            break;
        }
    }

    if (buf[0] != DV3K_START_BYTE) {
        return nullptr;
    }

    short remain = 3;
    for (int i = 0; i < 100; i++) {
        remain -= read(fd, buf + (4 - remain), remain);
        if (remain == 0) {
            break;
        }
    }

    if (remain > 0) {
        return nullptr;
    }

    short payloadLength = ntohs(*((short*) &buf[1]));
    remain = payloadLength;
    buf = (char*) realloc(buf, remain + 4);

    for (int i = 0; i < 100; i++) {
        remain -= read(fd, buf + (4 + payloadLength - remain), remain);
        if (remain == 0) {
            break;
        }
    }

    return parse(buf, payloadLength + 4);
}

size_t Packet::getPayloadLength() {
    return dataSize - 6;
}

std::string ProdIdResponse::getProductId() {
    return std::string(payload + 1, getPayloadLength() - 1);
}

std::string VersionStringResponse::getVersionString() {
    return std::string(payload + 1, getPayloadLength() - 1);
}