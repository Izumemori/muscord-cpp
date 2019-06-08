#include "playerctl.h"
#include "common.h"
#include "cancellation_token.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include <vector>
#include <algorithm>

namespace muscord
{
    Playerctl::Playerctl(std::unique_ptr<PlayerctlEvents>& events, std::vector<std::string>& blacklist) {
        GError* error = NULL;
        this->m_events = std::move(events);
        this->m_manager = playerctl_player_manager_new(&error);
        this->m_blacklist = blacklist;
        this->log_error(error);
        
        GList* players = playerctl_list_players(&error);
        this->log_error(error);

        players = g_list_copy(players);
        GList* l = NULL;

        for (l = players; l != NULL; l = l->next)
        {
            PlayerctlPlayerName* name = reinterpret_cast<PlayerctlPlayerName*>(l->data);
            
            if (vector_contains<std::string>(this->m_blacklist, to_title_case(name->name))) continue;
            
            PlayerctlPlayer* player = playerctl_player_new_from_name(name, &error);

            this->log_error(error);


            playerctl_player_manager_manage_player(this->m_manager, player);
            this->init_managed_player(player);
        }
       

        this->m_time_updater_cancel_token = std::make_unique<CancellationToken>();
        this->m_time_updater = std::async(std::launch::async, [this]{
                    while (true)
                    {
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        
                        GList* players = NULL;
                        g_object_get(this->m_manager, "players", &players, NULL);
                        
                        auto l = g_list_first(players);
                        
                        if (this->m_time_updater_cancel_token->cancel) break;
                        
                        if (!l) continue;
                        
                        auto top_player = PLAYERCTL_PLAYER(l->data);

                        this->send_track_info(top_player);
                        
                    }
                });
        
        g_signal_connect(PLAYERCTL_PLAYER_MANAGER(this->m_manager), "name-appeared", G_CALLBACK(Playerctl::on_name_appeared), this);
        g_signal_connect(PLAYERCTL_PLAYER_MANAGER(this->m_manager), "player-appeared", G_CALLBACK(Playerctl::on_player_appeared), this);
        g_signal_connect(PLAYERCTL_PLAYER_MANAGER(this->m_manager), "player-vanished", G_CALLBACK(Playerctl::on_player_vanished), this);

        this->m_main_loop = g_main_loop_new(NULL, FALSE);
        this->m_main_loop_future = std::async(std::launch::async, [this]{
                   g_main_loop_run(this->m_main_loop); 
                });
    }

    Playerctl::~Playerctl()
    {
        this->m_time_updater_cancel_token->cancel = true;
        g_main_loop_quit(this->m_main_loop);
        g_main_loop_unref(this->m_main_loop);
        this->m_time_updater.wait();
    }

    void Playerctl::init_managed_player(PlayerctlPlayer* player)
    {
        g_signal_connect(G_OBJECT(player), "play", G_CALLBACK(on_play), this);
    }

    void Playerctl::on_play(PlayerctlPlayer* player, gpointer* data)
    {
        Playerctl* player_class = reinterpret_cast<Playerctl*>(data);
        
        if (!player_class) return;

        playerctl_player_manager_move_player_to_top(player_class->m_manager, player);
    }

    void Playerctl::on_name_appeared(PlayerctlPlayerManager* manager, PlayerctlPlayerName* name, gpointer* data)
    {
        GError* error = nullptr;
        Playerctl* player_class = reinterpret_cast<Playerctl*>(data);
        
        if (!player_class) return;
        if (name->name) {
            player_class->log("New Player appeared: " + std::string(name->name), Severity::INFO);
            if (vector_contains(player_class->m_blacklist, to_title_case(name->name)))
            {
                player_class->log(std::string(name->name) + " is on the blacklist, ignored.", Severity::WARNING);
                return;
            }
        }


        PlayerctlPlayer* player = playerctl_player_new_from_name(name, &error);
        player_class->log_error(error);

        playerctl_player_manager_manage_player(manager, player);

        g_object_unref(player);
    }

    void Playerctl::on_player_appeared(PlayerctlPlayerManager* manager, PlayerctlPlayer* player, gpointer* data)
    {
        GError* error = nullptr;
        Playerctl* player_class = reinterpret_cast<Playerctl*>(data);

        if (!player_class) return;
         
        gchar* name = NULL;
        g_object_get(player, "player_name", &name, NULL);
        
        if (name) {
            player_class->log("Set up new player: " + std::string(name), Severity::INFO);
        }
        
        player_class->init_managed_player(player);
    }

    void Playerctl::on_player_vanished(PlayerctlPlayerManager* manager, PlayerctlPlayer* player, gpointer* data)
    {
        Playerctl* player_class = reinterpret_cast<Playerctl*>(data);

        gchar* name = NULL;
        g_object_get(player, "player_name", &name, NULL);
        
        if (!name) return;
        
        player_class->log("Player vanished: " + std::string(name), Severity::INFO);
    }

    void Playerctl::send_track_info(PlayerctlPlayer* player)
    {
        GError* error = NULL;

        PlayerState state;

        const char* temp;

        temp = playerctl_player_get_artist(player, &error);
        if (temp) {
            state.artist = temp;
            delete temp;
        }
        
        this->log_error(error);
        
        temp = playerctl_player_get_title(player, &error);
        if (temp) {
            state.title = temp;
            delete temp;
        }

        this->log_error(error);
        
        temp = playerctl_player_get_album(player, &error);
        if (temp) {
            state.album = temp;
            delete temp;
        }
        this->log_error(error);
        

        gint64 position = 0;        
        g_object_get(player, "position", &position, NULL);
        state.position = position;
        
        PlayerctlPlaybackStatus status = PLAYERCTL_PLAYBACK_STATUS_STOPPED;
        g_object_get(player, "playback-status", &status, NULL);

        switch (status)
        {
            case PLAYERCTL_PLAYBACK_STATUS_STOPPED:
                state.status = PlayerStatus::STOPPED;
            break;

            case PLAYERCTL_PLAYBACK_STATUS_PLAYING:
                state.status = PlayerStatus::PLAYING;
            break;

            case PLAYERCTL_PLAYBACK_STATUS_PAUSED:
                state.status = PlayerStatus::PAUSED;
            break;
        }
        
        gchar* name = NULL;
        g_object_get(player, "player_name", &name, NULL);
        state.player_name = to_title_case(name);
        
        this->m_events->state_changed(state);
    }

    void Playerctl::log_error(GError* error)
    {
        if (error == NULL) return;
        std::string error_message(error->message);
        this->m_events->error(error_message);
        g_clear_error(&error);
    }
}
