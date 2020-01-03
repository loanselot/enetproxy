#pragma once
#include <iostream>
#include <thread>
#include "http.h"
#include "enet/include/enet.h"
#include "server.h"
#include <iomanip>
#include <ctime>
#include <sstream>

int main()
{
	system("title proxy by ama");
	printf("enet proxy by ama\n");
	//209.59.191.86, 17126
	std::thread http(http::run, "127.0.0.1", "17191");
	http.detach();
	printf("HTTP server is running.\n");
	enet_initialize();
	printf("Server & client proxy is running.\n");
	server::run();
}
