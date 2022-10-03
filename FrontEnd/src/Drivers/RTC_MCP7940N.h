#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// I2C Start Transaction: START + write `address`
typedef bool (* RTC_MCP7940N_I2CStartTransactionFunc)(
    uint8_t address
);

// I2C Write: writes 1 byte to the bus
typedef bool (* RTC_MCP7940N_I2CWrite)(
    uint8_t data
);

// I2C Read: reads 1 byte from the bus
typedef bool (* RTC_MCP7940N_I2CRead)(
    uint8_t* data,
    bool ack
);

// I2C End Transaction: STOP
typedef bool (* RTC_MCP7940N_I2CEndTransactionFunc)();

typedef struct
{
    RTC_MCP7940N_I2CStartTransactionFunc i2cStartTransaction;
    RTC_MCP7940N_I2CWrite i2cWrite;
    RTC_MCP7940N_I2CRead i2cRead;
    RTC_MCP7940N_I2CEndTransactionFunc i2cEndTransaction;
} RTC_MCP7940N_Device;

bool RTC_MCP7940N_SetDateTime(
    const RTC_MCP7940N_Device* device,
    uint8_t year,
    uint8_t month,
    uint8_t date,
    uint8_t weekday,
    uint8_t hour,
    uint8_t minute,
    uint8_t second,
    bool mode12h,
    bool pm
);

bool RTC_MCP7940N_GetDateTime(
    const RTC_MCP7940N_Device* device,
    uint8_t* year,
    uint8_t* month,
    uint8_t* date,
    uint8_t* weekday,
    uint8_t* hour,
    uint8_t* minute,
    uint8_t* second,
    bool* mode12h,
    bool* pm
);

#ifdef __cplusplus
} // extern "C"
#endif