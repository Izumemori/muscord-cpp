#include "../yaml-cpp/include/yaml-cpp/yaml.h"
#include "common.h"
#include "log_message.h"
#include "muscord_config.h"
#include "playerctl.h"
#include <sys/stat.h>
#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <algorithm>

namespace YAML {
    template<>
    struct convert<muscord::Severity> {
        static Node encode(const muscord::Severity& severity) {
            Node node;
            switch (severity) {
                case muscord::Severity::TRACE:
                    node = "trace";
                    break;
                case muscord::Severity::INFO:
                    node = "info";
                    break;
                case muscord::Severity::WARNING:
                    node = "warning";
                    break;
                case muscord::Severity::ERROR:
                    node = "error";
                    break;
            }

            return node;
        }

        static bool decode(const Node& node, muscord::Severity& severity) {
            std::string text = node.as<std::string>();

            if (text == "trace") {
                severity = muscord::Severity::TRACE;
                return true;
            }

            if (text == "info") {
                severity = muscord::Severity::INFO;
                return true;
            }

            if (text == "warning") {
                severity = muscord::Severity::WARNING;
                return true;
            }

            if (text == "error") {
                severity = muscord::Severity::ERROR;
                return true;
            }

            return false;
        }
    };
}

namespace muscord {

    inline void throw_field_missing(const std::string& field_name)
    {
        throw std::runtime_error("Field or value missing: " + field_name);
    }

    inline std::string node_type_to_str(const YAML::NodeType::value type) {
        std::string str;

        switch (type) {
            case YAML::NodeType::Map:
                str = "map";
                break;
            case YAML::NodeType::Null:
                str = "null";
                break;
            case YAML::NodeType::Scalar:
                str = "scalar";
                break;
            case YAML::NodeType::Sequence:
                str = "sequence";
                break;
            case YAML::NodeType::Undefined:
                str = "undefined";
                break;
        }

        return str;
    }

    inline void throw_not_valid(const std::string& sequence_name, const YAML::NodeType::value field_type, const YAML::NodeType::value desired_type)
    {
        std::string type = node_type_to_str(field_type);
        std::string desired = node_type_to_str(desired_type);
        throw std::runtime_error("Field " + sequence_name + " is of type '" + type + "' expected was '" + desired + "'");
    }

    MuscordConfig::MuscordConfig(const std::string& path) {
        this->m_config = MuscordConfig::load_or_create(path);
        this->application_id = this->m_config["application_id"].as<std::string>();    
        this->disconnect_on_idle = this->m_config["disconnect_on_idle"].as<bool>();
        this->m_play_state_icons = this->m_config["play_state_icons"].as<std::map<std::string, std::string>>();
        this->m_default_icon = this->m_config["default_player_icon"].as<std::string>();

        if (this->disconnect_on_idle)
        {
            if (this->m_config["idle_timeout_ms"])
                this->idle_timeout_ms = this->m_config["idle_timeout_ms"].as<int64_t>();
            else
                this->idle_timeout_ms = this->m_config["idle_timeout"].as<int64_t>() * 1000;
        }

        if (this->m_config["player_blacklist"])
            this->blacklist = this->m_config["player_blacklist"].as<std::vector<std::string>>();

        if (this->m_config["player_icons"])
            this->m_player_icons = this->m_config["player_icons"].as<std::map<std::string, std::string>>();
        
        if (this->m_config["min_log_level"])
            this->min_log_level = this->m_config["min_log_level"].as<Severity>();
        
        if (this->m_config["format"])
            this->m_fmt_strings = this->m_config["format"].as<std::map<std::string, std::string>>();
    }

    std::string MuscordConfig::fmt_album_str(const std::string& album) {
        return this->get_formatted_string("album", album);
    }

    std::string MuscordConfig::fmt_title_str(const std::string& title) {
        return this->get_formatted_string("title", title);
    }

    std::string MuscordConfig::fmt_artist_str(const std::string& artist) {
        return this->get_formatted_string("artist", artist);
    }

    std::string MuscordConfig::fmt_player_str(const std::string& player_name) {
        return this->get_formatted_string("player_name", player_name);
    }

    std::string MuscordConfig::get_formatted_string(const std::string& key, const std::string& content) {
        auto it = this->m_fmt_strings.find(key);

        if (it == this->m_fmt_strings.end()) return content; // Nothing to format
        if (it->second.empty() || it->second == "{0}") return content; // Empty content, so nothing to format either
        
        std::string formatted = it->second;
        
        replace(formatted, "{0}", content);
        
        return formatted;
    };

    std::string MuscordConfig::get_play_state_icon(const PlayerStatus& play_state) {
        std::string search;

        switch (play_state) {
            case PlayerStatus::PLAYING:
                search = "Playing";
                break;
            case PlayerStatus::PAUSED:
                search = "Paused";
                break;
            case PlayerStatus::STOPPED:
                search = "Stopped";
                break;
        }
        
        auto it = this->m_play_state_icons.find(search);
        if (it == this->m_play_state_icons.end()) throw std::runtime_error("Key not found");

        return it->second;
    }

    std::string MuscordConfig::get_player_icon(const std::string& player_name) {
        auto it = this->m_player_icons.find(player_name);
        
        if (it == this->m_player_icons.end()) return this->m_default_icon;

        return it->second;            
    }



    YAML::Node MuscordConfig::load_or_create(const std::string& path) {
        struct stat buffer;

        if (stat(path.c_str(), &buffer) != 0)
        {
            YAML::Node new_config;
            new_config["application_id"] = 385845405193207840;
            new_config["disconnect_on_idle"] = true;
            new_config["idle_timeout"] = 30;
            new_config["player_blacklist"].push_back("Spotify");
            new_config["player_icons"]["Spotify"] = "spotify_large";
            new_config["player_icons"]["Lollypop"] = "lollypop_large_mini";
            new_config["play_state_icons"]["Playing"] = "play_white_small";
            new_config["play_state_icons"]["Paused"] = "pause_white_small";
            new_config["play_state_icons"]["Stopped"] = "stop_white_small";
            new_config["default_player_icon"] = "unknown";
            new_config["min_log_level"] = Severity::INFO;
            new_config["format"]["title"] = "{0}";
            new_config["format"]["artist"] = "by {0}";
            new_config["format"]["album"] = "{0}";
            new_config["format"]["player_name"] = "{0}";

            YAML::Emitter emitter;
            emitter << new_config;

            std::ofstream config_file(path);
            config_file << emitter.c_str();
        }

        YAML::Node config = YAML::LoadFile(path);
        
        std::map<std::string, std::string> required_fields = {
            {"application_id", ""},
            {"idle_timeout_ms", "idle_timeout"},
            {"play_state_icons", ""},
            {"default_player_icon", ""}
        };

        std::map<std::string, YAML::NodeType::value> sequences = {
            {"player_blacklist", YAML::NodeType::Sequence},
            {"player_icons", YAML::NodeType::Map},
            {"play_state_icons", YAML::NodeType::Map},
            {"format", YAML::NodeType::Map}
        };

        // check required fields for existence
        for (auto& field : required_fields) {
            if (config[field.first]) continue;
            if (!field.second.empty() && config[field.second]) continue;
            throw_field_missing(field.first + (field.second.empty() ? "" : " or " + field.second));
        }

        // check if fields are sequences
        for (auto& field : sequences) {
            if (!config[field.first]) continue;
            auto conf_field = config[field.first];
            if (conf_field.Type() == field.second) continue;
            throw_not_valid(field.first, conf_field.Type(), field.second);
        }

        return config;
    }
}
