# retchat
> veeery simple TCP/IP chat app made originally in C, later to be rewritten in C++. it allows you to communicate with other clients connected to the same host, through rooms. it works via Diffie-Hellman key exchange, SHA256 XOR encryption. full spec can be found [here](https://github.com/retinc/retchat-docs)


## usage

### download
1. `git clone https://github.com/retuci0/retchat/`
2. `cd retchat`

### compile
1. `cmake -B build`
2. `cmake --build build`

### run
> args are optional
> - default port is 6677 (SIX SEVEN!!!)
> - default bans file is `bans.txt`

```
./build/server <port=6677> <bans_file=bans.txt>
```

### console
the server provides you with an interactive console you can use to either kick, ban or query users.

| command | description |
|---------------------|----------------------------------------|
| `kick <fd\|nick>`   | kick a client by socket fd or nickname |
| `ban <nick>`        | ban a nickname (saved to bans file)    |
| `ipban <ip>`        | ban an IP address (saved to bans file) |
| `unban <nick>`      | remove a nickname ban                  |
| `unbanip <ip>`      | remove an IP ban                       |
| `list clients`      | show all connected clients             |
| `list rooms`        | show all active rooms                  |
| `list bans`         | show all active bans                   |
| `query client <fd>` | show details for a specific client     |
| `query room <name>` | show details for a specific room       |
| `stop`              | shut down the server                   |
| `help`              | show this list                         |

## bans
bans are stored in a plain text file (default `bans.txt`), one entry per line:
```
nick:someuser
ip:1.2.3.4
```
the file is loaded on startup and updated automatically after every ban or unban. you can edit it manually while the server is stopped.

## clients
- **android** — [retchat-android](https://github.com/retinc/retchat-android)
- **linux CLI** — [retchat-client](https://github.com/retinc/retchat-linux)

## license

MIT or something idk just don't claim it as yours i guess
