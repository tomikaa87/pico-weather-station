#pragma once

#include <memory>

class Display
{
public:
    explicit Display();
    ~Display();

    void clear();
    void setContrast(uint8_t value);

private:
    struct Private;
    std::unique_ptr<Private> _p;

    void setup();
};