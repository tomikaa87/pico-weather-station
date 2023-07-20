#pragma once

#include "../IRunnable.h"

#include <cstdint>
#include <functional>

#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>
#include <lwip/tcp.h>

namespace Network
{

class NetworkController;

class IODevice
{
public:
    virtual ~IODevice() = default;

    virtual void read() = 0;
    virtual void write() = 0;
};

template <typename Func>
class CallOnce
{
public:
    CallOnce() = default;

    explicit CallOnce(Func&& func)
        : _func{ std::move(func) }
    {}

    void callIfSignaled()
    {
        if (_funcWrapper) {
            decltype(_funcWrapper) funcWrapper;
            std::swap(funcWrapper, _funcWrapper);
            funcWrapper();
        }
    }

    template <typename... Args>
    void signal(Args... args)
    {
        if (_func) {
            _funcWrapper = [...args{ std::forward<Args>(args) }, this]() mutable {
                _func(std::forward<Args>(args)...);
            };
        }
    }

private:
    Func _func;
    std::function<void ()> _funcWrapper;
    bool _call{ false };
};

class Socket : public IODevice, public IRunnable
{
public:
    explicit Socket(NetworkController& controller);
    ~Socket() override;

    virtual bool connect(const char* host, uint16_t port) = 0;

    enum class State
    {
        Disconnected,
        Connecting,
        Connected,
        Disconnecting
    };

    using StateChangedHandler = std::function<void (State)>;
    using ReceivedHandler = std::function<void()>;
    using ErrorHandler = std::function<void ()>;

    void setStateChangedHandler(StateChangedHandler&& handler);
    void setReceivedHandler(ReceivedHandler&& handler);
    void setErrorHandler(ErrorHandler&& handler);

    void run() override;

protected:
    NetworkController& _controller;
    CallOnce<StateChangedHandler> _stateChangedHandler;
    CallOnce<ReceivedHandler> _receivedHandler;
    CallOnce<ErrorHandler> _errorHandler;
};

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