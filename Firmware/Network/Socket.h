#pragma once

#include "IoDevice.h"
#include "Runnable.h"

#include "Utils/CallOnce.h"

#include <cstdint>

namespace Network
{

class NetworkController;

class Socket : public IoDevice, public Runnable
{
public:
    explicit Socket(NetworkController& controller);
    ~Socket() override;

    virtual bool connect(const char* host, uint16_t port) = 0;
    virtual bool disconnect() = 0;

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

}