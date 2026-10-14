#include "handlers.hpp"

namespace mq {

    asio::awaitable<void> handle_connect(handler_context& context) {
        context.session->authenticate();
        co_await context.session->write("CONNACK\n");
    }

    asio::awaitable<void> handle_subscribe(handler_context& context) {
        // Message is the topic channel, strip all extra characters
        std::string channel_name = context.message;
        std::erase_if(channel_name, [](unsigned char c) {
            return std::isspace(c);
        });

        if (!channel_name.empty()) {
            context.channels.subscribe(channel_name, context.session);

            co_await context.session->write("SUBACK\n");

            // Replay "backlogged" messages to the new subscriber
            channel& channel = context.channels[channel_name];
            for (const auto& logged_msg : channel.get_cache()) {
                co_await context.session->write("[BACKLOG] " + logged_msg);
            }
        }
    }

    asio::awaitable<void> handle_publish(handler_context& context) {
        size_t separator_pos = context.message.find(':');
        if (separator_pos == std::string::npos) co_return;

        // Broadcast the message to all subscribed sessions, skip itself
        std::string channel_name = context.message.substr(0, separator_pos);
        std::string payload = context.message.substr(separator_pos + 1);

        // TODO: Lazy loading channels could be an attack vector later...
        channel& target_channel = context.channels[channel_name]; 
        target_channel.cache_message(payload);

        for (auto& session_weak : target_channel.get_subscribers()) {
            auto session_strong = session_weak.lock();
            if (!session_strong || session_strong == context.session) continue;

            co_await session_strong->write(payload);
        }

        co_await context.session->write("PUBACK\n");
    }

    asio::awaitable<void> handle_disconnect(handler_context& context) {
        context.channels.unsubscribe(context.session);
        context.session->end();

        co_return;
    }

}