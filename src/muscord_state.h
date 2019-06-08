#pragma once
#include "playerctl.h"
#include <string>

namespace muscord {
    typedef struct MuscordState {
        PlayerStatus status;
        std::string artist;
        std::string title;
        std::string album;
        std::string player_name;
        uint64_t time;
        bool idle; // set if nothing is playing

        MuscordState();
        MuscordState(const PlayerState& state);

        bool equals(const MuscordState& other);
    } MuscordState;
}