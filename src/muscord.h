#include "../include/discord_rpc.h"
#include "cancellation_token.h"
#include <functional>
#include <future>
#pragma once
namespace muscord
{
    class Muscord {
        public:
            Muscord(const char* application_id, DiscordEventHandlers* handlers);
            void update_presence(const std::function<void (DiscordRichPresence*)>& func);
            void (*log)(const char* message);
            ~Muscord();
        private:
            cancellation_token* callback_token;
            std::future<void> callback_future;
            void start_callback_checks();
            void stop_callback_checks();
    };
};
