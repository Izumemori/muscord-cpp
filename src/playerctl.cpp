#include "playerctl.h"
#include "cancellation_token.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace muscord::playerctl
{
    Playerctl::Playerctl(PlayerctlEvents* events)
    {
        this->m_events = events;
        GError* error = NULL;

        this->m_manager = playerctl_player_manager_new(&error);
        
        if (error != NULL)
            this->m_events->error(error->message);
        
        GList* players = playerctl_list_players(&error);
        
        if (error != NULL)
            this->m_events->error(error->message);

        players = g_list_copy(players);
        
        GList* l = NULL;

        for (l = players; l != NULL; l = l->next)
        {
            PlayerctlPlayerName* name = (PlayerctlPlayerName*)l->data;
            PlayerctlPlayer* player = playerctl_player_new_from_name(name, &error);

            if (error != NULL)
                this->m_events->error(error->message);

            playerctl_player_manager_manage_player(this->m_manager, player);
            this->init_managed_player(player);
        }
        
        this->m_time_updater_cancel_token = new cancellation_token();
        this->m_time_updater = std::async(std::launch::async, [this]{
                    while (true)
                    {
                        GList* players = NULL;
                        g_object_get(this->m_manager, "players", &players, NULL);
                        
                        auto l = g_list_first(players);
                        auto top_player = PLAYERCTL_PLAYER(l->data);

                        on_time_change(top_player, (gpointer*)this);
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
        this->cleanup();
    }

    void Playerctl::cleanup()
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

    void Playerctl::on_time_change(PlayerctlPlayer* player, gpointer* data)
    {
        GError* error = NULL;

        Playerctl* playerClass = (Playerctl*)data;
        
        player_state* state = new player_state();
        state->artist = playerctl_player_get_artist(player, &error);
        log_error(playerClass, error);
        state->title = playerctl_player_get_title(player, &error);
        log_error(playerClass, error);
        state->album = playerctl_player_get_album(player, &error);
        log_error(playerClass, error);
        gint64 position = 0;        
        g_object_get(player, "position", &position, NULL);
        state->position = position;

        PlayerctlPlaybackStatus status = PLAYERCTL_PLAYBACK_STATUS_STOPPED;
        g_object_get(player, "playback-status", &status, NULL);

        gchar* name = NULL;
        g_object_get(player, "player_name", &name, NULL);
        state->player_name = name;

        switch (status)
        {
            case PLAYERCTL_PLAYBACK_STATUS_STOPPED:
                state->status = player_status::STOPPED;
            break;

            case PLAYERCTL_PLAYBACK_STATUS_PLAYING:
                state->status = player_status::PLAYING;
            break;

            case PLAYERCTL_PLAYBACK_STATUS_PAUSED:
                state->status = player_status::PAUSED;
            break;
        }
        
        playerClass->m_events->state_changed(state); 
        delete state;
    }

    void Playerctl::log_error(Playerctl* playerClass, GError* error)
    {
        if (error == NULL) return;
        playerClass->m_events->error(error->message);
        delete error;
    }
}
