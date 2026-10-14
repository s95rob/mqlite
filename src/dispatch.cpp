#include "dispatch.hpp"
#include "handlers.hpp"

#include <array>

namespace mq {

    // Using a function table over strategizing handlers into
    // virtual methods avoids runtime vtable traversal overhead.

    struct dispatch_entry {
        handler_fn func = nullptr;
        bool requires_auth = true;
    };

    static constexpr dispatch_entry dispatch_table_[static_cast<size_t>(message_type::COUNT)] {
        {}, // message_type::UNKNOWN
        { handle_connect, false },
        { handle_disconnect },
        { handle_publish },
        { handle_subscribe }
    };

    asio::awaitable<void> dispatch_message(message_type type, handler_context& context) {
        size_t index = static_cast<size_t>(type);
        size_t table_len = static_cast<size_t>(message_type::COUNT);

        if (index >= table_len || !dispatch_table_[index].func) {
            co_return;
        }

        // Validate authentication
        if (dispatch_table_[index].requires_auth && !context.session->is_authenticated()) {
            co_await context.session->write("ERROR: Not authenticated\n");
            co_return;
        }

        co_await dispatch_table_[index].func(context);
    }

}