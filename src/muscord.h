#pragma once
#include "muscord_rpc.h"
#include "playerctl.h"
#include "../discord-rpc/include/discord_rpc.h"
#include <vector>
#include <memory>

namespace muscord {
    typedef struct MuscordState {
        PlayerStatus status;
        std::string artist;
        std::string title;
        std::string album;
        std::string player_name;
        uint64_t time;

        MuscordState();
        MuscordState(PlayerState& state);

        bool equals(MuscordState& other);
    } MuscordState;

    typedef struct MuscordEvents {
        std::function<void(LogMessage&)> log;
        std::function<void(MuscordState&, PlayerStatus, DiscordRichPresence*)> play_state_change;
        std::function<void(const DiscordUser*)> ready; 
    } MuscordEvents;

    typedef struct MuscordConfig {
        std::string application_id;
        bool disconnect_on_idle;
        int64_t idle_timeout_ms;
        std::vector<std::string> ignored_players;
        
        MuscordConfig() : disconnect_on_idle(true), idle_timeout_ms(30000)
        {}
    } MuscordConfig;

    class Muscord {
        public:
            Muscord(std::unique_ptr<MuscordConfig>& config, std::unique_ptr<MuscordEvents>& handlers);
            void run();
            void stop();
        private:
            std::shared_ptr<DiscordEventHandlers> m_discord_events;
            std::shared_ptr<MuscordState> m_state;
            std::unique_ptr<MuscordConfig> m_config;
            std::unique_ptr<MuscordEvents> m_handlers;
            std::unique_ptr<MuscordRpc> m_rpc;
            std::unique_ptr<Playerctl> m_player;
            std::future<void> m_idle_check_future;
            std::unique_ptr<CancellationToken> m_idle_check_token;
            void on_ready(const DiscordUser* user);
            void on_disconnected(int error_code, const char* message);
            void on_errored(int error_code, const char* message);
            void on_state_change(PlayerState& state);
            void create_new_playerctl();
    };
}
