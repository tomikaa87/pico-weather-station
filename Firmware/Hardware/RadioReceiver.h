#pragma once

#include "../IRunnable.h"

#include <array>
#include <cstdint>

namespace Hardware
{
    class RadioReceiver : public IRunnable
    {
    public:
        RadioReceiver();

        void run() override;

    private:
        enum class PacketDecoderState
        {
            Idle,
            Preamble,
            WaitingForFirstNibble,
            FirstNibbleDecoded,
            PacketDecoded
        } _packetDecoderState = PacketDecoderState::Idle;

        /*
         * Packet format
         * [0xFA 0xAF][VOLTAGE][DHT_TEMP][DHT_RH][BMP085_TEMP][BMP085_PRES][ERROR_CODE][CHECKSUM]
         *  __ 2 B __  _ 2 B _  __ 2 B _  _2 B__  __ 2 B ____  __ 4 B ____  __ 1 B ___  __ 2 B _
         *
         * Total size: 17 Bytes
         */
        std::array<uint8_t, 17> _packetData{};
        std::size_t _packetDataIndex = 0;

        bool inputPacketData(uint8_t data);
        bool processPacketData();

        static uint8_t decodeManchesterSymbols(uint8_t symbols);
    };
}