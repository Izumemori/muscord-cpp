#include "muscord.h"
#include <iostream>
#include <map>

using namespace muscord;

int main()
{
    std::map<std::string, std::string> player_icons = {
        {"spotify", "spotify_large"},
        {"Lollypop", "lollypop_large_mini"},
        {"default", "unknown"}
    };
    
    MuscordConfig* config = new MuscordConfig();
    config->application_id = "385845405193207840";
    config->disconnect_on_idle = true;
    config->idle_timeout_ms = 30000;

    MuscordEvents* events = new MuscordEvents();
    events->log = [&](LogMessage* log) {
        if (log->severity == Severity::TRACE) return;
        
        std::cout << log->message << std::endl;
    };

    events->ready = [&](const DiscordUser* user) {
        std::cout << "Logged in as: " << user->username << std::endl;
    };
   
    /* work around scope limitations */
    std::string artist;
    std::string title;
    std::string album;
    std::string player_name;
    events->play_state_change = [&](MuscordState* state, PlayerStatus status, DiscordRichPresence* presence) {
        artist = "by " + state->artist;
        title = state->title;
        album = state->album;
        player_name = state->player_name;

        presence->state = artist.c_str();
        presence->details = title.c_str();

        switch (status) {
            case PlayerStatus::PLAYING:
                presence->smallImageKey = "play_white_small";
                break;
            case PlayerStatus::PAUSED:
                presence->smallImageKey = "pause_white_small";
                break;
            case PlayerStatus::STOPPED:
                presence->smallImageKey = "stop_white_small";
                break;
        }
        
        auto large_image_key = player_icons.find(player_name);

        if (large_image_key != player_icons.end())
            presence->largeImageKey = large_image_key->second.c_str();
        else
            presence->largeImageKey = player_icons["default"].c_str();

        presence->smallImageText = album.c_str();
        presence->largeImageText = player_name.c_str();
    };

    Muscord muscord(config, events);
    
    muscord.run();

    std::cin.get();

    muscord.stop();

    return 0;
}

