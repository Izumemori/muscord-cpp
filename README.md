# Muscord <!-- omit in toc -->

A linux tool to display what you're listening to via Discord's Rich Presence utilizing the offical [SDK](https://github.com/discordapp/discord-rpc) and [playerctl](https://github.com/acrisci/playerctl/).  
It gathers music and player information from D-BUS/mpris2.

- [Features](#features)
- [Installation](#installation)
  - [Prerequisites](#prerequisites)
  - [Compiling](#compiling)
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

To compile Muscord just run the `build.sh` file

## Contributing

If you want to add something, improve code, or want to report bugs feel free to open an issue or pull request

## License

This project is licensed under MIT