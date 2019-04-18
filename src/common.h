#pragma once
#include "log_message.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <vector>

namespace muscord {
    inline int64_t get_current_ms()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    template<typename Titem>
    inline bool vector_contains(std::vector<Titem> list, Titem item) {
        return std::find(list.begin(), list.end(), item) != list.end();
    }

    inline std::string severity_to_str(const Severity& severity) {
        switch (severity) {
            case Severity::TRACE: return "TRACE";
            case Severity::INFO: return "INFO";
            case Severity::WARNING: return "WARNING";
            case Severity::ERROR: return "ERROR";
        }
        return "INVALID";
    }

    std::string get_config_dir();
    template<typename Tout> void split(const std::string& s, const char delimiter, Tout result); 
    std::string to_title_case(const std::string& input);
    void replace(std::string& str, const std::string& from, const std::string& to);
}
