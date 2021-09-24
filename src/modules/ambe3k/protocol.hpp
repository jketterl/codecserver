#pragma once

#include <string>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <arpa/inet.h> // htons

#define AMBE3K_START_BYTE 0x61
#define AMBE3K_PARITY_BYTE 0x2f

#define AMBE3K_TYPE_CONTROL 0x00
#define AMBE3K_TYPE_AMBE 0x01
#define AMBE3K_TYPE_AUDIO 0x02

#define AMBE3K_CONTROL_RATEP 0x0A
#define AMBE3K_CONTROL_RATET 0x09
#define AMBE3K_CONTROL_INIT 0x0B
#define AMBE3K_CONTROL_PRODID 0x30
#define AMBE3K_CONTROL_VERSTRING 0x31
#define AMBE3K_CONTROL_RESET 0x33
#define AMBE3K_CONTROL_READY 0x39
#define AMBE3K_CONTROL_CHANFMT 0x15
#define AMBE3K_CONTROL_SPCHFMT 0x16
#define AMBE3K_CONTROL_COMPAND 0x32

#define AMBE3K_DIRECTION_ENCODE 0x01
#define AMBE3K_DIRECTION_DECODE 0x02

#define AMBE3K_CHANNEL_CHAND 0x01
#define AMBE3K_SPEECH_SPEECHD 0x00

#define AMBE3K_FIFO_MAX_PENDING 3

namespace Ambe3K::Protocol {

    class Field {
        public:
            explicit Field(char* data);
            virtual ~Field() = default;
            virtual size_t getLength() = 0;
        protected:
            char* data;
    };

    class ChecksumField: public Field {
        public:
            explicit ChecksumField(char* data);
            size_t getLength() override;
            void setChecksum(char checksum);
            bool isCorrect(char checksum);
    };

    class ResetField: public Field {
        public:
            explicit ResetField(char* data);
            size_t getLength() override;
    };

    class ReadyField: public Field {
        public:
            explicit ReadyField(char* data);
            size_t getLength() override;
    };

    class ProdIdRequestField: public Field {
        public:
            explicit ProdIdRequestField(char* data);
            size_t getLength() override;
    };

    class ProdIdResponseField: public Field {
        public:
            explicit ProdIdResponseField(char* data);
            size_t getLength() override;
            std::string getProductId();
        private:
            std::string productId;
    };

    class VersionStringRequestField: public Field {
        public:
            explicit VersionStringRequestField(char* data);
            size_t getLength() override;
    };

    class VersionStringResponseField: public Field {
        public:
            explicit VersionStringResponseField(char* data);
            size_t getLength() override;
            std::string getVersionId();
    };

    class ChannelRequestField: public Field {
        public:
            explicit ChannelRequestField(char* data, unsigned char channel);
            size_t getLength() override;
    };

    class ChannelResponseField: public Field {
        public:
            explicit ChannelResponseField(char* data);
            size_t getLength() override;
            unsigned char getChannel();
    };

    class RateTRequestField: public Field {
        public:
            explicit RateTRequestField(char* data, unsigned char index);
            size_t getLength() override;
    };

    class RateTResponseField: public Field {
        public:
            explicit RateTResponseField(char* data);
            size_t getLength() override;
    };

    class RatePRequestField: public Field {
        public:
            explicit RatePRequestField(char* data, short* cwds);
            size_t getLength() override;
    };

    class RatePResponseField: public Field {
        public:
            explicit RatePResponseField(char* data);
            size_t getLength() override;
    };

    class InitRequestField: public Field {
        public:
            explicit InitRequestField(char* data, unsigned char direction);
            size_t getLength() override;
    };

    class InitResponseField: public Field {
        public:
            explicit InitResponseField(char* data);
            size_t getLength() override;
    };

    class ChanDField: public Field {
        public:
            explicit ChanDField(char* data);
            ChanDField(char* data, char* channelData, unsigned char bits);
            size_t getLength() override;
            size_t getChannelData(char* output);
        private:
            size_t bits;
    };

