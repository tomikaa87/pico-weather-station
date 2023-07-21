#pragma once

#include "Runnable.h"

namespace Network
{

class WiFiController : public Runnable
{
public:
    WiFiController();
    ~WiFiController() override;

    void run() override;

    [[nodiscard]] bool isConnected() const;

private:
    bool _initialized{ false };
    bool _connected{ false };
};

}