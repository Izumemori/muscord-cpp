#pragma once

#include "cancellation_token.h"
#include <future>

extern "C" {
    #include <playerctl/playerctl.h>
}

namespace muscord::playerctl {
    enum player_status {
        PAUSED,
        PLAYING,
        STOPPED
    } typedef player_status;
    
    struct player_state {
        std::string artist;
        std::string title;
        std::string album;
        std::string player_name;
        uint64_t position;
        uint64_t total;
        player_status status;
    } typedef player_state;
    
    struct PlayerctlEvents {    
        void (*error)(const char* message);
        void (*state_changed)(player_state* state);
    } typedef PlayerctlEvents;
    
    class Playerctl {
        public:
            Playerctl(PlayerctlEvents* events);
            ~Playerctl();
        private:
            void init_managed_player(PlayerctlPlayer* player);
            void cleanup();
            PlayerctlPlayerManager* m_manager;
            GMainLoop* m_main_loop;
            std::future<void> m_time_updater;
            std::future<void> m_main_loop_future;
            PlayerctlEvents* m_events;
            cancellation_token* m_time_updater_cancel_token;
            static void on_play(PlayerctlPlayer*, gpointer*);
            static void on_metadata(PlayerctlPlayer*, gpointer*);
            static void on_time_change(PlayerctlPlayer*, gpointer*);
            static void log_error(Playerctl*, GError*);
    };
}    
