/*
    NC-Net
    Copyright (c) 2017 by NuclearC
*/

#ifndef NC_HTTP_H_
#define NC_HTTP_H_

#define BUFFER_SIZE 8192

#include <map>
#include <thread>
#include <chrono>
#include <regex>

#include "socket.h"

namespace nc {

    struct http_data {
        std::string method;
        std::string body;
        std::map<std::string, std::string> headers;
        unsigned short code;
    };

    std::pair<std::string, std::string> split(const std::string& str, char delimiter) {
        if (str.find(delimiter) < 0)
            return{};

        std::string str1 = str.substr(0, str.find(delimiter));
        std::string str2 = str.substr(str.find(delimiter), str.length() - str.find(delimiter));

        for (int i = 0; i < str2.size(); i++) {
            if (str2[i] == 0x20) {
                str2.erase(str2.begin() + i);
                i--;
            }
            else if (str2[i] == delimiter) {
                str2.erase(str2.begin() + i);
                i--;
            }
            else {
                break;
            }
        }

        return std::make_pair(str1, str2);
    }

    void http_parse(const std::string& raw_text, http_data& result) {
        result.headers.clear();
        result.body.clear();
        result.method.clear();

        // parse response code
        std::istringstream f(raw_text);
        std::string line;

        int http_code;
        char http_string[256];

        int res = 0;

        if (std::getline(f, line)) {
            res = sscanf(line.c_str(), "HTTP/1.1 %d %s", &http_code, &http_string);
            if (res != 2) {
                return;
            }
            else {
                result.code = http_code;
            }
        }

        while (std::getline(f, line)) {
            if (line.find(':') != std::string::npos) {
                result.headers.insert(split(line, ':'));
            }
            else if (line.find('\r') < 2) {
                break;
            }
        }

        while (std::getline(f, line)) {
            result.body.append(line);
        }
    }

    http_data http_send(const std::string& url,
        const std::string& uri, const http_data& request) {
        socket_client client(url, "80");

        http_data response;

        std::string response_raw;

        client.on_open = [&](socket_client* sender) mutable {
            
            std::stringstream buffer;

            buffer << request.method << " " << uri << " HTTP/1.1\r\n";

            for (auto it = request.headers.begin();
                it != request.headers.end(); it++) {
                buffer << it->first;
                buffer << ": ";
                buffer << it->second;
                buffer << "\r\n";
            }

            buffer << "\r\n";

            sender->send(buffer.str());

            int size = 0;
            char dest[BUFFER_SIZE];

            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));

                size = sender->poll(dest, BUFFER_SIZE);

                if (size > 0)
                    break;
            }

            response_raw = dest;
            response_raw.resize(size);

            sender->close();
        };

        client.connect();

        http_parse(response_raw, response);

        return response;
    }

} // namespace nc

#endif // NC_HTTP_H_
