#pragma once

#include <asio.hpp>

#include "protocol.hpp"
#include "session.hpp"
#include "channel.hpp"
#include "handlers.hpp"

namespace mq {

    asio::awaitable<void> dispatch_message(message_type type, handler_context& context);

}