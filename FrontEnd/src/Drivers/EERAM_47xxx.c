#include "EERAM_47xxx.h"

static inline uint8_t calculateControlByte(
    const EERAM_47xxx_Device* const device,
    const uint8_t controlByteMask
)
{
    return controlByteMask | (device->a1 << 2) | (device->a2 << 3);
}

static bool writeRegister(
    const EERAM_47xxx_Device* const device,
    const uint8_t address,
    const uint8_t data
)
{
    const uint8_t controlByte = calculateControlByte(
        device,
        EERAM_47xxx_CTRL_REG_WRITE_MASK
    );

    if (!device->i2cStartTransaction(controlByte)) {
        return false;
    }

    if (!device->i2cWrite(address)) {
        return false;
    }

    if (!device->i2cWrite(data)) {
        return false;
    }

    if (!device->i2cEndTransaction()) {
        return false;
    }

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
    const uint8_t controlByte = calculateControlByte(
        device,
        EERAM_47xxx_CTRL_SRAM_WRITE_MASK
    );

    if (!device->i2cStartTransaction(controlByte)) {
        return false;
    }

    // Address high byte
    if (!device->i2cWrite((address >> 8) & 0xFF)) {
        return false;
    }

    // Address low byte
    if (!device->i2cWrite(address & 0xFF)) {
        return false;
    }

    // Data bytes
    for (size_t i = 0; i < length; ++i) {
        if (!device->i2cWrite(buffer[i])) {
            return false;
        }
    }

    if (!device->i2cEndTransaction()) {
        return false;
    }

    return true;
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
    const uint8_t writeControlByte = calculateControlByte(
        device,
        EERAM_47xxx_CTRL_SRAM_WRITE_MASK
    );

    if (!device->i2cStartTransaction(writeControlByte)) {
        return false;
    }

    // Address high byte
    if (!device->i2cWrite((address >> 8) & 0xFF)) {
        return false;
    }

    // Address low byte
    if (!device->i2cWrite(address & 0xFF)) {
        return false;
    }

    // Send Control Byte
    const uint8_t readControlByte = calculateControlByte(
        device,
        EERAM_47xxx_CTRL_SRAM_READ_MASK
    );

    // Repeated start condition
    if (!device->i2cStartTransaction(readControlByte)) {
        return false;
    }

    for (size_t i = 0; i < length; ++i) {
        const bool last = i == length - 1;

        if (!device->i2cRead(&buffer[i], last)) {
            return false;
        }
    }

    if (!device->i2cEndTransaction()) {
        return false;
    }

    return true;
}

bool EERAM_47xxx_SetASE(
    const EERAM_47xxx_Device* const device,
    const bool enabled
)
{
    uint8_t status = 0;

    if (!EERAM_47xxx_ReadControlRegister(device, &status)) {
        return false;
    }

    status |= EERAM_47xxx_STATUS_BIT_ASE;

    if (!writeRegister(device, EERAM_47xxx_REG_STATUS, status)) {
        return false;
    }

    return true;
}

bool EERAM_47xxx_GetASE(
    const EERAM_47xxx_Device* const device,
    bool* const output
)
{
    uint8_t status = 0;

    if (!EERAM_47xxx_ReadControlRegister(device, &status)) {
        return false;
    }

    *output = status & EERAM_47xxx_STATUS_BIT_ASE > 0;

    return true;
}

bool EERAM_47xxx_SetBP(
    const EERAM_47xxx_Device* const device,
    const EERAM_47xxx_BP bp
)
{
    uint8_t status = 0;

    if (!EERAM_47xxx_ReadControlRegister(device, &status)) {
        return false;
    }

    status &= (uint8_t)~(EERAM_47xxx_BP_BIT_MASK << 2);
    status |= ((uint8_t)bp & EERAM_47xxx_BP_BIT_MASK) << 2;

    if (!writeRegister(device, EERAM_47xxx_REG_STATUS, status)) {
        return false;
    }

    return true;
}

bool EERAM_47xxx_GetBP(
    const EERAM_47xxx_Device* const device,
    EERAM_47xxx_BP* const output
)
{
    uint8_t status = 0;

    if (!EERAM_47xxx_ReadControlRegister(device, &status)) {
        return false;
    }

    *output = (EERAM_47xxx_BP)((status & EERAM_47xxx_BP_BIT_MASK) >> 2);

    return true;
}

bool EERAM_47xxx_GetAM(
    const EERAM_47xxx_Device* const device,
    bool* const output
)
{
    uint8_t status = 0;

    if (!EERAM_47xxx_ReadControlRegister(device, &status)) {
        return false;
    }

    *output = status & EERAM_47xxx_STATUS_BIT_AM > 0;

    return true;
}

bool EERAM_47xxx_ClearEVENT(
    const EERAM_47xxx_Device* const device
)
{
    uint8_t status = 0;

    if (!EERAM_47xxx_ReadControlRegister(device, &status)) {
        return false;
    }

    status &= (uint8_t)~EERAM_47xxx_STATUS_BIT_EVENT;

    if (!writeRegister(device, EERAM_47xxx_REG_STATUS, status)) {
        return false;
    }

    return true;
}

bool EERAM_47xxx_GetEVENT(
    const EERAM_47xxx_Device* device,
    bool* output
)
{
    uint8_t status = 0;

    if (!EERAM_47xxx_ReadControlRegister(device, &status)) {
        return false;
    }

    *output = status & EERAM_47xxx_STATUS_BIT_EVENT > 0;

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

    if (!device->i2cStartTransaction(controlByte)) {
        return false;
    }

    if (!device->i2cRead(output, false)) {
        return false;
    }

    if (!device->i2cEndTransaction()) {
        return false;
    }

    return true;
}

bool EERAM_47xxx_WriteCommand(
    const EERAM_47xxx_Device* const device,
    const EERAM_47xxx_Command command
)
{
    return writeRegister(
        device,
        EERAM_47xxx_REG_COMMAND,
        (uint8_t)command
    );
}