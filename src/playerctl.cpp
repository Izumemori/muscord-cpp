#include "playerctl.h"
#include "cancellation_token.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace muscord
{
    Playerctl::Playerctl(PlayerctlEvents* events) {
        GError* error = NULL;
        this->m_events = events;
        this->m_manager = playerctl_player_manager_new(&error);
        this->log_error(this, error);
        
        GList* players = playerctl_list_players(&error);
        this->log_error(this, error);

        players = g_list_copy(players);
        GList* l = NULL;

        for (l = players; l != NULL; l = l->next)
        {
            PlayerctlPlayerName* name = (PlayerctlPlayerName*)l->data;
            PlayerctlPlayer* player = playerctl_player_new_from_name(name, &error);

            this->log_error(this, error);
            
            playerctl_player_manager_manage_player(this->m_manager, player);
            this->init_managed_player(player);
        }
        
        this->m_time_updater_cancel_token = new CancellationToken();
        this->m_time_updater = std::async(std::launch::async, [this]{
                    while (true)
                    {
                        GList* players = NULL;
                        g_object_get(this->m_manager, "players", &players, NULL);
                        
                        auto l = g_list_first(players);
                        auto top_player = PLAYERCTL_PLAYER(l->data);

                        this->send_track_info(top_player);
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        
                        if (this->m_time_updater_cancel_token->cancel) break;
                    }
                });

        this->m_main_loop = g_main_loop_new(NULL, FALSE);
        this->m_main_loop_future = std::async(std::launch::async, [this]{
                   g_main_loop_run(this->m_main_loop); 
                });
    }

    Playerctl::~Playerctl()
    {
        this->m_time_updater_cancel_token->cancel = true;
        g_main_loop_quit(this->m_main_loop);
        this->m_time_updater.wait();
    }

    void Playerctl::init_managed_player(PlayerctlPlayer* player)
    {
        g_signal_connect(G_OBJECT(player), "play", G_CALLBACK(on_play), this);
    }

    void Playerctl::on_play(PlayerctlPlayer* player, gpointer* data)
    {
        Playerctl* playerClass = reinterpret_cast<Playerctl*>(data); 
        playerctl_player_manager_move_player_to_top(playerClass->m_manager, player);
    }

    void Playerctl::send_track_info(PlayerctlPlayer* player)
    {
        GError* error = NULL;

        PlayerState* state = new PlayerState();

        state->artist = playerctl_player_get_artist(player, &error);
        log_error(this, error);
        
        state->title = playerctl_player_get_title(player, &error);
        log_error(this, error);
        
        state->album = playerctl_player_get_album(player, &error);
        log_error(this, error);
        
        gint64 position = 0;        
        g_object_get(player, "position", &position, NULL);
        state->position = position;
        
        PlayerctlPlaybackStatus status = PLAYERCTL_PLAYBACK_STATUS_STOPPED;
        g_object_get(player, "playback-status", &status, NULL);

        switch (status)
        {
            case PLAYERCTL_PLAYBACK_STATUS_STOPPED:
                state->status = PlayerStatus::STOPPED;
            break;

            case PLAYERCTL_PLAYBACK_STATUS_PLAYING:
                state->status = PlayerStatus::PLAYING;
            break;

            case PLAYERCTL_PLAYBACK_STATUS_PAUSED:
                state->status = PlayerStatus::PAUSED;
            break;
        }

        gchar* name = NULL;
        g_object_get(player, "player_name", &name, NULL);
        state->player_name = name;
        
        this->m_events->state_changed(state); 
        delete state;
    }

    void Playerctl::log_error(Playerctl* playerClass, GError* error)
    {
        if (error == NULL) return;
        playerClass->m_events->error(error->message);
        delete error;
    }
}
