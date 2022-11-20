#pragma once

#include <cstdint>
#include <utility>

namespace Hardware
{
    class Keypad
    {
        // Number of cycles to wait between reading two buttons
        static constexpr uint32_t ScanDelay = 10;
        // Number of key scans at every read instruction
        static constexpr uint32_t ReadSamples = 10;
        // Number of ticks before repeating
        static constexpr uint32_t DelayBeforeRepeat = 500;
        // Maximum number of ticks to wait between repeats
        static constexpr uint32_t RepeatDelayBegin = 100;
        // Minimum number of tick to wait between repeats
        static constexpr uint32_t RepeatDelayEnd = 25;
        // Ticks to wait before decreasing repeat wait
        static constexpr uint32_t RepeatSpeedUpDelay = 100;
        static constexpr uint32_t RepeatSpeedUpSteps = 5;
        // Delay after resetting, it gives some more bounce filtering
        static constexpr uint32_t ResetDelay = 100;

    public:
        enum class Key : uint16_t{
            None = 0,
            Key0 = 1 << 0,
            Key1 = 1 << 1,
            Key2 = 1 << 2,
            Key3 = 1 << 3,
            // The last two keys are swapped on the PCB
            Key5 = 1 << 4,
            Key4 = 1 << 5
        };

        enum class Press {
            None,
            Short,
            Long
        };

        Keypad();

        std::pair<Key, Press> task(uint32_t sysTicks);

    private:
        uint32_t _pressTicks = 0;
        uint32_t _repeatTicks = 0;
        uint32_t _repeatDelay = 0;
        uint16_t _lastScanCode = 0;
        uint32_t _resetTicks = 0;

        enum class State {
            Idle,
            Pressed,
            Repeat
        } _state = State::Idle;

        void setDriverEnabled(bool enabled, int driver);
        bool readScanInput(int input);
        public: uint16_t scan();
    };
}