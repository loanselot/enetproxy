#pragma once
#include "utils.h"
#include "proton/variant.hpp"
#include <algorithm>

char* utils::get_text(ENetPacket* packet)
{
	gametankpacket_t* tank = reinterpret_cast<gametankpacket_t*>(packet->data);
	memset(packet->data + packet->dataLength - 1, 0, 1);
	return static_cast<char*>(&tank->m_data);
}
gameupdatepacket_t* utils::get_struct(ENetPacket* packet) {
	if (packet->dataLength < sizeof(gameupdatepacket_t) - 4)
		return nullptr;
	gametankpacket_t* tank = reinterpret_cast<gametankpacket_t*>(packet->data);
	gameupdatepacket_t* gamepacket = reinterpret_cast<gameupdatepacket_t*>(packet->data + 4);
	if (gamepacket->m_packet_flags & 8) {
		if (packet->dataLength < gamepacket->m_data_size + 60) {
			printf("got invalid packet. (too small)\n");
			return nullptr;
		}
		return reinterpret_cast<gameupdatepacket_t*>(&tank->m_data);
	}
	else gamepacket->m_data_size = 0;
	return gamepacket;
}

bool utils::replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}
void utils::send(ENetPeer* peer, ENetHost* host, int32_t type, uint8_t* data, int32_t len) {
	ENetPacket* packet = enet_packet_create(0, len + 5, ENET_PACKET_FLAG_RELIABLE);
	gametextpacket_t* game_packet = (gametextpacket_t*)packet->data;
	game_packet->m_type = type;
	if (data)
		memcpy(&game_packet->m_data, data, len);

	memset(&game_packet->m_data + len, 0, 1);
	int code = enet_peer_send(peer, 0, packet);
	if (code != 0)
		PRINTS("Error sending packet! code: %d\n", code);
	enet_host_flush(host);
}
void utils::send(ENetPeer* peer, ENetHost* host, variantlist_t& list, int32_t netid, int32_t delay) {
	uint32_t data_size = 0;
	void* data = list.serialize_to_mem(&data_size, nullptr);

	//optionally we wouldnt allocate this much but i dont want to bother looking into it
	gameupdatepacket_t* update_packet = MALLOC(gameupdatepacket_t, +data_size);
	gametextpacket_t* game_packet = MALLOC(gametextpacket_t, +sizeof(gameupdatepacket_t) + data_size);

	if (!game_packet || !update_packet)
		return;

	memset(update_packet, 0, sizeof(gameupdatepacket_t) + data_size);
	memset(game_packet, 0, sizeof(gametextpacket_t) + sizeof(gameupdatepacket_t) + data_size);
	game_packet->m_type = 4;

	update_packet->m_type = 1;
	update_packet->m_player_flags = netid;
	update_packet->m_packet_flags |= 8;
	update_packet->m_int_data = delay;
	memcpy(&update_packet->m_data, data, data_size);
	update_packet->m_data_size = data_size;
	memcpy(&game_packet->m_data, update_packet, sizeof(gameupdatepacket_t) + data_size);
	free(update_packet);

	ENetPacket* packet = enet_packet_create(game_packet, data_size + sizeof(gameupdatepacket_t), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
	enet_host_flush(host);
	free(game_packet);
}
bool utils::is_number(const std::string& s) {
	return !s.empty() && std::find_if(s.begin() + (*s.data() == '-' ? 1 : 0), s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}