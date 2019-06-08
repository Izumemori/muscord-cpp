#include "muscord.h"
#include "cancellation_token.h"
#include "../discord-rpc/include/discord_rpc.h"
#include <functional>
#include <iostream>
#include <future>
#include <thread>
#include <chrono>
#include <memory>

namespace muscord 
{
    MuscordRpc::MuscordRpc(std::string& application_id, std::shared_ptr<DiscordEventHandlers>& handlers)
    {
        this->connected = false;
        this->m_application_id = application_id;
        this->m_handlers = handlers;
    }
    
    MuscordRpc::~MuscordRpc()
    {
        if (this->connected)
            this->disconnect();
    }

    void MuscordRpc::connect()
    {   
        this->connected = true;
        if (this->m_callback_token)
        {
            this->m_callback_token->cancel = true;
            this->m_callback_future.wait();
            this->m_callback_token.reset(new CancellationToken());
        }
        else this->m_callback_token = std::make_unique<CancellationToken>();
        
        this->log("Connecting...", Severity::INFO);
        Discord_Initialize(this->m_application_id.c_str(), this->m_handlers.get(), 1, "");
        
        this->start_callback_checks();
    }

    void MuscordRpc::disconnect()
    {
        this->connected = false;
        this->log("Disconnecting...", Severity::INFO);
        this->stop_callback_checks();
        this->clear_presence();
        Discord_Shutdown();
        this->log("Disconnected", Severity::INFO);
    }

    void MuscordRpc::update_presence(const std::function<void (DiscordRichPresence*)>& f)
    {
        if (!this->connected)
            this->connect();

        std::unique_ptr<DiscordRichPresence> rich_presence = std::make_unique<DiscordRichPresence>();
        f(rich_presence.get());
        Discord_UpdatePresence(rich_presence.get());
    }

    void MuscordRpc::clear_presence()
    {
        Discord_ClearPresence();
    }

    void MuscordRpc::start_callback_checks()
    {
        this->m_callback_future = std::async(std::launch::async, [this]{             
            while(true)
            {
                Discord_RunCallbacks();
                this->log("Ran callbacks");
                std::this_thread::sleep_for(std::chrono::seconds(1));
                if (this->m_callback_token->cancel) break;
            }
        });
    }

    void MuscordRpc::stop_callback_checks()
    {
        this->m_callback_token->cancel = true;
        this->m_callback_future.wait();
    }
}
