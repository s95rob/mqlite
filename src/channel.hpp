#pragma once

#include <algorithm>
#include <string>
#include <unordered_map>
#include <deque>

#include "common.hpp"

namespace mq {

    class session;

    class channel {
    public:
        void subscribe(std::weak_ptr<session> subscriber) {
            subscribers_.emplace(subscriber);
        }

        void unsubscribe(std::weak_ptr<session> subscriber) {
            if (subscribers_.contains(subscriber)) {
                subscribers_.erase(subscriber);
            }
        }

        bool has_subscriber(std::weak_ptr<session> subscriber) const {
            return subscribers_.contains(subscriber);
        }

        void cache_message(const std::string& message) {
            if (message_cache_.size() >= MESSAGE_CACHE_SIZE_MAX) {
                message_cache_.pop_front();
            }

            message_cache_.push_back(message);
        }

        const auto& get_subscribers() const { return subscribers_; }
        const auto& get_cache() const { return message_cache_; }
        
    private:
        static constexpr size_t MESSAGE_CACHE_SIZE_MAX = 100;
        weak_ptr_set<session> subscribers_;
        std::deque<std::string> message_cache_;
    };

    class channel_manager {
    public:
        void subscribe(const std::string& channel_name, std::weak_ptr<session> subscriber) {
            if (has_channel(channel_name)) {
                channels_.at(channel_name).subscribe(subscriber);
            }
        }

        void unsubscribe(std::weak_ptr<session> subscriber) {
            for (auto& [ channel_name, channel ] : channels_) {
                channel.unsubscribe(subscriber);
            }
        }

        auto& get_channel(const std::string& channel_name) {
            return channels_.at(channel_name);
        }

        bool has_channel(const std::string& channel_name) {
            auto it = channels_.find(channel_name);
            if (it == channels_.end()) {
                return false;
            }

            return true;
        }

        auto begin() { return channels_.begin(); }
        auto end() { return channels_.end(); }
        const auto begin() const { return channels_.begin(); }
        const auto end() const { return channels_.end(); }

        channel& operator[](const std::string& channel_name) { return channels_[channel_name]; }

    private:
        std::unordered_map<std::string, channel> channels_;
    };

}