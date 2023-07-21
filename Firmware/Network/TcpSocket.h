#pragma once

#include "Socket.h"

#include "Utils/CallOnce.h"

#include <cstdint>
#include <functional>

#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>
#include <lwip/tcp.h>

namespace Network
{

class NetworkController;

class TcpSocket : public Socket
{
public:
    TcpSocket(NetworkController& controller);

    bool connect(const char* host, uint16_t port) override;

    void read() override;
    void write() override;

protected:
    void run() override;

private:
    static constexpr auto PoolTimeSeconds = 5;

    enum class State
    {
        Idle,
        ConnectCalled,
        Connecting,
        Connected
    } _state{ State::Idle };

    uint32_t _lastConnectMillis{ 0 };
    struct tcp_pcb* _tcpPcb{ nullptr };
    ip_addr_t _remoteAddr{};
    uint16_t _port{};

    err_t poll();
    err_t sent(u16_t len);
    err_t recv(struct pbuf* p, err_t err);
    void err(err_t err);
    err_t connected(err_t err);

    void doConnect();
};

}