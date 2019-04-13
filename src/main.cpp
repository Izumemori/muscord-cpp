#include "muscord.h"
#include "playerctl.h"
#include <iostream>

muscord::playerctl::Playerctl* player = NULL;
muscord::Muscord* m_muscord = NULL;

void set_up_playerctl()
{
    muscord::playerctl::PlayerctlEvents* player_events = new muscord::playerctl::PlayerctlEvents();
    player_events->error = [](const char* message) {
        std::cout << "Error: " << message << std::endl; 
    };

    player_events->state_changed = [](muscord::playerctl::player_state* state) {
        std::cout << state->artist << "-" << state->title << std::endl;
        std::cout << state->album << std::endl;
        std::cout << state->position/1000000.0 << std::endl;
        m_muscord->update_presence([=](DiscordRichPresence* presence){
                    presence->details = state->artist.c_str();
                    presence->state = state->title.c_str();
                    presence->smallImageKey = state->status == muscord::playerctl::player_status::PLAYING ? "play_white_small" : "pause_white_small";
                    presence->smallImageText = state->album.c_str();
                    
                    if (state->player_name == "spotify")
                        presence->largeImageKey = "spotify_large";
                    else
                        presence->largeImageKey = "unknown";

                    presence->largeImageText = state->player_name.c_str();
                });
    
    };

    player = new muscord::playerctl::Playerctl(player_events);
    std::cout << "Player set up" << std::endl;
}

void ready(const DiscordUser* user)
{
    std::cout << user->username << std::endl;
    set_up_playerctl();
}

int main()
{
    DiscordEventHandlers* events = new DiscordEventHandlers();
    events->ready = ready;

    m_muscord = new muscord::Muscord("385845405193207840", events);

    m_muscord->log = [](const char* message){
        std::cout << "Log: " << message << std::endl;
    };
    
    std::cin.get();

    player->~Playerctl();
    m_muscord->~Muscord();
    delete m_muscord;
    delete player;
    return 0;
}

