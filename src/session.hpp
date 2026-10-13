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

        static std::shared_ptr<session> create(channel_manager& channels, tcp::socket socket) {
            auto ptr = std::shared_ptr<session>(new session(channels, std::move(socket)));
            ptr->session_bootstrap();
            return ptr;
        }

        asio::awaitable<void> write(const std::string& message) {
            try {
                co_await asio::async_write(
                    socket_, 
                    asio::buffer(message.data(), message.size()), 
                    asio::use_awaitable
                );
            }
            catch (const std::exception& e) {
                std::cerr << "Write failed: " << e.what() << std::endl;
            }  
        }

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

        asio::awaitable<void> start() {
            auto self = shared_from_this();

            try {
                std::string buffer;

                while (true) {
                    // Async read from the client
                    size_t read_bytes = co_await asio::async_read_until(
                        socket_,
                        asio::dynamic_buffer(buffer),
                        '\n',
                        asio::use_awaitable
                    );

                    // Decode and handle message
                    auto [ type, message ] = decode_message(buffer);
                    co_await handle_message(type, message);

                    // If we disconnected, we're done
                    if (!socket_.is_open()) {
                        co_return;
                    }

                    buffer.erase(0, read_bytes);
                }
            } 
            catch (const std::exception& e) {
                channels_.unsubscribe(self);
                std::cout << "Session closed: " << e.what() << std::endl;
            }
        }

        asio::awaitable<void> handle_message(message_type type, const std::string& message) {
            auto self = shared_from_this();

            if (type == message_type::CONNECT && !is_authenticated_) {
                is_authenticated_ = true;
                co_await write("OK\n");
                co_return;
            }

            if (!is_authenticated_) {
                co_await write("ERROR: Not authenticated\n");
                co_return;
            }

            switch (type) {
            case message_type::SUBSCRIBE: {
                // Message is the topic channel, strip all extra characters
                std::string channel_name = message;
                std::erase_if(channel_name, [](unsigned char c) {
                    return std::isspace(c);
                });

                if (!channel_name.empty()) {
                    channels_.subscribe(channel_name, self);

                    // Replay "backlogged" messages to the new subscriber
                    channel& channel = channels_[channel_name];
                    for (const auto& logged_msg : channel.get_cache()) {
                        co_await write("[BACKLOG] " + logged_msg);
                    }
                }
            } break;

            case message_type::DISCONNECT: {
                channels_.unsubscribe(self);
                socket_.shutdown(tcp::socket::shutdown_send);
                socket_.close();

                co_return;
            }

            case message_type::PUBLISH: {
                size_t separator_pos = message.find(':');
                if (separator_pos == std::string::npos) break;

                // Broadcast the message to all subscribed sessions, skip itself
                std::string channel_name = message.substr(0, separator_pos);
                std::string payload = message.substr(separator_pos + 1);

                // TODO: Lazy loading channels could be an attack vector later...
                channel& target_channel = channels_[channel_name]; 
                target_channel.cache_message(payload);

                for (auto& session_weak : target_channel.get_subscribers()) {
                    auto session_strong = session_weak.lock();
                    if (!session_strong || session_strong == self) continue;

                    co_await session_strong->write(payload);
                }
            } break;
            
            default: break;
            }
        }

    private:
        bool is_authenticated_ = false;
        channel_manager& channels_;
        tcp::socket socket_;
    };

}