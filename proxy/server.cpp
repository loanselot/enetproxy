#pragma once
#include "server.h"
#include <iostream>
#include "enet/include/enet.h"
#include "proton/hash.hpp"
#include "proton/rtparam.hpp"
#include "proton/variant.hpp"
#include "utils.h"

ENetHost* m_proxy_server;
ENetHost* m_real_server;
ENetPeer* m_server_peer;
ENetPeer* m_gt_peer;
int m_proxyport = 17191;
std::string m_gtversion = "3.01";

void server::start() {
    ENetAddress address;
    enet_address_set_host(&address, "0.0.0.0");
    address.port = m_proxyport;
    m_proxy_server = enet_host_create(&address, 1024, 10, 0, 0);

    if (!m_proxy_server) {
        PRINTS("failed to start the server!\n");
        return;
    }
    m_proxy_server->checksum = enet_crc32;
    enet_host_compress_with_range_coder(m_proxy_server);
    PRINTS("started the enet server.\n");
}

//TODO: move
int m_user = 0;
int m_token = 0;
std::string m_server = "209.59.191.76";
int m_port = 17093;
int m_netid = -1;

void on_disconnect(bool reset) {
    m_netid = -1;
    if (m_server_peer) {
        enet_peer_disconnect(m_server_peer, 0);
        m_server_peer = nullptr;
    }
    if (reset) {
        m_user = 0;
        m_token = 0;
        m_server = "209.59.191.76";
        m_port = 17093;
    }
}

bool connecting = false;
bool ingame = false;

//Separated these into two because outgoing returned in cases of packet disconnect therefore not handling incoming packets at all at that time.
void handle_outgoing() {
    ENetEvent evt;
    while (enet_host_service(m_proxy_server, &evt, 0) > 0) {
        m_gt_peer = evt.peer;

        switch (evt.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                PRINTS("Connecting to server.\n");
                ENetAddress address;
                enet_address_set_host(&address, m_server.c_str());
                address.port = m_port;
                PRINTS("port is %d and server is %s\n", m_port, m_server.c_str());
                client::start();
                m_server_peer = enet_host_connect(m_real_server, &address, 2, 0);
                if (!m_server_peer) {
                    PRINTS("Failed to connect to real server.\n");
                    return;
                }
            } break;
            case ENET_EVENT_TYPE_RECEIVE: {
                int packet_type = get_packet_type(evt.packet);
                switch (packet_type) {
                    case NET_MESSAGE_GENERIC_TEXT: {
                        auto text = utils::get_text(evt.packet);
                        std::string packet = text;
                        PRINTS("Generic text: %s\n", packet.c_str());

                        if (packet.find("|text|/name ") != -1) { //ghetto solution, but too lazy to make a framework for commands.
                            std::string name = "`2" + packet.substr(packet.find("/name ") + 5);
                            variantlist_t va{ "OnNameChanged" };
                            va[1] = name;
                            utils::send(m_gt_peer, m_proxy_server, va, m_netid, -1);
                            std::string acti = ("action|log\nmsg|name set to: " + name);
                            utils::send(m_gt_peer, m_proxy_server, NET_MESSAGE_GAME_MESSAGE, (uint8_t*)acti.c_str(), acti.length());
                            enet_packet_destroy(evt.packet);
                            return;
                        }
                        if (packet.find("game_version|2.996") != -1) {
                            utils::replace(packet, "2.996", m_gtversion);
                            ingame = false;
                            PRINTS("Spoofing gt version\n");
                            utils::send(m_server_peer, m_real_server, NET_MESSAGE_GENERIC_TEXT, (uint8_t*)packet.c_str(), packet.length());
                            enet_packet_destroy(evt.packet);
                            return;
                        }
                    } break;
                    case NET_MESSAGE_GAME_MESSAGE: {
                        auto text = utils::get_text(evt.packet);
                        std::string packet = text;
                        PRINTS("Game message: %s\n", packet.c_str());
                    } break;
                    case NET_MESSAGE_GAME_PACKET: {
                        auto packet = utils::get_struct(evt.packet);
                        if (!packet)
                            break;
                        PRINTS("gamepacket type: %d\n", packet->m_type);

                        switch (packet->m_type) {
                            case PACKET_CALL_FUNCTION: {
                                variantlist_t varlist{};
                                varlist.serialize_from_mem(utils::get_extended(packet));
                                PRINTS("varlist: %s\n", varlist.print().c_str());
                            } break;

                            case PACKET_PING_REPLY: {
                                //since this is a pointer we do not need to copy memory manually again
                                packet->m_vec2_x = 1000.f;  //gravity
                                packet->m_vec2_y = 250.f;   //move speed
                                packet->m_vec_x = 64.f;     //punch range
                                packet->m_vec_y = 64.f;     //build range
                                packet->m_jump_amount = 0;  //for example unlim jumps set it to high which causes ban
                                packet->m_player_flags = 0; //effect flags. good to have as 0 if using mod noclip, or etc.
                            } break;
                            case PACKET_DISCONNECT:
                            case PACKET_APP_INTEGRITY_FAIL: {
                                if (ingame)
                                    return;
                            } break;
                        }
                    } break;
                    case NET_MESSAGE_TRACK: //track one should never be used, but its not bad to have it in case.
                    case NET_MESSAGE_CLIENT_LOG_RESPONSE: return;

                    default: PRINTS("Got unknown packet of type %d.\n", packet_type); break;
                }

                enet_peer_send(m_server_peer, 0, evt.packet);
                if (m_real_server)
                    enet_host_flush(m_real_server);
            } break;
            case ENET_EVENT_TYPE_DISCONNECT: {
                if (ingame)
                    return;
                if (connecting) {
                    on_disconnect(false);
                    connecting = false;
                    return;
                }

            } break;
            default: PRINTS("UNHANDLED\n"); break;
        }
    }
}

