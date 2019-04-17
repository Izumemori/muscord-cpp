#include "muscord.h"
#include "log_message.h"
#include <iostream>
#include <map>
#include <memory>

using namespace muscord;

int main()
{
    std::map<std::string, std::string> player_icons = {
        {"spotify", "spotify_large"},
        {"Lollypop", "lollypop_large_mini"},
        {"default", "unknown"}
    };
    
    std::unique_ptr<MuscordConfig> config = std::make_unique<MuscordConfig>();
    config->application_id = "385845405193207840";
    config->disconnect_on_idle = true;
    config->idle_timeout_ms = 30000;

    std::unique_ptr<MuscordEvents> events = std::make_unique<MuscordEvents>();
    events->log = [&](const LogMessage& log) {
        if (log.severity == Severity::TRACE) return;
        
        std::cout << log.message << std::endl;
    };

    events->ready = [&](const DiscordUser* user) {
        std::cout << "Logged in as: " << user->username << std::endl;
    };
   
    std::string artist; // work around scope
    events->play_state_change = [&](const MuscordState& state, PlayerStatus status, DiscordRichPresence* presence) {
        artist = "by " + state.artist;
        
        presence->state = artist.c_str();
        presence->details = state.title.c_str();

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
        
        auto large_image_key = player_icons.find(state.player_name);

        if (large_image_key != player_icons.end())
            presence->largeImageKey = large_image_key->second.c_str();
        else
            presence->largeImageKey = player_icons["default"].c_str();

        presence->smallImageText = state.album.c_str();
        presence->largeImageText = state.player_name.c_str();
    };

    Muscord muscord(config, events);
    
    muscord.run();

    std::cin.get();

    muscord.stop();

    return 0;
}

