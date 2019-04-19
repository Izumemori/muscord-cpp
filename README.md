# Muscord <!-- omit in toc -->

A linux tool to display what you're listening to via Discord's Rich Presence utilizing the offical [SDK](https://github.com/discordapp/discord-rpc), [playerctl](https://github.com/acrisci/playerctl/), and [yaml-cpp](https://github.com/jbeder/yaml-cpp) for configuration parsing.  
It gathers music and player information from D-BUS/mpris2.

- [Features](#features)
- [Installation](#installation)
  - [Prerequisites](#prerequisites)
  - [Compiling](#compiling)
    - [Simple](#simple)
    - [Manual CMake](#manual-cmake)
  - [Configuring](#configuring)
    - [Example configuration](#example-configuration)
- [Systemd](#systemd)
  - [Systemd example service file](#systemd-example-service-file)
- [Contributing](#contributing)
- [License](#license)

## Features

- Automatic discovery of media players through D-BUS
- Disconnecting after idleing for 30 seconds
  
## Installation

### Prerequisites

- [playerctl](https://github.com/acrisci/playerctl); for instructions refer to the [Installation guide](https://github.com/acrisci/playerctl#installing)

- A desktop environment that utilizes DBus

### Compiling

#### Simple

To compile Muscord just run the `build.sh` file.

#### Manual CMake

First create a build directory `mkdir build` and then switch into it `cd build`.

Then run `cmake .. -DCMAKE_INSTALL_PREFIX=.` or `cmake .. -DCMAKE_INSTALL_PREFIX=. -DDEBUG` for a debug build.

After that run `make`.

The muscord binary should now be in `build/bin`.

### Configuring

Muscord will generate a `config.yml` if it cannot find one in `$HOME/muscord` or `$XDG_CONFIG_HOME/muscord` if `$XDG_CONFIG_HOME` is set.

#### Example configuration

```yml
# main settings
application_id: 385845405193207840
disconnect_on_idle: true

## one of the following is required
## but only if disconnect_on_idle is true/on/yes
## if both are defined idle_timeout_ms has priority
### timeout in milliseconds
idle_timeout_ms: 30000
### or in seconds
# idle_timeout: 30

## these are players that will be ignored by muscord
## for example spotify, as it has way better integration already
## names are Title Cased
player_blacklist: # optional
  - Spotify

# Icon Settings
player_icons: # optional
  Spotify: spotify_large
  Lollypop: lollypop_large_mini

play_state_icons:
  Playing: play_white_small
  Paused: pause_white_small
  Stopped: stop_white_small

default_player_icon: unknown

# Logging
min_log_level: info # possible: (trace, info, warning, error)

# Formatting
## each item can be omitted; artist will default to: "by {0}"; idle will default to: "Idle..."
## "{0}" and empty values will be ignored
format: # optional
  title: 🎵 {0}
  artist: 👤 {0}
  album: 💿 {0}
  player_name: "Player: {0}"
  idle: Not playing anything
```

## Systemd

This tool is designed to be run via systemd, to enable it create a `muscord.service` file in `~/.config/systemd/user/`  
Then simply run `systemctl --user enable muscord.service` and it should start automatically on login

### Systemd example service file

```ini
[Unit]
Description=Muscord

[Service]
ExecStart=/usr/bin/muscord # Path to the executable

[Install]
WantedBy=default.target
```


## Contributing

If you want to add something, improve code, or want to report bugs feel free to open an issue or pull request

## License

This project is licensed under MIT
