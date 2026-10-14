#pragma once

#include <string>
#include <cstring>

namespace mq {

    enum class message_type {
        UNKNOWN = 0,
        CONNECT,
        DISCONNECT,
        PUBLISH,
        SUBSCRIBE,

        COUNT
    };

    // message_type -> prefix substring
    static const char* MESSAGE_PREFIXES[] = {
        0,  // message_type::UNKNOWN
        "CONNECT",
        "DISCONNECT",
        "PUBLISH",
        "SUBSCRIBE"
    };

    inline std::pair<message_type, std::string> decode_message(const std::string& data) {
        size_t num_types = static_cast<size_t>(message_type::COUNT);
        for (size_t i = 1; i < num_types; i++) {
            size_t prefix_len = std::strlen(MESSAGE_PREFIXES[i]);
            if (data.compare(0, prefix_len, MESSAGE_PREFIXES[i]) == 0) {
                return { static_cast<message_type>(i), data.substr(prefix_len + 1, data.size()) };
            }
        }

        return { message_type::UNKNOWN, {} };
    }

}