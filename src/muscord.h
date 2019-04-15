#pragma once
#include "muscord_rpc.h"
#include "playerctl.h"
#include "../include/discord_rpc.h"
#include <vector>

namespace muscord {
    typedef struct MuscordState {
        PlayerStatus status;
        std::string artist;
        std::string title;
        std::string album;
        std::string player_name;
        uint64_t time;

        MuscordState();
        MuscordState(PlayerState* state);

        bool equals(MuscordState* other);
    } MuscordState;

    typedef struct MuscordEvents {
        std::function<void(LogMessage*)> log;
        std::function<void(MuscordState*, PlayerStatus, DiscordRichPresence*)> play_state_change;
        std::function<void(const DiscordUser*)> ready; 
    } MuscordEvents;

    typedef struct MuscordConfig {
        std::string application_id;
        std::vector<std::string> ignored_players;
        bool auto_reconnect;

        MuscordConfig() : auto_reconnect(true)
        {};
    } MuscordConfig;

    class Muscord {
        public:
            Muscord(MuscordConfig* config, MuscordEvents* handlers);
            void run();
            void stop();
        private:
            MuscordState* m_state;
            MuscordConfig* m_config;
            MuscordEvents* m_handlers;
            MuscordRpc* m_rpc;
            Playerctl* m_player;
            DiscordEventHandlers* m_discord_events;
            void on_ready(const DiscordUser* user);
            void on_disconnected(int error_code, const char* message);
            void on_errored(int error_code, const char* message);
            void on_state_change(PlayerState* state);
            void create_new_playerctl();
    };
}
