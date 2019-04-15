#pragma once
#include <iostream>

namespace muscord {
    typedef enum Severity {
        TRACE,
        INFO,
        WARNING,
        ERROR
    } Severity;

    typedef struct LogMessage {
        Severity severity;
        std::string message;

        LogMessage(std::string message, Severity severity = Severity::TRACE) : severity(severity), message(message)
        {};
    } LogMessage;
}
