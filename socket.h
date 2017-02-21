/*
    NC-Net
    Copyright (c) 2017 by NuclearC
*/

#ifndef NC_SOCKET_H_
#define NC_SOCKET_H_

#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <functional>

#if defined WIN32 || defined _WIN32
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#endif // WIN32 || _WIN32

namespace nc {

    enum socket_error {
        invalid_state,
        socket_error_state,
    };

    enum socket_state {
        open,
        closed
    };

    class nc_exception
        : public std::exception {
    public:
        nc_exception(const std::string& msg, socket_error code_)
            : std::exception(msg.c_str()), code(code_) {}

        socket_error code;
    };

    class socket_client {
    public:
        socket_client();
        socket_client(const socket_client& other_);
        socket_client(const std::string& url_);
        socket_client(const std::string& ip_, const std::string& port_);
        ~socket_client();

        void connect(int mode = 6, bool blocking = false);

        void send(const char* src_, const std::size_t& size_);
        void send(const std::vector<unsigned char>& packet_);
        void send(const std::vector<char>& packet_);
        void send(const std::string& packet_);
        void shutdown_send();
        void shutdown_recv();
        void shutdown_both();

        const int& poll(char* dest_, const std::size_t& max_size_);

        void close();
        void destroy();

        const socket_state& get_state() const { return state; }

        std::function<void(socket_client*)> on_open;
        std::function<void(socket_client*)> on_close;
    private:
        void init_socket();

        std::string url;
        std::string ip;
        std::string port;

        socket_state state;

        SOCKET sfd = 0;
    };
} // namespace nc

#endif // NC_SOCKET_H
