#pragma once
#include <iostream>

namespace muscord {
    typedef enum Severity {
        TRACE = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3    
    } Severity;

    typedef struct LogMessage {
        Severity severity;
        std::string message;

        LogMessage(std::string message, Severity severity = Severity::TRACE) : severity(severity), message(message)
        {};
    } LogMessage;
}
