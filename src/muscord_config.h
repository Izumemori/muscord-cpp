#pragma once
#include "playerctl.h"
#include "log_message.h"
#include "../yaml-cpp/include/yaml-cpp/yaml.h"
#include <vector>
#include <map>
#include <iostream>

namespace muscord {
    class MuscordConfig {
        public:
            std::string application_id;
            bool disconnect_on_idle;
            int64_t idle_timeout_ms = 0;
            std::vector<std::string> blacklist = {};
            Severity min_log_level;
            std::string fmt_artist_str(const std::string& artist);
            std::string fmt_player_str(const std::string& player);
            std::string fmt_title_str(const std::string& title);
            std::string fmt_album_str(const std::string& album);
            std::string get_play_state_icon(const PlayerStatus& play_status);
            std::string get_player_icon(const std::string& player_name);
            MuscordConfig(const std::string& path);
        private:
            YAML::Node m_config;
            std::map<std::string, std::string> m_play_state_icons;
            std::map<std::string, std::string> m_player_icons;
            std::map<std::string, std::string> m_fmt_strings = {{"artist", "by {0}"}};
            std::string m_default_icon;
            std::string get_formatted_string(const std::string& key, const std::string& content);
            static YAML::Node load_or_create(const std::string& path);
    };
}
