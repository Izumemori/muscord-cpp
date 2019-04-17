#pragma once

#include "../discord-rpc/include/discord_rpc.h"
#include "cancellation_token.h"
#include "log_message.h"
#include <iostream>
#include <functional>
#include <future>
#include <memory>

namespace muscord
{
    class MuscordRpc {
        public:
            MuscordRpc(std::string application_id, std::shared_ptr<DiscordEventHandlers>& handlers);
            void connect();
            void disconnect();
            void update_presence(const std::function<void (DiscordRichPresence*)>& func);
            void clear_presence();
            bool connected;
            std::function<void(LogMessage&)> log;
            ~MuscordRpc();
        private:
            std::string m_application_id;
            std::shared_ptr<DiscordEventHandlers> m_handlers;
            std::unique_ptr<CancellationToken> m_callback_token;
            std::future<void> m_callback_future;
            void start_callback_checks();
            void stop_callback_checks();
    };
}
