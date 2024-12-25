#include "websocketpp/config/asio_no_tls.hpp"
#include <iostream>
#include <thread>
#include <atomic>
#include <sqlite3.h>

#include "broadcast_server.h"
#include "fmt/chrono.h"

sqlite3* db;
const std::string dbName = "sensor_data.db";

bool init_sqlite() {
	char* errMsg = nullptr;
	int rc;

	rc = sqlite3_open(dbName.c_str(), &db);
	if (rc) {
		std::cerr << "can't open: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}

	std::string sql = "CREATE TABLE IF NOT EXISTS SensorData ("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
		"AccelX REAL NOT NULL, "
		"AccelY REAL NOT NULL, "
		"AccelZ REAL NOT NULL, "
		"Time TEXT NOT NULL);";
	rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK) {
		std::cerr << errMsg << std::endl;
		sqlite3_free(errMsg);
		sqlite3_close(db);
		return false;
	}
	std::cout << "Table Created Successes." << std::endl;
	return true;
}

bool insertSensorData(double accelX, double accelY, double accelZ) {
	char* errMsg = nullptr;  //用于存储错误信息

	//准备插入数据的SQL语句
	std::string sql = "INSERT INTO SensorData (AccelX, AccelY, AccelZ, Time) VALUES (" +
					  std::to_string(accelX) + ", " +
					  std::to_string(accelY) + ", " +
					  std::to_string(accelZ) + ", " +
					  	"strftime('%Y-%m-%d %H:%M:%S', 'now'));";

	int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK) {
		std::cerr << "mistake of inserting data : " << errMsg << std::endl;
		sqlite3_free(errMsg);
		sqlite3_close(db);
		return false;
	}
	return true;
}

void close_sqlite() {
	sqlite3_close(db);
}

int main() {
	if (!init_sqlite()) {
		return 0;
	}

    broadcast_server server;

    std::thread server_thread([&] {
        constexpr uint16_t port = 9002;
        std::cout << "Server initialized. Listening port: " + std::to_string(port) << std::endl;
        server.run(port);
    });

    // 定时器：主线程每隔1秒发送一次加速度数据
    std::atomic running(true);
    std::thread timer_thread([&] {
    	int fd;
		const char *filename = "/dev/adxl345";
		signed int databuf[3];
		double accel_x_act, accel_y_act, accel_z_act;

		long ret = 0;

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
				insertSensorData(accel_x_act, accel_y_act, accel_z_act);
			}

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
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
	close_sqlite();
}