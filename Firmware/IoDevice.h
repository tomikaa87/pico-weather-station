#pragma once

#include <cstdint>
#include <vector>

class IoDevice
{
public:
    virtual ~IoDevice() = default;

    virtual std::vector<std::byte> read() = 0;
    virtual bool write(std::vector<std::byte>&& data) = 0;
};