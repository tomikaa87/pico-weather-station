#include "TcpSocket.h"
#include "NetworkController.h"

#include <cstring>

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

bool TcpSocket::disconnect()
{
    if (_state == State::Idle) {
        return false;
    }

    if (tcp_close(_tcpPcb) != ERR_OK) {
        return false;
    }

    _state = State::Idle;
    _tcpPcb = nullptr;
    _stateChangedHandler.signal(Socket::State::Disconnected);

    return true;
}

std::vector<std::byte> TcpSocket::read()
{
    std::vector<std::byte> v;
    std::swap(v, _rxData);
    return v;
}

bool TcpSocket::write(std::vector<std::byte>&& data)
{
    if (_state != State::Connected) {
        return false;
    }

    if (data.empty() || !_txData.empty()) {
        return false;
    }

    _txData = std::move(data);

    return true;
}

void TcpSocket::run()
{
    switch (_state) {
        case State::ConnectCalled: {
            doConnect();
            break;
        }

        case State::Connected: {
            doSend();
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

    _txPaused = false;

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

    cyw43_arch_lwip_check();

    if (p && p->tot_len > 0) {
        if (_rxData.size() + p->tot_len > RxBufSize) {
            puts("TcpSocket: received data too large");
            pbuf_free(p);
            return ERR_ABRT;
        }

        auto offset = _rxData.size();
        _rxData.resize(offset + p->tot_len);

        for (auto* buf = p; buf != nullptr; buf = buf->next) {
            std::memcpy(&_rxData[offset], buf->payload, buf->len);
            offset += buf->len;
        }

        tcp_recved(_tcpPcb, p->tot_len);
    }

    if (p) {
        pbuf_free(p);
    }

    _receivedHandler.signal();

    return ERR_OK;
}

void TcpSocket::err(const err_t err)
{
    printf("TcpSocket: error, err=%d\n", err);

    _state = State::Idle;
    _stateChangedHandler.signal(Socket::State::Disconnected);
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

void TcpSocket::doSend()
{
    if (_txPaused) {
        return;
    }

    if (_state != State::Connected) {
        return;
    }

    if (_txData.empty() || _txOffset >= _txData.size()) {
        return;
    }

    const std::size_t remaining = _txData.size() - _txOffset;
    const std::size_t chunkSize = std::min(static_cast<std::size_t>(tcp_sndbuf(_tcpPcb)), remaining);
    const std::byte* chunk = _txData.data() + _txOffset;
    const auto last = remaining == chunkSize;
    const auto flags = !last ? TCP_WRITE_FLAG_MORE : 0;

    printf("TcpSocket::doSend: _txOffset=%u, remaining=%u, flags=%d, chunkSize=%u, chunk=", _txOffset, remaining, flags, chunkSize);
    for (auto i = 0u; i < chunkSize; ++i) {
        printf("%02X ", *(chunk + i));
    };
    puts("");

    cyw43_arch_lwip_check();

    if (const auto e = tcp_write(_tcpPcb, chunk, chunkSize, flags); e != ERR_OK) {
        if (e == ERR_MEM) {
            _txPaused = true;
        } else {
            decltype(_txData) v;
            std::swap(_txData, v);
            _txOffset = 0;
        }
    } else {
        _txOffset += chunkSize;
    }
}

}