void handle_incoming() {
    ENetEvent event;
    while (enet_host_service(m_real_server, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: PRINTC("connection event\n"); break;
            case ENET_EVENT_TYPE_DISCONNECT: on_disconnect(true); return;
            case ENET_EVENT_TYPE_RECEIVE: {
                if (event.packet->data) {
                    int packet_type = get_packet_type(event.packet);
                    switch (packet_type) {
                        case NET_MESSAGE_GENERIC_TEXT: {
                            auto text = utils::get_text(event.packet);
                            std::string packet = text;
                            PRINTC("Generic text: %s\n", packet.c_str());
                        } break;
                        case NET_MESSAGE_GAME_MESSAGE: {
                            auto text = utils::get_text(event.packet);
                            std::string packet = text;
                            PRINTC("Game message: %s\n", packet.c_str());
                        } break;
                        case NET_MESSAGE_GAME_PACKET: {
                            auto packet = utils::get_struct(event.packet);
                            if (!packet)
                                break;

                            switch (packet->m_type) {
                                case PACKET_CALL_FUNCTION: {
                                    variantlist_t varlist{};
                                    auto extended = utils::get_extended(packet);
                                    extended += 4; //since it casts to data size not data but too lazy to fix this
                                    varlist.serialize_from_mem(extended);
                                    PRINTC("varlist: %s\n", varlist.print().c_str());
                                    auto func = varlist[0].get_string();

                                    //probably subject to change, so not including in switch statement.
                                    if (func.find("OnSuperMainStartAcceptLogon") != -1)
                                        ingame = true;

                                    switch (hs::hash32(func.c_str())) {
                                        case fnv32("OnSendToServer"): {
                                            m_port = varlist[1].get_uint32();
                                            m_token = varlist[2].get_uint32();
                                            m_user = varlist[3].get_uint32();
                                            auto str = varlist[4].get_string();
                                            m_server = str.erase(str.length() - 1); //remove the | from the end
                                            PRINTC("port: %d token %d user %d server %s\n", m_port, m_token, m_user, m_server.c_str());
                                            varlist[1] = m_proxyport;
                                            varlist[4] = "127.0.0.1|";
                                            connecting = true;
                                            utils::send(m_gt_peer, m_proxy_server, varlist);
                                            enet_packet_destroy(event.packet);

                                            if (m_real_server) {
                                                enet_host_destroy(m_real_server);
                                                m_real_server = nullptr;
                                            }
                                            return;
                                        }

                                        case fnv32("OnConsoleMessage"): {
                                            varlist[1] = "`4[PROXY]`` " + varlist[1].get_string();
                                            utils::send(m_gt_peer, m_proxy_server, varlist);
                                        }
                                        case fnv32("OnSpawn"): {
                                            std::string meme = varlist.get(1).get_string();
                                            rtvar var = rtvar::parse(meme);
                                            auto name = var.find("name");
                                            auto netid = var.find("netID");
                                            auto onlineid = var.find("onlineID");

                                            if (name && netid && onlineid) {
                                                name->m_values[0] += " `4[" + netid->m_value + "]``";

                                                if (meme.find("type|local") != -1) {
                                                    //set mod state to 1 (allows infinite zooming, this doesnt ban cuz its only the zoom not the actual long punch)
                                                    var.find("mstate")->m_values[0] = "1";

                                                    //get our netid from OnSpawn
                                                    m_netid = atoi(netid->m_value.c_str());
                                                }

                                                auto str = var.serialize();
                                                //serialize removes | it and this causes the whole thing to break
                                                utils::replace(str, "onlineID", "onlineID|");
                                                varlist[1] = str;
                                                PRINTC("new: %s\n", varlist.print().c_str());
                                                utils::send(m_gt_peer, m_proxy_server, varlist, -1, -1);
                                                return;
                                            }
                                        }
                                    }
                                } break;

                                //no need to print this for handled packet types such as func call, because we know its 1
                                default: PRINTC("gamepacket type: %d\n", packet->m_type); break;
                            }
                        } break;

                        //ignore tracking packet, and request of client crash log
                        case NET_MESSAGE_TRACK: {
                            auto text = utils::get_text(event.packet);
                            std::string packet = text;
                            PRINTC("Tracking packet: %s\n", packet.c_str());
                            return; //Do not send packet to client
                        } break;
                        case NET_MESSAGE_CLIENT_LOG_REQUEST: return;

                        default: PRINTS("Got unknown packet of type %d.\n", packet_type); break;
                    }
                }

                enet_peer_send(m_gt_peer, 0, event.packet);
                enet_host_flush(m_proxy_server);
            } break;

            default: PRINTC("UNKNOWN event: %d\n", event.type); break;
        }
    }
}

void server::poll() {
    //outgoing packets going to real server that are intercepted by our proxy server
    handle_outgoing();

    if (!m_real_server)
        return;

    //ingoing packets coming to gt client intercepted by our proxy client
    handle_incoming();
}

void server::run() {
    start();
    client::start();
    while (true) {
        poll();
        Sleep(1);
    }
}

void client::start() {
    m_real_server = enet_host_create(0, 1, 2, 0, 0);
    if (!m_real_server) {
        PRINTC("failed to start the client\n");
        return;
    }
    m_real_server->checksum = enet_crc32;
    enet_host_compress_with_range_coder(m_real_server);
    enet_host_flush(m_real_server);
    PRINTC("Started enet client\n");
}
