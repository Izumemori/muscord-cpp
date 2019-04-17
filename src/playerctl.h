#pragma once

#include "cancellation_token.h"
#include "log_message.h"
#include <future>
#include <memory>

extern "C" {
    #include <playerctl/playerctl.h>
}

namespace muscord {
    typedef enum PlayerStatus {
        PAUSED,
        PLAYING,
        STOPPED
    } PlayerStatus;
    
    typedef struct PlayerState {
        std::string artist;
        std::string title;
        std::string album;
        std::string player_name;
        uint64_t position;
        uint64_t total;
        PlayerStatus status;
    } PlayerState;
    
    typedef struct PlayerctlEvents {    
        std::function<void(std::string&)> error;
        std::function<void(PlayerState&)> state_changed;
        std::function<void(LogMessage&)> log;
    } PlayerctlEvents;
    
    class Playerctl {
        public:
            Playerctl(std::unique_ptr<PlayerctlEvents>& events);
            ~Playerctl();
        private:
            PlayerctlPlayerManager* m_manager;
            GMainLoop* m_main_loop;
            std::future<void> m_time_updater;
            std::future<void> m_main_loop_future;
            std::unique_ptr<PlayerctlEvents> m_events;
            std::unique_ptr<CancellationToken> m_time_updater_cancel_token;
            void init_managed_player(PlayerctlPlayer* player);
            void send_track_info(PlayerctlPlayer* player);
            static void on_play(PlayerctlPlayer* player, gpointer* data);
            static void log_error(Playerctl*, GError*);
            static void on_name_appeared(PlayerctlPlayerManager* manager, PlayerctlPlayerName* name, gpointer* data);
            static void on_player_appeared(PlayerctlPlayerManager* manager, PlayerctlPlayer* player, gpointer* data);
            static void on_player_vanished(PlayerctlPlayerManager* manager, PlayerctlPlayer* player, gpointer* data);
    };
}    
