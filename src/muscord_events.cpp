#include "muscord_events.h"

namespace muscord
{
    void MuscordEvents::log(const std::string& message, const Severity severity) {
        LogMessage msg(message, severity);
        this->log_received(msg);
    }
}
