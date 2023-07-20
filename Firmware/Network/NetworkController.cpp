#include "NetworkController.h"
#include "TcpSocket.h"

#include <cstdio>

namespace Network
{

void NetworkController::addSocket(Socket* socket)
{
    for (auto i = 0u; i < _sockets.size(); ++i) {
        if (!_sockets[i]) {
            _sockets[i] = socket;
            return;
        }
    }

    printf("NetworkController: failed to add socket %p\n", socket);
}

void NetworkController::removeSocket(Socket* socket)
{
    for (auto i = 0u; i < _sockets.size(); ++i) {
        if (_sockets[i] == socket) {
            _sockets[i] = nullptr;
            return;
        }
    }

    printf("NetworkController: failed to remove socket %p\n", socket);
}

void NetworkController::run()
{
    _wifiController.run();

    if (_wifiController.isConnected()) {
        for (auto* const socket : _sockets) {
            if (!socket) {
                continue;
            }

            socket->run();
        }
    }
}

}