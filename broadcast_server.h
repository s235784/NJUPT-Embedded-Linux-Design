//
// Created by 诺天 on 2024/12/24.
//

#ifndef BROADCAST_SERVER_H
#define BROADCAST_SERVER_H

#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"
#include <set>

class broadcast_server {
public:
    broadcast_server();
    void on_open(const websocketpp::connection_hdl& hdl);
    void on_close(const websocketpp::connection_hdl& hdl);
    void on_message(const websocketpp::connection_hdl& hdl, const websocketpp::server<websocketpp::config::asio>::message_ptr& msg);
    void send_light(int light);
    void run(uint16_t port);
    void stop();
private:
    typedef websocketpp::server<websocketpp::config::asio> server;
    typedef std::set<websocketpp::connection_hdl,std::owner_less<websocketpp::connection_hdl>> con_list;

    server m_server;
    con_list m_connections;
};

#endif //BROADCAST_SERVER_H
