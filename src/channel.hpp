#pragma once

#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

namespace mq {

    class session;

    class channel_manager {
    public:
        void subscribe(const std::string& channel, std::weak_ptr<session> subscriber) {
            if (has_channel(channel)) {
                channels_.at(channel).emplace(subscriber);
            }
        }

        void unsubscribe(std::weak_ptr<session> subscriber) {
            for (auto& [ channel_name, subscribers ] : channels_) {
                if (subscribers.contains(subscriber)) {
                    subscribers.erase(subscriber);
                }
            }
        }

        auto& get_subscribers(const std::string& channel) {
            return channels_.at(channel);
        }

        bool has_channel(const std::string& channel) {
            auto it = channels_.find(channel);
            if (it == channels_.end()) {
                return false;
            }

            return true;
        }

        auto begin() { return channels_.begin(); }
        auto end() { return channels_.end(); }
        const auto begin() const { return channels_.begin(); }
        const auto end() const { return channels_.end(); }

    private:
        template <typename T>
        using weak_set = std::set<std::weak_ptr<T>, std::owner_less<std::weak_ptr<T>>>;

        // Channel name -> Subscribed sessions
        std::unordered_map<std::string, weak_set<session>> channels_;
    };

}