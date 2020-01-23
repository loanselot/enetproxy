# Growtopia enet proxy

## How to use
* Does not need separate http server
* Use HxD or similar to edit "growtopia1.com" and "growtopia2.com" to "localhost" In growtopia binary.
* Alternative: Use hosts file to set growtopia1 and growtopia2 to point to 127.0.0.1 if you want to
* Start proxy -> Run localhost patched file (or normal with hosts) -> Works as supposed

## Features
* Print all variantlists and function calls
* Print all text packets
* Supports modifying, ignoring, and making new packets
* Has a PoC /name name command to call OnNameChanged function for local client.
* Has a PoC OnSpawn modifier which appends netid to each players' name
* Can both intercept outgoing and incoming packets
* Integrated http server

## TODO
* Don't ignore disconnect packets & disconnect events due to not responding to ping request packet with ping reply, it needs to be implemented asap
* Clean code
* Support more gamepacket types
* Support subserver redirection (OnSendToServer) - actually no clue if this works already
* Ignore tracking packets (and maybe print them)


### Video: https://streamable.com/bhokj  

![x](https://i.imgur.com/Lndhj70.png "Proxy pic 1")

