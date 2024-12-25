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
    // 构造函数
    broadcast_server();

    // 处理连接打开
    void on_open(const websocketpp::connection_hdl& hdl);

    // 处理连接关闭
    void on_close(const websocketpp::connection_hdl& hdl);

    // 处理消息
    void on_message(const websocketpp::connection_hdl& hdl, const websocketpp::server<websocketpp::config::asio>::message_ptr& msg);

    // 向所有客户端发送加速度数据
    void send_accelerate(float& x, float& y, float& z);

    // 启动服务器
    void run(uint16_t port);

    // 停止服务器
    void stop();

private:
    typedef websocketpp::server<websocketpp::config::asio> server;
    typedef std::set<websocketpp::connection_hdl,std::owner_less<websocketpp::connection_hdl>> con_list;

    server m_server;
    con_list m_connections;
};

#endif //BROADCAST_SERVER_H
