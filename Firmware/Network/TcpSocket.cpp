#include "TcpSocket.h"
#include "NetworkController.h"

#include <pico/cyw43_arch.h>

namespace Network
{

TcpSocket::TcpSocket(NetworkController& controller)
    : Socket{ controller }
{}

bool TcpSocket::connect(const char* const host, const uint16_t port)
{
    if (_state != State::Idle) {
        puts("TcpSocket: connect failed, not in Idle state");
        return false;
    }

    if (!ip4addr_aton(host, &_remoteAddr)) {
        puts("TcpSocket: connect failed, address can't be converted");
        return false;
    }

    _port = port;

    _tcpPcb = tcp_new_ip_type(IP_GET_TYPE(&netState.remoteAddr));
    if (!_tcpPcb) {
        puts("TcpSocket: connect failed, PCB can't be created");
        return false;
    }

    tcp_arg(_tcpPcb, this);

    tcp_poll(
        _tcpPcb,
        [](auto* arg, auto* /* tpcb */) {
            return reinterpret_cast<TcpSocket*>(arg)->poll();
        },
        PoolTimeSeconds * 2
    );

    tcp_sent(
        _tcpPcb,
        [](auto* arg, auto* /* tpcb */, auto length) {
            return reinterpret_cast<TcpSocket*>(arg)->sent(length);
        }
    );

    tcp_recv(
        _tcpPcb,
        [](auto* arg, auto* /* tpcb */, auto* pbuf, auto err) {
            return reinterpret_cast<TcpSocket*>(arg)->recv(pbuf, err);
        }
    );

    tcp_err(
        _tcpPcb,
        [](auto* arg, auto err) {
            reinterpret_cast<TcpSocket*>(arg)->err(err);
        }
    );

    _state = State::ConnectCalled;

    return true;
}

void TcpSocket::read()
{}

void TcpSocket::write()
{}

void TcpSocket::run()
{
    switch (_state) {
        case State::ConnectCalled: {
            doConnect();
            break;
        }

        default:
            break;
    }

    Socket::run();
}

err_t TcpSocket::poll()
{
    puts("TcpSocket: poll");
    return ERR_OK;
}

err_t TcpSocket::sent(const u16_t len)
{
    printf("TcpSocket: sent, len=%u\n", len);
    return ERR_OK;
}

err_t TcpSocket::recv(struct pbuf* p, const err_t err)
{
    printf("TcpSocket: recv, err=%d\n", err);

    _receivedHandler.signal();

    return ERR_OK;
}

void TcpSocket::err(const err_t err)
{
    printf("TcpSocket: error, err=%d\n", err);

    _errorHandler.signal();

    _state = State::Idle;
}

err_t TcpSocket::connected(const err_t /* err */)
{
    puts("TcpSocket: connected");
    _state = State::Connected;

    _stateChangedHandler.signal(Socket::State::Connected);

    return ERR_OK;
}

void TcpSocket::doConnect()
{
    puts("TcpSocket: doConnect");

    cyw43_arch_lwip_begin();

    const auto err = tcp_connect(
        _tcpPcb,
        &_remoteAddr,
        _port,
        [](auto* arg, auto* /* tpcb */, auto err) {
            return reinterpret_cast<TcpSocket*>(arg)->connected(err);
        }
    );

    cyw43_arch_lwip_end();

    _lastConnectMillis = to_ms_since_boot(get_absolute_time());

    if (err != 0) {
        printf("TcpSocket: connect failed, error=%d\n", err);

        _state = State::Idle;
        tcp_close(_tcpPcb);
        _tcpPcb = nullptr;

        return;
    }

    _state = State::Connecting;
    _stateChangedHandler.signal(Socket::State::Connecting);
}

}