#include "muscord.h"
#include "cancellation_token.h"
#include "../include/discord_rpc.h"
#include <functional>
#include <iostream>
#include <future>
#include <thread>
#include <chrono>

namespace muscord 
{
    Muscord::Muscord(const char* application_id, DiscordEventHandlers* handlers)
    {
        Discord_Initialize(application_id, handlers, 1, "");
        this->callback_token = new cancellation_token();
        this->start_callback_checks();
    }

    Muscord::~Muscord()
    {
        this->stop_callback_checks();
        Discord_Shutdown();
    }

    void Muscord::update_presence(const std::function<void (DiscordRichPresence*)>& f)
    {
        auto rich_presence = new DiscordRichPresence();
        f(rich_presence);
        Discord_UpdatePresence(rich_presence);
        delete rich_presence;
    }

    void Muscord::start_callback_checks()
    {
        this->callback_future = std::async(std::launch::async, [this]{
            while(true)
            {
                Discord_RunCallbacks();
                this->log("Ran callbacks");
                std::this_thread::sleep_for(std::chrono::seconds(1));
                if (this->callback_token->cancel) break;
            }
        });
    }

    void Muscord::stop_callback_checks()
    {
        this->log("Shutting down...");
        this->callback_token->cancel = true;
        this->callback_future.wait();
    }
}
