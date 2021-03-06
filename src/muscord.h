#pragma once
#include "muscord_rpc.h"
#include "muscord_config.h"
#include "muscord_events.h"
#include "muscord_state.h"
#include "playerctl.h"
#include "../discord-rpc/include/discord_rpc.h"
#include <vector>
#include <memory>

namespace muscord {
    class Muscord {
        public:
            Muscord(std::shared_ptr<MuscordConfig>& config, std::unique_ptr<MuscordEvents>& handlers);
            void run();
            void stop();
        private:
            std::shared_ptr<DiscordEventHandlers> m_discord_events;
            std::shared_ptr<MuscordState> m_state;
            std::shared_ptr<MuscordConfig> m_config;
            std::unique_ptr<MuscordEvents> m_handlers;
            std::unique_ptr<MuscordRpc> m_rpc;
            std::unique_ptr<Playerctl> m_player;
            std::future<void> m_idle_check_future;
            std::unique_ptr<CancellationToken> m_idle_check_token;
            void on_ready(const DiscordUser* user);
            void on_disconnected(int error_code, const char* message);
            void on_errored(int error_code, const char* message);
            void on_state_change(const PlayerState& state);
            void create_new_playerctl();
    };
}
