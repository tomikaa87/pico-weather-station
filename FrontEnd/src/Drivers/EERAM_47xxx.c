#include "EERAM_47xxx.h"

enum
{
    EERAM_47xxx_CTRL_SRAM_READ_MASK = 0b10100001,
    EERAM_47xxx_CTRL_SRAM_WRITE_MASK = 0b10100000,
    EERAM_47xxx_CTRL_REG_READ_MASK = 0b00110001,
    EERAM_47xxx_CTRL_REG_WRITE_MASK = 0b00110000,

    EERAM_47xxx_REG_STATUS = 0x00,
    EERAM_47xxx_REG_COMMAND = 0x55,

    EERAM_47xxx_STATUS_BIT_EVENT = 1 << 0,
    EERAM_47xxx_STATUS_BIT_ASE = 1 << 1,
    EERAM_47xxx_STATUS_BIT_BP0 = 1 << 2,
    EERAM_47xxx_STATUS_BIT_BP1 = 1 << 3,
    EERAM_47xxx_STATUS_BIT_BP2 = 1 << 4,
    EERAM_47xxx_STATUS_BIT_AM = 1 << 7,

    EERAM_47xxx_BP_BIT_MASK = 0b111,
};

static inline uint8_t calculateControlByte(
    const EERAM_47xxx_Device* const device,
    const uint8_t controlByteMask
)
{
    return controlByteMask | (device->a1 << 2) | (device->a2 << 3);
}

static bool readMemory(
    const EERAM_47xxx_Device* const device,
    const uint8_t address,
    uint8_t* const data,
    const size_t length,
    const bool sram
)
{
    uint8_t controlByte = calculateControlByte(
        device,
        sram
            ? EERAM_47xxx_CTRL_SRAM_WRITE_MASK
            : EERAM_47xxx_CTRL_REG_WRITE_MASK
    );

    if (
        !device->i2cStartTransmission(
            device->i2cFunctionArg,
            controlByte
        )
    ) {
        return false;
    }

    if (
        !device->i2cWrite(
            device->i2cFunctionArg,
            &address,
            1,
            true
        )
    ) {
        return false;
    }

    controlByte = calculateControlByte(
        device,
        sram
            ? EERAM_47xxx_CTRL_SRAM_READ_MASK
            : EERAM_47xxx_CTRL_REG_READ_MASK
    );

    // Repeated start condition
    if (
        !device->i2cStartTransmission(
            device->i2cFunctionArg,
            controlByte
        )
    ) {
        return false;
    }

    bool ok = device->i2cRead(device->i2cFunctionArg, data, length, false);

    if (!device->i2cEndTransmission(device->i2cFunctionArg)) {
        return false;
    }

    return ok;
}

static bool writeMemory(
    const EERAM_47xxx_Device* const device,
    const uint8_t address,
    const uint8_t* const data,
    const size_t length,
    const bool sram
)
{
    const uint8_t controlByte = calculateControlByte(
        device,
        sram
            ? EERAM_47xxx_CTRL_SRAM_READ_MASK
            : EERAM_47xxx_CTRL_REG_READ_MASK
    );

    if (
        !device->i2cStartTransmission(
            device->i2cFunctionArg,
            controlByte
        )
    ) {
        return false;
    }

    if (!device->i2cWrite(device->i2cFunctionArg, &address, 1, true)) {
        return false;
    }

    if (!device->i2cWrite(device->i2cFunctionArg, data, length, false)) {
        return false;
    }

    if (!device->i2cEndTransmission(device->i2cFunctionArg)) {
        return false;
    }

    return true;
}

bool changeControlRegisterFlags(
    const EERAM_47xxx_Device* const device,
    const uint8_t flags,
    const uint8_t mask
)
{
    uint8_t value = 0;

    if (!EERAM_47xxx_ReadControlRegister(device, &value)) {
        return false;
    }

    // Clear the bits specified in `mask`
    value &= ~mask;

    // Set the bits specified in `flags` masked by `mask`
    value |= flags & mask;

    if (
        !writeMemory(
            device,
            EERAM_47xxx_REG_STATUS,
            &value,
            sizeof(value),
            false
        )
    ) {
        return false;
    }

    return true;
}

bool readControlRegisterFlags(
    const EERAM_47xxx_Device* const device,
    uint8_t* const value,
    const uint8_t mask
)
{
    if (!EERAM_47xxx_ReadControlRegister(device, value)) {
        return false;
    }

    *value &= mask;

    return true;
}

