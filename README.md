# Growtopia enet proxy

## How to use
* Does not need hosts file editing or separate http server
* Use HxD or similar to edit "growtopia1.com" and "growtopia2.com" to "localhost" In growtopia binary.
* Start proxy -> Run localhost patched file -> Works as supposed

## Features
* Print all variantlists and function calls
* Print all text packets
* Supports modifying, ignoring, and making new packets
* Has a PoC /name name command to call OnNameChanged function for local client.
* Has a PoC OnSpawn modifier which appends netid to each players' name
* Can both intercept outgoing and incoming packets
* Integrated http server
