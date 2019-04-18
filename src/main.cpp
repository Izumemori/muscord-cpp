#include "muscord.h"
#include "common.h"
#include "log_message.h"
#include <cstdlib>
#include <iostream>
#include <string> 
#include <map>
#include <memory>
#include <csignal>

using namespace muscord;

std::map<int, std::string> signal_map {
    {SIGTERM, "SIGTERM"},
    {SIGINT, "SIGINT"},
    {SIGSEGV, "SIGSEGV"},
    {SIGABRT, "SIGABRT"}
};

std::shared_ptr<MuscordConfig> config;
std::unique_ptr<Muscord> muscord_client;
bool should_exit = false;

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

int main()
{
    std::signal(SIGTERM, handle_signal);
    std::signal(SIGINT, handle_signal);
    std::signal(SIGSEGV, handle_signal);
    std::signal(SIGABRT, handle_signal);

    std::string config_dir_base = get_config_dir() + "/muscord";

    config = std::make_shared<MuscordConfig>(config_dir_base + "/config.yml");

    std::unique_ptr<MuscordEvents> events = std::make_unique<MuscordEvents>();
    events->log = log;

    events->ready = [](const DiscordUser* user) {
        LogMessage logged_in_msg("Logged in as " + std::string(user->username) + "#" + std::string(user->discriminator) + " (" + std::string(user->userId) + ")", Severity::INFO);
        log(logged_in_msg);
    };
   
    std::string artist; // work around scope
    std::string title;
    std::string player_name;
    std::string album;
    std::string player_icon;
    std::string play_state_icon;
    events->play_state_change = [&](const MuscordState& state, DiscordRichPresence* presence) {
        artist = config->fmt_artist_str(state.artist);
        title = config->fmt_title_str(state.title);
        player_name = config->fmt_player_str(state.player_name);
        album = config->fmt_album_str(state.album);

        player_icon = config->get_player_icon(state.player_name);
        play_state_icon = config->get_play_state_icon(state.status);

        presence->state = artist.c_str();
        presence->details = title.c_str();

        presence->largeImageKey = player_icon.c_str();
        presence->smallImageKey = play_state_icon.c_str();

        presence->smallImageText = album.c_str();
        presence->largeImageText = player_name.c_str();
    };

    muscord_client = std::make_unique<Muscord>(config, events);
    
    muscord_client->run();

    std::promise<void>().get_future().wait(); // wait indefinitely

    return 0;
}

