#pragma once

#include <algorithm>
#include <cctype>
#include <iostream>
#include <set>
#include <unordered_map>

#include <asio.hpp>

#include "protocol.hpp"
#include "channel.hpp"

namespace mq {

    using asio::ip::tcp;

    class session : public std::enable_shared_from_this<session> {
    public:

        // Create a bootstrapped session
        static std::shared_ptr<session> create(channel_manager& channels, tcp::socket socket) {
            auto ptr = std::shared_ptr<session>(new session(channels, std::move(socket)));
            ptr->session_bootstrap();
            return ptr;
        }

        // Write message to the connection
        asio::awaitable<void> write(const std::string& message);

        // Shut down the session socket and close the connection
        void end() {
            socket_.shutdown(tcp::socket::shutdown_send);
            socket_.close();
        }

        void authenticate() { is_authenticated_ = true; }

        bool is_authenticated() const { return is_authenticated_; }

    private:

        session(channel_manager& channels, tcp::socket socket)
            : channels_(channels), 
              socket_(std::move(socket)) {}

        void session_bootstrap() {
            auto self = shared_from_this();

            asio::co_spawn(
                socket_.get_executor(),
                [self]() -> asio::awaitable<void> {
                    co_await self->start();
                },
                asio::detached
            );
        }

        asio::awaitable<void> start();

    private:
        bool is_authenticated_ = false;
        channel_manager& channels_;
        tcp::socket socket_;
    };

}