bool EERAM_47xxx_WriteByte(
    const EERAM_47xxx_Device* const device,
    const uint16_t address,
    const uint8_t byte
)
{
    return EERAM_47xxx_WriteBuffer(
        device,
        address,
        &byte,
        sizeof(byte)
    );
}

bool EERAM_47xxx_WriteBuffer(
    const EERAM_47xxx_Device* device,
    const uint16_t address,
    const uint8_t* const buffer,
    const size_t length
)
{
    return writeMemory(device, address, buffer, length, true);
}

bool EERAM_47xxx_ReadByte(
    const EERAM_47xxx_Device* device,
    const uint16_t address,
    uint8_t* const byte
)
{
    return EERAM_47xxx_ReadBuffer(
        device,
        address,
        byte,
        sizeof(uint8_t)
    );
}

bool EERAM_47xxx_ReadBuffer(
    const EERAM_47xxx_Device* const device,
    const uint16_t address,
    uint8_t* const buffer,
    const size_t length
)
{
    return readMemory(device, address, buffer, length, true);
}

bool EERAM_47xxx_SetASE(
    const EERAM_47xxx_Device* const device,
    const bool enabled
)
{
    return changeControlRegisterFlags(
        device,
        enabled ? 0xFF : 0,
        EERAM_47xxx_STATUS_BIT_ASE
    );
}

bool EERAM_47xxx_GetASE(
    const EERAM_47xxx_Device* const device,
    bool* const output
)
{
    uint8_t value = 0;

    if (
        !readControlRegisterFlags(
            device,
            &value,
            EERAM_47xxx_STATUS_BIT_ASE
        )
    ) {
        return false;
    }

    *output = value > 0;

    return true;
}

bool EERAM_47xxx_SetBP(
    const EERAM_47xxx_Device* const device,
    const EERAM_47xxx_BP bp
)
{
    return changeControlRegisterFlags(
        device,
        (bp & 0b111) << 2,
        EERAM_47xxx_STATUS_BIT_BP0
            | EERAM_47xxx_STATUS_BIT_BP1
            | EERAM_47xxx_STATUS_BIT_BP2
    );
}

bool EERAM_47xxx_GetBP(
    const EERAM_47xxx_Device* const device,
    EERAM_47xxx_BP* const output
)
{
    uint8_t value = 0;

    if (
        !readControlRegisterFlags(
            device,
            &value,
            EERAM_47xxx_STATUS_BIT_BP0
                | EERAM_47xxx_STATUS_BIT_BP1
                | EERAM_47xxx_STATUS_BIT_BP2
        )
    ) {
        return false;
    }

    *output = (EERAM_47xxx_BP)(value >> 2);

    return true;
}

bool EERAM_47xxx_GetAM(
    const EERAM_47xxx_Device* const device,
    bool* const output
)
{
    uint8_t value = 0;

    if (
        !readControlRegisterFlags(
            device,
            &value,
            EERAM_47xxx_STATUS_BIT_AM
        )
    ) {
        return false;
    }

    *output = value > 0;

    return true;
}

bool EERAM_47xxx_ClearEVENT(
    const EERAM_47xxx_Device* const device
)
{
    return changeControlRegisterFlags(
        device,
        0,
        EERAM_47xxx_STATUS_BIT_EVENT
    );
}

bool EERAM_47xxx_GetEVENT(
    const EERAM_47xxx_Device* device,
    bool* output
)
{
    uint8_t value = 0;

    if (
        !readControlRegisterFlags(
            device,
            &value,
            EERAM_47xxx_STATUS_BIT_EVENT
        )
    ) {
        return false;
    }

    *output = value > 0;

    return true;
}

bool EERAM_47xxx_ReadControlRegister(
    const EERAM_47xxx_Device* const device,
    uint8_t* const output
)
{
    const uint8_t controlByte = calculateControlByte(
        device,
        EERAM_47xxx_CTRL_REG_READ_MASK
    );

    if (!device->i2cStartTransmission(device->i2cFunctionArg, controlByte)) {
        return false;
    }

    if (!device->i2cRead(device->i2cFunctionArg, output, 1, false)) {
        return false;
    }

    if (!device->i2cEndTransmission(device->i2cFunctionArg)) {
        return false;
    }

    return true;
}

bool EERAM_47xxx_WriteCommand(
    const EERAM_47xxx_Device* const device,
    const EERAM_47xxx_Command command
)
{
    return writeMemory(
        device,
        EERAM_47xxx_REG_COMMAND,
        (const uint8_t*)&command,
        1,
        false
    );
}