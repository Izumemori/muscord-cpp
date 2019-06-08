#pragma once
#include "../discord-rpc/include/discord_rpc.h"
#include "log_message.h"
#include "muscord_state.h"
#include <functional>

namespace muscord {
    typedef struct MuscordEvents {
            std::function<void(const LogMessage&)> log_received;
            std::function<void(const MuscordState&, DiscordRichPresence*)> play_state_change;
            std::function<void(const DiscordUser*)> ready;
            void log(const std::string& message, const Severity severity = Severity::TRACE);
    } MuscordEvents;
}