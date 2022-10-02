#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// I2C Start Transaction: START + write `address`
typedef bool (* EERAM_47xxx_I2CStartTransactionFunc)(
    uint8_t address
);

// I2C Write: writes 1 byte to the bus
typedef bool (* EERAM_47xxx_I2CWrite)(
    uint8_t data
);

// I2C Read: reads 1 byte from the bus
typedef bool (* EERAM_47xxx_I2CRead)(
    uint8_t* data,
    bool ack
);

// I2C End Transaction: STOP
typedef bool (* EERAM_47xxx_I2CEndTransactionFunc)();

typedef struct
{
    int a1 : 1;
    int a2 : 1;
    int : 0;

    EERAM_47xxx_I2CStartTransactionFunc i2cStartTransaction;
    EERAM_47xxx_I2CWrite i2cWrite;
    EERAM_47xxx_I2CRead i2cRead;
    EERAM_47xxx_I2CEndTransactionFunc i2cEndTransaction;
} EERAM_47xxx_Device;

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

typedef enum
{
    EERAM_47xxx_BP_NONE = 0b000,
    EERAM_47xxx_BP_UPPER_1_64 = 0b001,
    EERAM_47xxx_BP_UPPER_1_32 = 0b010,
    EERAM_47xxx_BP_UPPER_1_16 = 0b011,
    EERAM_47xxx_BP_UPPER_1_8 = 0b100,
    EERAM_47xxx_BP_UPPER_1_4 = 0b101,
    EERAM_47xxx_BP_UPPER_1_2 = 0b110,
    EERAM_47xxx_BP_ALL_BLOCKS = 0b111
} EERAM_47xxx_BP;

typedef enum
{
    EERAM_47xxx_Command_SoftwareStore = 0b00110011,
    EERAM_47xxx_Command_SoftwareRecall = 0b11011101
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
