set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")

include(FetchContent)

cmake_minimum_required(VERSION 3.26)

project(program_design LANGUAGES CXX)

cmake_policy(SET CMP0167 NEW)
cmake_policy(SET CMP0169 OLD)
# === boost ===
set(BOOST_STATIC_LIB ON)
find_package(Boost REQUIRED COMPONENTS filesystem system thread regex)

# === websocket++ ===
FetchContent_Declare(websocketpp
GIT_REPOSITORY https://github.com/zaphoyd/websocketpp.git
  GIT_TAG 0.8.2)
FetchContent_GetProperties(websocketpp)
if(NOT websocketpp_POPULATED)
  FetchContent_Populate(websocketpp)
  add_subdirectory(${websocketpp_SOURCE_DIR} ${websocketpp_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
# add interface library with all websocketpp dependencies
add_library(Websockets INTERFACE)
target_include_directories(Websockets INTERFACE ${websocketpp_SOURCE_DIR})
target_link_libraries(Websockets INTERFACE Boost::system Boost::thread Boost::regex)

# === fmt ===
FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG        e69e5f977d458f2650bb346dadf2ad30c5320281) # 10.2.1
FetchContent_MakeAvailable(fmt)

find_package(SQLite3 REQUIRED)

# === program_design ===
add_executable(program_design
        main.cpp
)
target_link_libraries(program_design SQLite::SQLite3)