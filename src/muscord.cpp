#include "muscord.h"
#include "common.h"
#include "../discord-rpc/include/discord_rpc.h"
#include "muscord_rpc.h"
#include "playerctl.h"
#include <chrono>
#include <memory>
#include <thread>

namespace muscord {    
    std::function<void(const DiscordUser*)> ready_func;
    void ready_proxy(const DiscordUser* user) { if (ready_func) ready_func(user); }

    std::function<void(int, const char*)> disconnected_func;
    void disconnected_proxy(int error_code, const char* message) { if (disconnected_func) disconnected_func(error_code, message); }

    std::function<void(int, const char*)> errored_func;
    void errored_proxy(int error_code, const char* message) { if (errored_func) errored_func(error_code, message); }
   
    Muscord::Muscord(std::shared_ptr<MuscordConfig>& config, std::unique_ptr<MuscordEvents>& handlers)
    {
        this->m_config = config;
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
        this->m_rpc->log_received = [this](const std::string& message, const Severity severity) {
            this->m_handlers->log(message, severity);
        };

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
        this->m_handlers->log(message, Severity::ERROR);
    }
    
    void Muscord::on_disconnected(int error_code, const char* message)
    {
        this->m_handlers->log(message, Severity::WARNING);
    }

    void Muscord::on_state_change(const PlayerState& state)
    {
        std::unique_ptr<MuscordState> mus_state = std::make_unique<MuscordState>(state);

        mus_state->idle = state.title.empty() && state.artist.empty() && state.album.empty(); // Nothing is playing
        
        if (!this->m_rpc->connected && state.status != PlayerStatus::PLAYING) return; // Don't bother with anything that's not playing when already disconnected
        if (!this->m_state && state.status != PlayerStatus::PLAYING) return; // Don't display anything that's not playing at the start

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
        
        if (this->m_state && this->m_state->equals(*mus_state)) return;

        this->m_state.reset();
        this->m_state = std::move(mus_state);
        this->m_handlers->log(message, Severity::INFO);
        this->m_rpc->update_presence([this](DiscordRichPresence* presence) { 
                this->m_handlers->play_state_change(*this->m_state, presence);
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
        events->log_received = [this](const std::string& message, const Severity severity){ this->m_handlers->log(message, severity); };
        this->m_player = std::make_unique<Playerctl>(events, this->m_config->blacklist);
    }
}
