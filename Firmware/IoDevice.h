#pragma once

class IoDevice
{
public:
    virtual ~IoDevice() = default;

    virtual void read() = 0;
    virtual void write() = 0;
};