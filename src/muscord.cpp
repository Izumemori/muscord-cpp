#include "muscord.h"
#include "../discord-rpc/include/discord_rpc.h"
#include "muscord_rpc.h"
#include "playerctl.h"
#include <chrono>
#include <memory>

namespace muscord {    
    inline int64_t get_current_ms()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }
    
    std::function<void(const DiscordUser*)> ready_func;
    void ready_proxy(const DiscordUser* user) { if (ready_func) ready_func(user); }

    std::function<void(int, const char*)> disconnected_func;
    void disconnected_proxy(int error_code, const char* message) { if (disconnected_func) disconnected_func(error_code, message); }

    std::function<void(int, const char*)> errored_func;
    void errored_proxy(int error_code, const char* message) { if (errored_func) errored_func(error_code, message); }
   
    Muscord::Muscord(std::unique_ptr<MuscordConfig>& config, std::unique_ptr<MuscordEvents>& handlers)
    {
        this->m_config = std::move(config);
        this->m_handlers = std::move(handlers);
        this->m_discord_events = std::make_unique<DiscordEventHandlers>();
        ready_func = [this](const DiscordUser* user) {
            this->on_ready(user);
        };
        disconnected_func = [this](int error_code, const char* message) {
            this->on_disconnected(error_code, message);
        };
        errored_func = [this](int error_code, const char* message) {
            this->on_errored(error_code, message);
        };
        this->m_discord_events->ready = ready_proxy;
        this->m_discord_events->disconnected = disconnected_proxy;
        this->m_discord_events->errored = errored_proxy;

        this->m_rpc = std::make_unique<MuscordRpc>(this->m_config->application_id, this->m_discord_events);
        this->m_rpc->log = this->m_handlers->log;

        if (this->m_config->disconnect_on_idle) {
            this->m_idle_check_token = std::make_unique<CancellationToken>();
            this->m_idle_check_future = std::async(std::launch::async, [this]{
                        while (true) {
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                            if (this->m_idle_check_token->cancel) break;
                            if (!this->m_state) continue;
                            if (this->m_state->status == PlayerStatus::PLAYING) continue;
                            
                            int64_t difference = get_current_ms() - this->m_state->time;
                            
                            if (this->m_rpc->connected && difference >= this->m_config->idle_timeout_ms)
                                this->m_rpc->disconnect();
                        }
                    });
        }
    }

    void Muscord::run()
    {
        this->m_rpc->connect();
    }

    void Muscord::stop()
    {
        if (this->m_rpc->connected)
            this->m_rpc->disconnect();

        if (this->m_config->disconnect_on_idle) {
            this->m_idle_check_token->cancel = true;
            this->m_idle_check_future.wait();
        }
    }
    
    void Muscord::on_errored(int error_code, const char* message)
    {
        LogMessage error(message, Severity::ERROR);
        this->m_handlers->log(error);
    }
    
    void Muscord::on_disconnected(int error_code, const char* message)
    {
        LogMessage disconnect(message, Severity::INFO);
        this->m_handlers->log(disconnect);
    }

    void Muscord::on_state_change(const PlayerState& state)
    {
        std::string status;

        switch (state.status) {
            case PlayerStatus::PAUSED:
                status = "PAUSED";
                break;
            case PlayerStatus::PLAYING:
                status = "PLAYING";
                break;
            case PlayerStatus::STOPPED:
                status = "STOPPED";
                break;
        }

        std::string message = "[" + state.player_name + "] [" + status + "] " + state.artist + " - " + state.title;
        LogMessage song(message, Severity::INFO);
        
        MuscordState* mus_state = new MuscordState(state);
        
        if (this->m_state && this->m_state->equals(*mus_state))
        {
            delete mus_state;
            return;
        }

        this->m_state.reset(mus_state);
        this->m_handlers->log(song);
        this->m_rpc->update_presence([this](DiscordRichPresence* presence) { 
                this->m_handlers->play_state_change(*this->m_state, this->m_state->status, presence);
            });
    }

    void Muscord::on_ready(const DiscordUser* user)
    {
        if (!this->m_player) this->create_new_playerctl();
        this->m_handlers->ready(user);
    }
    
    void Muscord::create_new_playerctl()
    {
        std::unique_ptr<PlayerctlEvents> events = std::make_unique<PlayerctlEvents>();
        events->error = [this](std::string message){ this->on_errored(0, message.c_str()); };
        events->state_changed = [this](const PlayerState& state){ this->on_state_change(state); };
        events->log = [this](const LogMessage& message){ this->m_handlers->log(message); };
        this->m_player = std::make_unique<Playerctl>(events);
    }

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
