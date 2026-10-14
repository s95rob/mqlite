#include "session.hpp"
#include "dispatch.hpp"

namespace mq {

    asio::awaitable<void> session::write(const std::string& message) {
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

    asio::awaitable<void> session::start() {
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

                // Decode and dispatch message
                auto [ type, message ] = decode_message(buffer);
                handler_context context{self, channels_, message};
                co_await dispatch_message(type, context);

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

}