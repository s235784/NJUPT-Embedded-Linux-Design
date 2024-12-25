//
// Created by 诺天 on 2024/12/24.
//

#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"
#include <iostream>
#include <set>

#include "fmt/chrono.h"

#include "broadcast_server.h"

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

broadcast_server::broadcast_server() {
    m_server.init_asio();

    // 设置事件处理器
    m_server.set_open_handler(bind(&broadcast_server::on_open, this, ::_1));
    m_server.set_close_handler(bind(&broadcast_server::on_close, this, ::_1));
    m_server.set_message_handler(bind(&broadcast_server::on_message, this, ::_1, ::_2));
}

// 处理连接打开事件
void broadcast_server::on_open(const connection_hdl& hdl) {
    m_connections.insert(hdl);
}

// 处理连接关闭事件
void broadcast_server::on_close(const connection_hdl& hdl) {
    m_connections.erase(hdl);
}

// 处理收到消息事件
void broadcast_server::on_message(const connection_hdl& hdl, const server::message_ptr& msg) {
    std::cout << "on_message called with hdl: " << hdl.lock().get()
              << " and message: " << msg->get_payload()
              << std::endl;

    if (msg->get_payload() == "stop") {
        m_server.stop_listening();
        return;
    }

    try {
        m_server.send(hdl, msg->get_payload(), msg->get_opcode());
    } catch (websocketpp::exception const& e) {
        std::cout << "Echo failed because: "
                  << "(" << e.what() << ")" << std::endl;
    }
}

// 向所有客户端发送加速度数据
void broadcast_server::send_accelerate(float& x, float& y, float& z) {
    const std::string content = fmt::format("({:.3f},{:.3f},{:.3f});", x, y, z);
    for (const auto& m_connection : m_connections) {
        try {
            m_server.send(m_connection, content, websocketpp::frame::opcode::text);
        } catch (websocketpp::exception const& e) {
            std::cout << "Send failed because: "
                      << "(" << e.what() << ")" << std::endl;
        }
    }
}

// 运行服务器
void broadcast_server::run(const uint16_t port) {
    m_server.listen(port);
    m_server.start_accept();
    m_server.run();
}

// 停止服务器
void broadcast_server::stop() {
    std::cout << "Stopping Websocket";
    m_server.stop_listening();
}
