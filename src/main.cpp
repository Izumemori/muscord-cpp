#include "muscord.h"
#include <iostream>

int main()
{
    using namespace muscord;
    MuscordConfig* config = new MuscordConfig();
    config->application_id = "385845405193207840";
    
    MuscordEvents* events = new MuscordEvents();
    events->log = [&](LogMessage* log) {
        if (log->severity == Severity::TRACE) return;
        
        std::cout << log->message << std::endl;
    };

    events->ready = [&](const DiscordUser* user) {
        std::cout << "Logged in as: " << user->username << std::endl;
    };

    events->play_state_change = [](MuscordState* state, PlayerStatus status, DiscordRichPresence* presence) {
        presence->state = ("by " + state->artist).c_str();
        presence->details = state->title.c_str();

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

        if (state->player_name == "spotify")
            presence->largeImageKey = "spotify_large";
        else
            presence->largeImageKey = "unknown";

        presence->smallImageText = state->album.c_str();
        presence->largeImageText = state->player_name.c_str();
    };

    Muscord muscord(config, events);
    
    muscord.run();

    std::cin.get();

    muscord.stop();

    return 0;
}

