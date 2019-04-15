#include "muscord.h"
#include "cancellation_token.h"
#include "../include/discord_rpc.h"
#include "../include/discord_register.h"
#include <functional>
#include <iostream>
#include <future>
#include <thread>
#include <chrono>

namespace muscord 
{
    MuscordRpc::MuscordRpc(const std::string application_id, DiscordEventHandlers* handlers)
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
        }
        
        this->m_callback_token = new CancellationToken();
        
        LogMessage conn_message("Connecting...", Severity::INFO);
        this->log(&conn_message);
        Discord_Initialize(this->m_application_id.c_str(), this->m_handlers, 1, "");
        
        this->start_callback_checks();
    }

    void MuscordRpc::disconnect()
    {
        this->connected = false;
        LogMessage disconn_message("Disconnecting...", Severity::INFO);
        this->log(&disconn_message);
        this->m_callback_token->cancel = true;
        this->m_callback_future.wait();
        this->stop_callback_checks();
        this->clear_presence();
        Discord_Shutdown();
        LogMessage disconn_finish_message("Disconnected", Severity::INFO);
        this->log(&disconn_finish_message);
    }

    void MuscordRpc::update_presence(const std::function<void (DiscordRichPresence*)>& f)
    {
        if (!this->connected)
            this->connect();

        auto rich_presence = new DiscordRichPresence();
        f(rich_presence);
        Discord_UpdatePresence(rich_presence);
        delete rich_presence;
    }

    void MuscordRpc::clear_presence()
    {
        Discord_ClearPresence();
    }

    void MuscordRpc::start_callback_checks()
    {
        this->m_callback_future = std::async(std::launch::async, [this]{
            LogMessage ran_callbacks("Ran callbacks");
                
            while(true)
            {
                Discord_RunCallbacks();
                this->log(&ran_callbacks);
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
