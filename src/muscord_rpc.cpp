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
        this->m_application_id = application_id;
        this->m_handlers = handlers;
    }
    
    MuscordRpc::~MuscordRpc()
    {
        this->stop_callback_checks();
        Discord_Shutdown();
    }

    void MuscordRpc::connect()
    {   
        if (this->m_callback_token)
        {
            this->m_callback_token->cancel = true;
            this->m_callback_future.wait();
        }
        
        this->m_callback_token = new CancellationToken();
        
        LogMessage conn_message("Connecting...", Severity::INFO);
        this->log(&conn_message);
        Discord_Initialize(this->m_application_id.c_str(), this->m_handlers, 1, "");
        
        LogMessage finish_conn_message("Connected", Severity::INFO);
        this->start_callback_checks();
        this->log(&finish_conn_message);
    }

    void MuscordRpc::disconnect()
    {
        LogMessage disconn_message("Disconnecting...", Severity::INFO);
        this->log(&disconn_message);
        Discord_Shutdown();
        this->m_callback_token->cancel = true;
        this->m_callback_future.wait();
        LogMessage disconn_finish_message("Disconnected", Severity::INFO);
        this->log(&disconn_finish_message);
    }

    void MuscordRpc::update_presence(const std::function<void (DiscordRichPresence*)>& f)
    {
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
        LogMessage shutting_down("Shutting down", Severity::INFO);
        this->log(&shutting_down);
        this->m_callback_token->cancel = true;
        this->m_callback_future.wait();
    }
}
