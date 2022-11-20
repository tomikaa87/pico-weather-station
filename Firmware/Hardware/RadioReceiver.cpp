#include "RadioReceiver.h"
#include "UartRx.pio.h"

#include <fmt/core.h>

namespace Pins
{
    constexpr auto RX = 19;
}

Hardware::RadioReceiver::RadioReceiver()
{
    uart_rx_program_init(
        pio0,
        0,
        pio_add_program(pio0, &uart_rx_program),
        Pins::RX,
        600
    );
}

void Hardware::RadioReceiver::run()
{
    // 8-bit read from the uppermost byte of the FIFO, as data is left-justified
    auto* rxfifo_shift = reinterpret_cast<const io_rw_8*>(&pio0->rxf[0]) + 3;
    while (!pio_sm_is_rx_fifo_empty(pio0, 0)) {
        const uint8_t data = *rxfifo_shift;

        // printf("Radio: RX=%02X\r\n", data);

        if (inputPacketData(data)) {
            printf("Radio: byte accepted\r\n");

            if (_packetDecoderState == PacketDecoderState::PacketDecoded) {
                printf("Radio: packet decoded, data=");
                for (const auto b : _packetData) {
                    printf("%02X ", b);
                }
                printf("\r\n");
                _packetDecoderState = PacketDecoderState::Idle;

                if (!processPacketData()) {
                    printf("Radio: failed to process packet data\r\n");
                }
            }
        }
    }
}

bool Hardware::RadioReceiver::inputPacketData(const uint8_t data)
{
    switch (_packetDecoderState) {
        case PacketDecoderState::Idle:
            if (data == 0xF0) {
                _packetDecoderState = PacketDecoderState::Preamble;
                _packetDataIndex = 0;
                break;
            }
            return false;

        case PacketDecoderState::Preamble: {
            if (data == 0xF0) {
                printf("inputPacketData: ignoring preamble data, state=Preamble\r\n");
                break;
            }

            _packetDecoderState = PacketDecoderState::WaitingForFirstNibble;

            // Fallthrough

        case PacketDecoderState::WaitingForFirstNibble:
            const auto symbols = decodeManchesterSymbols(data);

            printf("inputPacketData: state=WaitingForFirstNibble, symbols=%02X\r\n", symbols);

            if (symbols & 0xF0) {
                printf("inputPacketData: invalid symbol, state=WaitingForFirstNibble\r\n");
                _packetDecoderState = PacketDecoderState::Idle;
                return false;
            }

            _packetData[_packetDataIndex] = symbols << 4;
            _packetDecoderState = PacketDecoderState::FirstNibbleDecoded;

            break;
        }

        case PacketDecoderState::FirstNibbleDecoded: {
            const auto symbols = decodeManchesterSymbols(data);

            printf("inputPacketData: state=FirstNibbleDecoded, symbols=%02X\r\n", symbols);

            if (symbols & 0xF0) {
                printf("inputPacketData: invalid symbol, state=FirstNibbleDecoded\r\n");
                _packetDecoderState = PacketDecoderState::Idle;
                return false;
            }

            _packetData[_packetDataIndex] |= symbols & 0x0F;
            
            if (++_packetDataIndex >= _packetData.size()) {
                _packetDecoderState = PacketDecoderState::PacketDecoded;
            } else {
                _packetDecoderState = PacketDecoderState::WaitingForFirstNibble;
            }

            break;
        }

        case PacketDecoderState::PacketDecoded:
            printf("inputPacketData: ignoring data, state=PacketDecoded\r\n");
            break;
    }

    return true;
}

bool Hardware::RadioReceiver::processPacketData()
{
    if (_packetData.size() != 17) {
        printf("processPacketData: unsupported packet size\r\n");
        return false;
    }

    static constexpr uint16_t Polynomial = 0x1021;
    uint16_t crc16 = 0;

    // Skip the first and last two bytes (0xFA, 0xAF and the CRC16 checksum)
    for (auto i = 2u; i <= 14; ++i) {
        crc16 ^= _packetData[i];
        for (auto j = 0; j < 8; ++j) {
            if (crc16 & 0x8000) {
                crc16 <<= 1;
            } else {
                crc16 <<= 1;
                crc16 ^= 0x1021;
            }
        }
    }

    // Check the received checksum before doing anything
    if (_packetData[15] != (crc16 >> 8) || _packetData[17] != (crc16 & 0xFF)) {
        printf(
            "processPacketData: checksum error, local=%04X, remote=%02X%02X\r\n",
            crc16,
            _packetData[15], _packetData[16]
        );
        return false;
    }

    printf("processPacketData: checksum OK, decoding\r\n");

    return true;
}

uint8_t Hardware::RadioReceiver::decodeManchesterSymbols(uint8_t symbols)
{
    if (symbols == 0xF0)
        return 0xF0;

    uint8_t decoded = 0;

    for (auto i = 0u; i < 4; ++i)
    {
        // First 2 MSBs
        const uint8_t symbol = symbols & 0xC0;
        // 10 -> 1
        if (symbol == 0x80u)
            decoded |= 1;
        // 01 -> 0
        else if (symbol == 0x40u)
            decoded |= 0;
        // Invalid code
        else
            return 0xFF;
        decoded <<= 1;
        symbols <<= 2;
    }

    decoded >>= 1;

    return decoded;
}