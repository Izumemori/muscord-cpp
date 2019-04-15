#pragma once

#include "../include/discord_rpc.h"
#include "cancellation_token.h"
#include "log_message.h"
#include <iostream>
#include <functional>
#include <future>

namespace muscord
{
    class MuscordRpc {
        public:
            MuscordRpc(const std::string application_id, DiscordEventHandlers* handlers);
            void connect();
            void disconnect();
            void update_presence(const std::function<void (DiscordRichPresence*)>& func);
            void clear_presence();
            std::function<void(LogMessage*)> log;
            ~MuscordRpc();
        private:
            std::string m_application_id;
            DiscordEventHandlers* m_handlers;
            CancellationToken* m_callback_token;
            std::future<void> m_callback_future;
            void start_callback_checks();
            void stop_callback_checks();
    };
}
