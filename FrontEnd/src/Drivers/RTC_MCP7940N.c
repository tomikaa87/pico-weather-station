#include "RTC_MCP7940N.h"

enum
{
    RTC_MCP7940N_CONTROL_BYTE_WRITE = 0b11011110,
    RTC_MCP7940N_CONTROL_BYTE_READ = 0b11011111,

    // Timekeeping
    RTC_MCP7940N_REG_TIME_RTCSEC = 0x00,
    RTC_MCP7940N_REG_TIME_RTCMIN,
    RTC_MCP7940N_REG_TIME_RTCHOUR,
    RTC_MCP7940N_REG_TIME_RTCWKDAY,
    RTC_MCP7940N_REG_TIME_RTCDATE,
    RTC_MCP7940N_REG_TIME_RTCMTH,
    RTC_MCP7940N_REG_TIME_RTCYEAR,
    RTC_MCP7940N_REG_TIME_CONTROL,
    RTC_MCP7940N_REG_TIME_OSCTRIM,

    // Alarm 0
    RTC_MCP7940N_REG_ALARM_ALM0SEC = 0x0A,
    RTC_MCP7940N_REG_ALARM_ALM0MIN,
    RTC_MCP7940N_REG_ALARM_ALM0HOUR,
    RTC_MCP7940N_REG_ALARM_ALM0WKDAY,
    RTC_MCP7940N_REG_ALARM_ALM0DATE,
    RTC_MCP7940N_REG_ALARM_ALM0MTH,

    // Alarm 1
    RTC_MCP7940N_REG_ALARM_ALM1SEC = 0x11,
    RTC_MCP7940N_REG_ALARM_ALM1MIN,
    RTC_MCP7940N_REG_ALARM_ALM1HOUR,
    RTC_MCP7940N_REG_ALARM_ALM1WKDAY,
    RTC_MCP7940N_REG_ALARM_ALM1DATE,
    RTC_MCP7940N_REG_ALARM_ALM1MTH,

    // Power-Down Time-Stamp
    RTC_MCP7940N_REG_PWRDNMIN = 0x18,
    RTC_MCP7940N_REG_PWRDNHOUR,
    RTC_MCP7940N_REG_PWRDNDATE,
    RTC_MCP7940N_REG_PWRDNMTH,

    // Power-Up Time-Stamp
    RTC_MCP7940N_REG_PWRUPMIN = 0x1C,
    RTC_MCP7940N_REG_PWRUPHOUR,
    RTC_MCP7940N_REG_PWRUPDATE,
    RTC_MCP7940N_REG_PWRUPMTH,

    // SRAM
    RTC_MCP7940N_SRAM_SIZE = 64,
    RTC_MCP7940N_SRAM_START = 0x20,
    RTC_MCP7940N_SRAM_END = RTC_MCP7940N_SRAM_START + RTC_MCP7940N_SRAM_SIZE,
    RTC_MCP7940N_SRAM_LAST = RTC_MCP7940N_SRAM_END - 1
};

