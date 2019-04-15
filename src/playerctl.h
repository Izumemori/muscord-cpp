#pragma once

#include "cancellation_token.h"
#include <future>

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
        std::function<void(std::string)> error;
        std::function<void(PlayerState*)> state_changed;
    } PlayerctlEvents;
    
    class Playerctl {
        public:
            Playerctl(PlayerctlEvents* events);
            ~Playerctl();
        private:
            PlayerctlPlayerManager* m_manager;
            GMainLoop* m_main_loop;
            std::future<void> m_time_updater;
            std::future<void> m_main_loop_future;
            PlayerctlEvents* m_events;
            CancellationToken* m_time_updater_cancel_token;
            void init_managed_player(PlayerctlPlayer* player);
            void send_track_info(PlayerctlPlayer*);
            static void on_play(PlayerctlPlayer*, gpointer*);
            static void log_error(Playerctl*, GError*);
    };
}    
