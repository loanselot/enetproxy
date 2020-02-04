# Growtopia enet proxy

# NOTE: This is not an internal cheat. The internals which have popped up for sale are not internal cheats but proxies based on this one, don't fall for their scam

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
* Has a PoC OnSpawn modifier which appends netid to each players' name, and gives you unlim zoom
* Can both intercept outgoing and incoming packets
* Integrated http server
* Ignore tracking packets and crash log requests
* Ignore autoban packets (NOTE: you can still get autobanned if the check is serversided)
* Works with subserver switching

## TODO
* Support more gamepacket types
* Automatically solve captcha
* More commands
* send set state to change gravity, speed, other things

## Changelog
# 1.1
* Subserver switching should work, apparently there was no problem in the first place as the implementation worked already
* Edit ping requests always to be non offensive behavior
* Ignore autoban packets sent by client
* Ignore tracking packets
* Ignore crash logs that would be sent to gt
* Clean code
* Gives unlimited zoom

### Video: https://streamable.com/bhokj  

![x](https://i.imgur.com/Lndhj70.png "Proxy pic 1")