static bool readMemory(
    const RTC_MCP7940N_Device* const device,
    const uint8_t address,
    uint8_t* const buffer,
    const uint8_t length
)
{
    if (!device->i2cStartTransaction(RTC_MCP7940N_CONTROL_BYTE_WRITE)) {
        return false;
    }

    if (!device->i2cWrite(address)) {
        return false;
    }

    // Repeated start condition
    if (!device->i2cStartTransaction(RTC_MCP7940N_CONTROL_BYTE_READ)) {
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

static bool writeMemory(
    const RTC_MCP7940N_Device* const device,
    const uint8_t address,
    const uint8_t* const buffer,
    const uint8_t length
)
{
    if (!device->i2cStartTransaction(RTC_MCP7940N_CONTROL_BYTE_WRITE)) {
        return false;
    }

    if (!device->i2cWrite(address)) {
        return false;
    }

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

bool RTC_MCP7940N_SetDateTime(
    const RTC_MCP7940N_Device* const device,
    const uint8_t year,
    const uint8_t month,
    const uint8_t date,
    const uint8_t weekday,
    const uint8_t hour,
    const uint8_t minute,
    const uint8_t second,
    const bool mode12h,
    const bool pm
)
{
    // Stop the oscillator before setting the time

    uint8_t rtcsec = 0;

    if (
        !readMemory(
            device,
            RTC_MCP7940N_REG_TIME_RTCSEC,
            &rtcsec,
            sizeof(rtcsec)
        )
    ) {
        return false;
    }

    // Clear ST bit
    rtcsec &= (uint8_t)~(1 << 7);

    if (
        !writeMemory(
            device,
            RTC_MCP7940N_REG_TIME_RTCSEC,
            &rtcsec,
            sizeof(rtcsec)
        )
    ) {
        return false;
    }

    bool vbaten = false;

    for (int i = 0; i < 1000; ++i) {
        uint8_t rtcwkday = 0;

        if (
            !readMemory(
                device,
                RTC_MCP7940N_REG_TIME_RTCWKDAY,
                &rtcwkday,
                sizeof(rtcwkday)
            )
        ) {
            return false;
        }

        // Check OSCRUN bit
        if (rtcwkday & (1 << 5) == 0) {
            // Preserve value of VBATEN
            vbaten = rtcwkday & (1 << 3) > 0;
            break;
        }
    }

    uint8_t regs[7] = { 0 };

    // RTCSEC
    regs[0] =
        // ST=1 - restart the oscillator
        1 << 7
        // SECONE
        | second % 10
        // SECTEN
        | ((second / 10) & 0b111) << 4;

    // RTCMIN
    regs[1] = minute % 10 | ((minute / 10) & 0b111) << 4;

    // RTCHOUR
    regs[2] =
        // 12/!24
        (mode12h ? 1 << 6 : 0)
        // !AM/PM
        | (mode12h && pm ? 1 << 5 : 0)
        // HRONE
        | hour % 10
        // HRTEN
        | (
            mode12h
                ? ((hour / 10) & 0b1) << 4
                : ((hour / 10) & 0b11) << 4
        );

    // RTCWKDAY
    regs[3] =
        // WKDAY
        weekday & 0b111
        // VBATEN
        | (vbaten ? 1 << 3 : 0);

    // RTCDATE
    regs[4] = date % 10 | ((date / 10) & 0b11) << 4;

    // RTCMTH
    regs[5] = month % 10 | ((month / 10) & 0b1) << 4;

    // RTCYEAR
    regs[6] = year % 10 | ((year / 10) & 0b1111) << 4;

    if (
        !writeMemory(
            device,
            RTC_MCP7940N_REG_TIME_RTCSEC,
            regs,
            sizeof(regs)
        )
    ) {
        return false;
    }

    return true;
}

bool RTC_MCP7940N_GetDateTime(
    const RTC_MCP7940N_Device* const device,
    uint8_t* const year,
    uint8_t* const month,
    uint8_t* const date,
    uint8_t* const weekday,
    uint8_t* const hour,
    uint8_t* const minute,
    uint8_t* const second,
    bool* const mode12h,
    bool* const pm
)
{
    uint8_t regs[7] = { 0 };

    if (
        !readMemory(
            device,
            RTC_MCP7940N_REG_TIME_RTCSEC,
            regs,
            sizeof(regs)
        )
    ) {
        return false;
    } 

    *second = ((regs[0] & (0b111 << 4)) >> 4) * 10 + regs[7] & 0b1111;
    
    *minute = ((regs[1] & (0b111 << 4)) >> 4) * 10 + regs[1] & 0b1111;
    
    *mode12h = (regs[2] & (1 << 6)) > 0;
    
    *pm = *mode12h ? regs[2] & (1 << 5) > 0 : false;
    
    *hour = (regs[2] & (1 << 5)) > 0 + (
        *mode12h
            ? ((regs[2] & (1 << 4)) >> 4) * 10
            : ((regs[2] & (0b11 << 4)) >> 4) * 10
    );

    *weekday = regs[3] & 0b111;

    *date = ((regs[4] & (0b11 << 4)) >> 4) * 10 + regs[4] & 0b1111;

    *month = ((regs[5] & (1 << 4)) >> 4) * 10 + regs[5] & 0b1111;

    *year = ((regs[6] & (0b1111 << 4)) >> 4) * 10 + regs[6] & 0b1111;

    return true;
}
