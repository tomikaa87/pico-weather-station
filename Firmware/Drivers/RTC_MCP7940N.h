#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// I2C Start Transaction: START + write `address`
typedef bool (* RTC_MCP7940N_I2CStartTransmissionFunc)(
    void* arg,
    uint8_t address
);

// I2C Write: writes n byte(s) to the bus
typedef bool (* RTC_MCP7940N_I2CWrite)(
    void* arg,
    const uint8_t* data,
    size_t length,
    bool noStop
);

// I2C Read: reads `length` byte(s) from the bus
typedef bool (* RTC_MCP7940N_I2CRead)(
    void* arg,
    uint8_t* data,
    size_t length,
    bool noStop
);

// I2C End Transaction: STOP
typedef bool (* RTC_MCP7940N_I2CEndTransmissionFunc)(void* arg);

typedef struct
{
    void* i2cFunctionArg;
    RTC_MCP7940N_I2CStartTransmissionFunc i2cStartTransmission;
    RTC_MCP7940N_I2CWrite i2cWrite;
    RTC_MCP7940N_I2CRead i2cRead;
    RTC_MCP7940N_I2CEndTransmissionFunc i2cEndTransmission;
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
    RTC_MCP7940N_ALARM_POLARITY_LOW,
    RTC_MCP7940N_ALARM_POLARITY_HIGH
} RTC_MCP7940N_AlarmPolarity;

typedef enum
{
    RTC_MCP7940N_SWQOUT_FREQ_1HZ,
    RTC_MCP7940N_SWQOUT_FREQ_4096HZ,
    RTC_MCP7940N_SWQOUT_FREQ_8192HZ,
    RTC_MCP7940N_SWQOUT_FREQ_32768HZ
} RTC_MCP7940N_SQWOutputFreq;

typedef enum
{
    RTC_MCP7940N_POWER_DOWN_TIMESTAMP,
    RTC_MCP7940N_POWER_UP_TIMESTAMP
} RTC_MCP7940N_PowerFailTimeStampType;

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

bool RTC_MCP7940N_GetLeapYearFlag(
    const RTC_MCP7940N_Device* device,
    bool* value
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

bool RTC_MCP7940N_ReadSRAM(
    const RTC_MCP7940N_Device* device,
    uint8_t address,
    uint8_t* buffer,
    uint8_t length
);

bool RTC_MCP7940N_WriteSRAM(
    const RTC_MCP7940N_Device* device,
    uint8_t address,
    const uint8_t* buffer,
    uint8_t length
);

bool RTC_MCP7940N_SetSquareWaveOutputFrequency(
    const RTC_MCP7940N_Device* device,
    RTC_MCP7940N_SQWOutputFreq frequency
);

bool RTC_MCP7940N_SetSquareWaveOutputEnabled(
    const RTC_MCP7940N_Device* device,
    bool enabled
);

bool RTC_MCP7940N_SetCoarseTrimmingEnabled(
    const RTC_MCP7940N_Device* device,
    bool enabled
);

bool RTC_MCP7940N_SetExternalOscillatorEnabled(
    const RTC_MCP7940N_Device* device,
    bool enabled
);

bool RTC_MCP7940N_SetOscillatorDigitalTrimming(
    const RTC_MCP7940N_Device* device,
    uint8_t value,
    bool subtract
);

bool RTC_MCP7940N_SetBatteryBackupEnabled(
    const RTC_MCP7940N_Device* device,
    bool enabled
);

bool RTC_MCP7940N_ClearPowerFailStatus(
    const RTC_MCP7940N_Device* device
);

bool RTC_MCP7940N_GetOscillatorRunningFlag(
    const RTC_MCP7940N_Device* device,
    bool* value
);

bool RTC_MCP7940N_SetGeneralPurposeOutput(
    const RTC_MCP7940N_Device* device,
    bool high
);

bool RTC_MCP7940N_GetPowerFailTimeStamp(
    const RTC_MCP7940N_Device* device,
    RTC_MCP7940N_PowerFailTimeStampType type,
    uint8_t* minute,
    uint8_t* hour,
    uint8_t* date,
    uint8_t* weekday,
    uint8_t* month,
    bool* mode12h,
    bool* pm
);

#ifdef __cplusplus
} // extern "C"
#endif