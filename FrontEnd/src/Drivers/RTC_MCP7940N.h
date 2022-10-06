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

typedef enum
{
    RTC_MCP7940N_ALARM_0,
    RTC_MCP7940N_ALARM_1,
} RTC_MCP7940N_Alarm;

typedef enum
{
    RTC_MCP7940N_ALARM_MASK_SECONDS,
    RTC_MCP7940N_ALARM_MASK_MINUTES,
    RTC_MCP7940N_ALARM_MASK_HOURS,
    RTC_MCP7940N_ALARM_MASK_DAY_OF_WEEK,
    RTC_MCP7940N_ALARM_MASK_DATE,
    RTC_MCP7940N_ALARM_MASK_ALL
} RTC_MCP7940N_AlarmMask;

typedef enum
{
    RTC_MCP7940N_ALARM_POLARITY_HIGH,
    RTC_MCP7940N_ALARM_POLARITY_LOW,
} RTC_MCP7940N_AlarmPolarity;

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

bool RTC_MCP7940N_SetAlarm(
    const RTC_MCP7940N_Device* device,
    RTC_MCP7940N_Alarm alarm,
    RTC_MCP7940N_AlarmMask mask,
    RTC_MCP7940N_AlarmPolarity polarity,
    uint8_t month,
    uint8_t date,
    uint8_t weekday,
    uint8_t hour,
    uint8_t minute,
    uint8_t second,
    bool mode12h,
    bool pm
);

bool RTC_MCP7940N_SetAlarmEnabled(
    const RTC_MCP7940N_Device* device,
    RTC_MCP7940N_Alarm alarm,
    bool enabled
);

bool RTC_MCP7940N_GetAlarmInterruptFlag(
    const RTC_MCP7940N_Device* device,
    RTC_MCP7940N_Alarm alarm,
    bool* value
);

bool RTC_MCP7940N_ClearAlarmInterruptFlag(
    const RTC_MCP7940N_Device* device,
    RTC_MCP7940N_Alarm alarm
);

#ifdef __cplusplus
} // extern "C"
#endif