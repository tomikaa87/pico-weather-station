#pragma once

#include "../IRunnable.h"

namespace Network
{

class WiFiController : public IRunnable
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