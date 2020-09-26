#include "muscord.h"
#include "common.h"
#include "log_message.h"
#include <cstdlib>
#include <iostream>
#include <string> 
#include <map>
#include <memory>
#include <csignal>
#include <stdio.h>
#include <string.h>

using namespace muscord;

std::map<int, std::string> signal_map {
    {SIGTERM, "SIGTERM"},
    {SIGINT, "SIGINT"},
    {SIGSEGV, "SIGSEGV"},
    {SIGABRT, "SIGABRT"}
};

std::shared_ptr<MuscordConfig> config;
std::unique_ptr<Muscord> muscord_client;

void log(const LogMessage& log) {
    if (log.severity < config->min_log_level) return;

    std::cout << "[" << severity_to_str(log.severity) << "] " << log.message << std::endl;
}

void handle_signal(int sig_num) {
    LogMessage exit_message("Received signal '" + signal_map[sig_num] + "', exiting...", Severity::WARNING);
    log(exit_message);
    
    muscord_client->stop();

    exit(0);
}

const char* allocate_dynamically(std::string& input) {
    char* ptr = new char[input.size() + 1];

    strcpy(ptr, input.c_str());

    return ptr;
}

int main()
{
    std::signal(SIGTERM, handle_signal);
    std::signal(SIGINT, handle_signal);
    std::signal(SIGSEGV, handle_signal);
    std::signal(SIGABRT, handle_signal);

    std::string config_dir_base = get_config_dir() + "/muscord";
    ensure_config_dir_created(config_dir_base);
    config = std::make_shared<MuscordConfig>(config_dir_base + "/config.yml");

    std::unique_ptr<MuscordEvents> events = std::make_unique<MuscordEvents>();
    events->log_received = static_cast<void(*)(const LogMessage&)>(log);

    events->ready = [](const DiscordUser* user) {
        LogMessage logged_in_msg("Logged in as " + std::string(user->username) + "#" + std::string(user->discriminator) + " (" + std::string(user->userId) + ")", Severity::INFO);
        log(logged_in_msg);
    };
   
    events->play_state_change = [](const MuscordState& state, DiscordRichPresence* presence) {
        // Player stuff
        std::string player_icon = config->get_player_icon(state.player_name);
        std::string play_state_icon = config->get_play_state_icon(state.status);
        std::string player_name = config->fmt_player_str(state.player_name);

        presence->largeImageKey = allocate_dynamically(player_icon);
        presence->smallImageKey = allocate_dynamically(play_state_icon);
        presence->largeImageText = allocate_dynamically(player_name);
      
        std::string artist;

        if (state.idle)
        {
            artist = config->get_idle_string();
            presence->state = allocate_dynamically(artist);
            return; // rest would be empty
        }
        
        // Song stuff
        artist = config->fmt_artist_str(state.artist);
        std::string title = config->fmt_title_str(state.title);
        std::string album = config->fmt_album_str(state.album);

        presence->state = allocate_dynamically(artist);
        presence->details = allocate_dynamically(title);
        presence->smallImageText = allocate_dynamically(album);
    };

    muscord_client = std::make_unique<Muscord>(config, events);
    
    muscord_client->run();

    std::promise<void>().get_future().wait(); // wait indefinitely

    return 0;
}

