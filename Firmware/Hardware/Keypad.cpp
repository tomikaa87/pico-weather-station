#include "Keypad.h"

#include <hardware/gpio.h>
#include <pico/time.h>

#include <array>
#include <cstdio>
#include <initializer_list>

namespace Pins::Keypad
{
    constexpr auto Drive0 = 1;
    constexpr auto Drive1 = 2;
    constexpr auto Drive2 = 26;
    constexpr auto Scan0 = 28;
    constexpr auto Scan1 = 27;
}

Hardware::Keypad::Keypad()
{
    for (
        const auto pin : {
            Pins::Keypad::Drive0,
            Pins::Keypad::Drive1,
            Pins::Keypad::Drive2,
            Pins::Keypad::Scan0,
            Pins::Keypad::Scan1
        }
    ) {
        gpio_init(pin);
        gpio_set_dir(pin, false);
    }

    for (
        const auto pin : {
            Pins::Keypad::Scan0,
            Pins::Keypad::Scan1
        }
    ) {
        gpio_set_pulls(pin, true, false);
        gpio_set_input_hysteresis_enabled(pin, true);
    }
}

std::pair<Hardware::Keypad::Key, Hardware::Keypad::Press>
Hardware::Keypad::task(const uint32_t sysTicks)
{
    const auto scanCode = scan();

    switch (_state) {
        case State::Idle: {
            if (_resetTicks != 0 && sysTicks - _resetTicks < ResetDelay) {
                break;
            }

            if (scanCode != 0) {
                _state = State::Pressed;
                _pressTicks = sysTicks;
                _lastScanCode = scanCode;

                return std::make_pair(
                    static_cast<Key>(scanCode),
                    Press::Short
                );
            }
            break;
        }

        case State::Pressed: {
            if (_lastScanCode != scanCode) {
                _state = State::Idle;
                _resetTicks = sysTicks;
            } else if (sysTicks - _pressTicks >= DelayBeforeRepeat) {
                _pressTicks = sysTicks;
                _repeatTicks = sysTicks;
                _repeatDelay = RepeatDelayBegin;
                _state = State::Repeat;

                printf("_repeatDelay=%u\n", _repeatDelay);

                return std::make_pair(
                    static_cast<Key>(scanCode),
                    Press::Long
                );
            }
            break;
        }

        case State::Repeat: {
            if (_lastScanCode != scanCode) {
                _state = State::Idle;
                break;
            }

            if (sysTicks - _repeatTicks >= _repeatDelay) {
                _repeatTicks = sysTicks;

                return std::make_pair(
                    static_cast<Key>(scanCode),
                    Press::Long
                );
            }

            if (
                _repeatDelay > RepeatDelayEnd
                && sysTicks - _pressTicks >= RepeatSpeedUpDelay
            ) {
                _repeatDelay -= RepeatSpeedUpSteps;
                _pressTicks = sysTicks;

                printf("_repeatDelay=%u\n", _repeatDelay);
            }

            break;
        }
    }

    return std::make_pair(Key::None, Press::None);
}

uint16_t Hardware::Keypad::scan()
{
    uint16_t code = 0;

    for (auto i = 0u; i < ReadSamples; ++i) {

        for (const auto driver : { 0, 1, 2 }) {
            setDriverEnabled(true, driver);
            
            uint16_t bits = 0;

            for (const auto input : { 0, 1 }) {
                sleep_us(ScanDelay);
                if (readScanInput(input) == 0) {
                    bits |= 1 << input;
                }
            }

            code |= bits << (driver * 2);

            setDriverEnabled(false, driver);
        }
    }

    // Allow only single keypresses (1 set bit at most)
    if (__builtin_popcount(code) > 1) {
        return 0;
    }

    return code;
}

void Hardware::Keypad::setDriverEnabled(
    const bool enabled,
    const int driver
)
{
    if (driver < 0 || driver > 2) {
        return;
    }

    static constexpr std::array Drivers{
        Pins::Keypad::Drive0,
        Pins::Keypad::Drive1,
        Pins::Keypad::Drive2
    };

    // printf("setDriverEnabled(e=%d, d=%d)\n", enabled, driver);

    gpio_put(Drivers[driver], !enabled);
    gpio_set_dir(Drivers[driver], enabled);
}

bool Hardware::Keypad::readScanInput(const int input)
{
    if (input < 0 || input > 1) {
        return false;
    }

    static constexpr std::array Inputs{
        Pins::Keypad::Scan0,
        Pins::Keypad::Scan1
    };

    const auto high = gpio_get(Inputs[input]);

    // printf("readScanInput(i=%d): %d\n", input, high);

    return high;
}