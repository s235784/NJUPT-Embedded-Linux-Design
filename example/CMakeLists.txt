cmake_minimum_required(VERSION 3.10)

project(BlueTooth LANGUAGES C)

# 指定 C 标准（可选）
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# 添加源文件
set(SOURCES bluetooth.c)

# 添加可执行文件
add_executable(BlueTooth ${SOURCES})