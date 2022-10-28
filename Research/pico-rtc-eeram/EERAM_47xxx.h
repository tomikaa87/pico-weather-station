#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// I2C Start Transaction: START + write `address`
typedef bool (* EERAM_47xxx_I2CStartTransmissionFunc)(
    void* arg,
    uint8_t address
);

// I2C Write: writes n byte(s) to the bus
typedef bool (* EERAM_47xxx_I2CWrite)(
    void* arg,
    const uint8_t* data,
    size_t length,
    bool noStop
);

// I2C Read: reads n byte(s) from the bus
typedef bool (* EERAM_47xxx_I2CRead)(
    void* arg,
    uint8_t* data,
    size_t length,
    bool noStop
);

// I2C End Transaction: STOP
typedef bool (* EERAM_47xxx_I2CEndTransmissionFunc)(void* arg);

typedef struct
{
    int a1 : 1;
    int a2 : 1;
    int : 0;

    void* i2cFunctionArg;
    EERAM_47xxx_I2CStartTransmissionFunc i2cStartTransmission;
    EERAM_47xxx_I2CWrite i2cWrite;
    EERAM_47xxx_I2CRead i2cRead;
    EERAM_47xxx_I2CEndTransmissionFunc i2cEndTransmission;
} EERAM_47xxx_Device;

typedef enum
{
    EERAM_47XXX_BP_NONE = 0b000,
    EERAM_47XXX_BP_UPPER_1_64 = 0b001,
    EERAM_47XXX_BP_UPPER_1_32 = 0b010,
    EERAM_47XXX_BP_UPPER_1_16 = 0b011,
    EERAM_47XXX_BP_UPPER_1_8 = 0b100,
    EERAM_47XXX_BP_UPPER_1_4 = 0b101,
    EERAM_47XXX_BP_UPPER_1_2 = 0b110,
    EERAM_47XXX_BP_ALL_BLOCKS = 0b111
} EERAM_47xxx_BP;

typedef enum
{
    EERAM_47XXX_CMD_SOFTWARE_STORE = 0b00110011,
    EERAM_47XXX_CMD_SOFTWARE_RECALL = 0b11011101
} EERAM_47xxx_Command;

bool EERAM_47xxx_WriteByte(
    const EERAM_47xxx_Device* device,
    uint16_t address,
    uint8_t byte
);

bool EERAM_47xxx_WriteBuffer(
    const EERAM_47xxx_Device* device,
    uint16_t address,
    const uint8_t* buffer,
    size_t length
);

bool EERAM_47xxx_ReadByte(
    const EERAM_47xxx_Device* device,
    uint16_t address,
    uint8_t* byte
);

bool EERAM_47xxx_ReadBuffer(
    const EERAM_47xxx_Device* device,
    uint16_t address,
    uint8_t* buffer,
    size_t length
);

bool EERAM_47xxx_SetASE(
    const EERAM_47xxx_Device* device,
    bool enabled
);

bool EERAM_47xxx_GetASE(
    const EERAM_47xxx_Device* device,
    bool* output
);

bool EERAM_47xxx_SetBP(
    const EERAM_47xxx_Device* device,
    EERAM_47xxx_BP bp
);

bool EERAM_47xxx_GetBP(
    const EERAM_47xxx_Device* device,
    EERAM_47xxx_BP* output
);

bool EERAM_47xxx_GetAM(
    const EERAM_47xxx_Device* device,
    bool* output
);

bool EERAM_47xxx_ClearEVENT(
    const EERAM_47xxx_Device* device
);

bool EERAM_47xxx_GetEVENT(
    const EERAM_47xxx_Device* device,
    bool* output
);

bool EERAM_47xxx_ReadControlRegister(
    const EERAM_47xxx_Device* device,
    uint8_t* output
);

bool EERAM_47xxx_WriteStatus(
    const EERAM_47xxx_Device* device,
    uint8_t status
);

bool EERAM_47xxx_WriteCommand(
    const EERAM_47xxx_Device* device,
    EERAM_47xxx_Command command
);

#ifdef __cplusplus
} // extern "C"
#endif
