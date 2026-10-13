#include <iostream>

#include <asio.hpp>

#include "protocol.hpp"
#include "session.hpp"

namespace mq {

    using asio::ip::tcp;

    constexpr uint16_t DEFAULT_PORT = 9090;

    channel_manager manager;

    asio::awaitable<void> listener(uint16_t port) {
        auto executor = co_await asio::this_coro::executor;
        
        tcp::acceptor acceptor(executor, {tcp::v4(), port});
        std::cout << "Accepting connections on port " << port << std::endl;

        while (true) {
            tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
            
            session::create(manager, std::move(socket));
        }
    }
}


int main(int argc, const char** argv) {
    uint16_t port = mq::DEFAULT_PORT;

    if (argc == 2) {
        port = std::stoi(argv[1]);
    }

    try {
        asio::io_context io_context;

        asio::co_spawn(io_context, mq::listener(port), asio::detached);

        io_context.run();
    } 
    catch (const std::exception& e) {
        std::cerr << "Fatal exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}