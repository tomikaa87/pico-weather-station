#include "Socket.h"
#include "NetworkController.h"

namespace Network
{

Socket::Socket(NetworkController& controller)
    : _controller{ controller }
{
    _controller.addSocket(this);
}

Socket::~Socket()
{
    _controller.removeSocket(this);
}

void Socket::setStateChangedHandler(StateChangedHandler&& handler)
{
    _stateChangedHandler = CallOnce<StateChangedHandler>(std::move(handler));
}

void Socket::setReceivedHandler(ReceivedHandler&& handler)
{
    _receivedHandler = CallOnce<ReceivedHandler>(std::move(handler));
}

void Socket::setErrorHandler(ErrorHandler&& handler)
{
    _errorHandler = CallOnce<ErrorHandler>(std::move(handler));
}

void Socket::run()
{
    _stateChangedHandler.callIfSignaled();
    _receivedHandler.callIfSignaled();
    _errorHandler.callIfSignaled();
}

}