    class SpeechDField: public Field {
        public:
            explicit SpeechDField(char* data);
            SpeechDField(char* data, char* speechData, unsigned char samples);
            size_t getLength() override;
            size_t getSpeechData(short* output);
        private:
            size_t samples;
    };

    class Packet {
        public:
            static Packet* parse(char* data, size_t bytes);
            bool hasChecksum();
            bool isChecksumValid();

            void writeTo(int fd);
            static Packet* receiveFrom(int fd);
            // necessary to maintain polymorphism
            virtual ~Packet();

            bool hasChannel();
            unsigned char getChannel();
        protected:
            // used when parsing incoming packets
            Packet(char* payload, size_t bytes);
            // used when constructing outgoing packets
            explicit Packet(size_t bytes);
            Packet(size_t bytes, char type);

            void scanFields();
            virtual Field* buildField(char* current);
            virtual void storeField(Field* field);

            void updateChecksum();
            size_t getPayloadLength();

            char* payload;
            ChecksumField* checksum = nullptr;
            ChannelResponseField* channel = nullptr;
        private:
            char getChecksum();
            char* data;
            size_t dataSize;
    };

    class ControlPacket: public Packet {
        public:
            explicit ControlPacket(size_t bytes);
            ControlPacket(char* payload, size_t bytes): Packet(payload, bytes) {}
            ~ControlPacket() override;

            bool hasReadyField();
            bool hasProductId();
            std::string getProductId();
            bool hasVersionString();
            std::string getVersionString();
            bool hasRatePResponse();
            bool hasRateTResponse();
            bool hasInitResponse();
        protected:
            Field* buildField(char* current) override;
            void storeField(Field* field) override;
        private:
            ReadyField* ready = nullptr;
            ProdIdResponseField* prodid = nullptr;
            VersionStringResponseField* version = nullptr;
            RateTResponseField* rateTResponse = nullptr;
            RatePResponseField* ratePResponse = nullptr;
            InitResponseField* initResponse = nullptr;
    };

    class ResetPacket: public ControlPacket {
        public:
            ResetPacket();
            ~ResetPacket() override;
        private:
            ResetField* reset;
    };

    class ProdIdRequest: public ControlPacket {
        public:
            ProdIdRequest();
            ~ProdIdRequest() override;
        private:
            ProdIdRequestField* request;
    };

    class VersionStringRequest: public ControlPacket {
        public:
            VersionStringRequest();
            ~VersionStringRequest() override;
        private:
            VersionStringRequestField* request;
    };

    class SetupRequest: public ControlPacket{
        public:
            SetupRequest(unsigned char channel, unsigned char index, unsigned char direction);
            SetupRequest(unsigned char channel, short* cwds, unsigned char direction);
            ~SetupRequest() override;
        private:
            ChannelRequestField* channel;
            Field* request;
            InitRequestField* init;
    };

    class ChannelPacket: public Packet {
        public:
            ChannelPacket(char* data, size_t size): Packet(data, size) {}
            ChannelPacket(unsigned char channel, char* channelData, unsigned char bits);
            ~ChannelPacket() override;
            size_t getChannelData(char* output);
        protected:
            Field* buildField(char* current) override;
            void storeField(Field* field) override;
        private:
            ChannelRequestField* channel = nullptr;
            ChanDField* chanD = nullptr;
    };

    class SpeechPacket: public Packet {
        public:
            SpeechPacket(char* data, size_t size): Packet(data, size) {}
            SpeechPacket(unsigned char channel, char* speechData, unsigned char samples);
            ~SpeechPacket() override;
            size_t getSpeechData(short* output);
        protected:
            Field* buildField(char* current) override;
            void storeField(Field* field) override;
        private:
            ChannelRequestField* channel = nullptr;
            SpeechDField* speechD = nullptr;
    };


}