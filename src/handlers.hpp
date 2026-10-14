#pragma once

#include <asio.hpp>

#include "session.hpp"

namespace mq {

    struct handler_context {
        std::shared_ptr<session>& session;
        channel_manager& channels;
        const std::string& message;
    };
    
    using handler_fn = asio::awaitable<void>(*)(handler_context&);

    asio::awaitable<void> handle_connect(handler_context& context);
    asio::awaitable<void> handle_subscribe(handler_context& context);
    asio::awaitable<void> handle_publish(handler_context& context);
    asio::awaitable<void> handle_disconnect(handler_context& context);
}