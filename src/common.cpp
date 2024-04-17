#include "common.h"
#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

void muscord::replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return;

    str.replace(start_pos, from.length(), to);
}

template<typename Tout>
void muscord::split(const std::string& s, char delimiter, Tout result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        if (item.empty()) continue;
        
        *(result++) = item;
    }
}

std::string muscord::get_config_dir() {
    std::string config_home;
    char* xdg_config_home = std::getenv("XDG_CONFIG_HOME");

    if (xdg_config_home)
    {
        config_home = xdg_config_home;
        return config_home;
    }

    char* user_home = std::getenv("HOME");

    if (!user_home) throw std::runtime_error("Could not determine config location, ensure $HOME or $XDG_CONFIG_HOME are set");

    return std::string(user_home) + "/.config";
}

void muscord::ensure_config_dir_created(std::string& path) {
    DIR* dir = opendir(path.c_str());
    if (dir) { 
        closedir(dir);
        return;
    }

    const int dir_err = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    if (dir_err != 0) {
        std::cout << "Failed to create directory!" << std::endl;
        exit(1);
    }
}

std::string muscord::to_title_case(const std::string& input) {
    std::vector<std::string> words;
    split(input, ' ', std::back_inserter(words));
        
    std::stringstream ss;
    for (auto& word : words) {
        word[0] = std::toupper(word[0]);
        ss << word;
        
        if (&word != &words.back()) ss << " ";
    }

    return ss.str();
}
