#include "protocol.hpp"
#include <cassert>
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
    char type = p->getType();
    if (type == DV3K_TYPE_CONTROL) {
        char opCode = p->payload[0];
        if (opCode >= 0x40 && opCode <= 0x42) {
            // response is for a specific channel... move forward
            opCode = p->payload[2];
        }
        switch(opCode) {
            case DV3K_CONTROL_READY:
                delete p;
                return new ReadyPacket(data, size);
            case DV3K_CONTROL_PRODID:
                delete p;
                return new ProdIdResponse(data, size);
            case DV3K_CONTROL_VERSTRING:
                delete p;
                return new VersionStringResponse(data, size);
            case DV3K_CONTROL_RATET:
                delete p;
                return new RateTResponse(data, size);
            case DV3K_CONTROL_RATEP:
                delete p;
                return new RatePResponse(data, size);
            default:
                std::cerr << "unexpected opcode: " << std::hex << +opCode << "\n";
        }
    } else if (type == DV3K_TYPE_AUDIO) {
        delete p;
        return new SpeechPacket(data, size);
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
    short remain = 4;
    for (int i = 0; i < 10; i++) {
        remain -= read(fd, buf + (4 - remain), remain);
        if (remain == 0) {
            break;
        }
    }

    if (remain > 0 || buf[0] != DV3K_START_BYTE) {
        return nullptr;
    }

    short payloadLength = ntohs(*((short*) &buf[1]));
    remain = payloadLength;
    buf = (char*) realloc(buf, remain + 4);

    for (int i = 0; i < 10; i++) {
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

unsigned char RateTResponse::getChannel() {
    return payload[0] - 0x40;
}

char RateTResponse::getResult() {
    return payload[3];
}

unsigned char RatePResponse::getChannel() {
    return payload[0] - 0x40;
}


char RatePResponse::getResult() {
    return payload[3];
}

size_t SpeechPacket::getSpeechData(char* output) {
    // skip channel packets
    char* pos = payload + 1;
    size_t len = 0;
    while (pos < payload + getPayloadLength()) {
        if (pos[0] == 0x00) {
            len = (unsigned char) pos[1];
            pos += 2;
            for (int i = 0; i < len; i++) {
                ((short*) output)[i] = ntohs(((short*) pos)[i]);
            }
            //memcpy(output, pos, len * 2);
            pos += len * 2;
        } else {
            std::cerr << "unexpected field data: " << std::hex << +pos[0] << "\n";
            pos += 1;
        }
    }
    return len * 2;
}

unsigned char SpeechPacket::getChannel() {
    return payload[0] - 0x40;
}