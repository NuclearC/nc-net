/*
    NC-Net
    Copyright (c) 2017 by NuclearC
*/

#include "socket.h"

namespace nc {
    socket_client::socket_client()
    {
    }
    socket_client::socket_client(const socket_client & other_)
    {
    }

    socket_client::socket_client(const std::string & url_) : url(url_) {}

    socket_client::socket_client(const std::string & ip_, const std::string& port_)
        : url(ip_ + ":" + port_), ip(ip_), port(port_) {}

    socket_client::~socket_client() { if (state == socket_state::open) close(); destroy(); }

    void socket_client::connect(int mode, bool blocking)
    {
        addrinfo hints = {};
        addrinfo* result;

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = mode;

        const char* ip_c = ip.c_str();
        const char* port_c = port.c_str();

        int status = getaddrinfo(ip_c, port_c, &hints, &result);

        if (status != 0) {
            throw nc_exception("getaddrinfo failed", socket_error::socket_error_state);
        }

        sfd = ::socket(result->ai_family, result->ai_socktype,
            result->ai_protocol);

        if (sfd == INVALID_SOCKET) {
            throw nc_exception("socket failed", socket_error::socket_error_state);
        }

        if (::connect(sfd, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
            closesocket(sfd);
            sfd = INVALID_SOCKET;
            state = socket_state::closed;
            if (on_close)
                on_close(this);
            return;
        }

        unsigned long socket_mode = blocking;

        ioctlsocket(sfd, FIONBIO, &socket_mode);

        freeaddrinfo(result);

        if (sfd == INVALID_SOCKET) {
            state = socket_state::closed;
            if (on_close)
                on_close(this);
            return;
        }

        state = socket_state::open;
        if (on_open)
            on_open(this);
    }

    void socket_client::send(const char * src_, const std::size_t & size_)
    {
        ::send(sfd, src_, size_, 0);
    }

    void socket_client::send(const std::vector<unsigned char>& packet_)
    {
        send((const char*)packet_.data(), packet_.size());
    }

    void socket_client::send(const std::string & packet_)
    {
        send(packet_.data(), packet_.size());
    }

    void socket_client::shutdown_send()
    {
        shutdown(sfd, SD_SEND);
    }

    void socket_client::shutdown_recv()
    {
        shutdown(sfd, SD_RECEIVE);
    }

    void socket_client::shutdown_both()
    {
        shutdown(sfd, SD_BOTH);
    }

    const int & socket_client::poll(char * dest_, const std::size_t& max_size_)
    {
        return ::recv(sfd, dest_, max_size_, 0);
    }

    void socket_client::close()
    {
        if (state != socket_state::open)
            throw nc_exception("", socket_error::invalid_state);

        ::shutdown(sfd, SD_BOTH);
        ::closesocket(sfd);

        state = socket_state::closed;

        if (on_close)
            on_close(this);
    }

    void socket_client::destroy()
    {
    }

    void socket_client::init_socket()
    {
    }
} // namespace nc