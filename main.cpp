#include "websocketpp/config/asio_no_tls.hpp"
#include <iostream>
#include <thread>
#include <atomic>

#include "broadcast_server.h"
#include "fmt/chrono.h"

int main() {
    broadcast_server server;

    std::thread server_thread([&] {
        constexpr uint16_t port = 9002;
        std::cout << "Server initialized. Listening port: " + std::to_string(port) << std::endl;
        server.run(port);
    });

    std::cout << "Main thread." << std::endl;

    // 定时器：主线程每隔1秒发送一次加速度数据
    std::atomic running(true);
    std::thread timer_thread([&] {
    	int fd;
		const char *filename = "/dev/adxl345";
		signed int databuf[3];
		float accel_x_act, accel_y_act, accel_z_act;

		int ret = 0;

		fd = open(filename, O_RDWR);
		if(fd < 0) {
			printf("can't open file %s\r\n", filename);
			return;
		}

        while (running) {
        	ret = read(fd, databuf, sizeof(databuf));
			if(ret == 0) { 			/* 数据读取成功 */
				accel_x_act = databuf[0] * 3.9 * 0.001;
				accel_y_act = databuf[1] * 3.9 * 0.001;
				accel_z_act = databuf[2] * 3.9 * 0.001;

				server.send_accelerate(accel_x_act, accel_y_act, accel_z_act);
			}

            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 休眠0.5秒
        }
    });

    // 等待线程
    std::cout << "Press Enter to stop the server..." << std::endl;
    std::cin.get();  // 等待用户按下回车键

    // 停止定时器和服务器
    running = false;  // 停止定时器线程
    timer_thread.join();  // 等待定时器线程结束

    server.stop();
    server_thread.join();
}