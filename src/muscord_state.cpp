#include "muscord_state.h"
#include "common.h"

namespace muscord
{
    bool MuscordState::equals(const MuscordState& other)
    {
        return (this->artist == other.artist &&
                this->title == other.title &&
                this->album == other.album &&
                this->player_name == other.player_name && 
                this->status == other.status);
    }

    MuscordState::MuscordState()
    {
       this->time = get_current_ms(); 
    }

    MuscordState::MuscordState(const PlayerState& state) : MuscordState::MuscordState()
    {
        this->artist = state.artist;
        this->title = state.title;
        this->album = state.album;
        this->player_name = state.player_name;
        this->status = state.status;
    }
}