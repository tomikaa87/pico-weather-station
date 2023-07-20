#pragma once

#include "WiFiController.h"

#include "../IRunnable.h"

#include <array>

namespace Network
{

class Socket;

class NetworkController : public IRunnable
{
public:
    void addSocket(Socket* socket);
    void removeSocket(Socket* socket);

    void run() override;

private:
    WiFiController _wifiController;
    std::array<Socket*, 2> _sockets{{}};
};

}