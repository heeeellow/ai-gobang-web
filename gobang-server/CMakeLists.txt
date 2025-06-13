cmake_minimum_required(VERSION 3.10)
project(gobang-server)

set(CMAKE_CXX_STANDARD 17)

# 递归所有子文件夹
file(GLOB SRC
    "*.cpp"
    "db/*.cpp"
    "model/*.cpp"
    "service/*.cpp"
    "network/*.cpp"
    "utils/*.cpp"
    "gobang/*.cpp"
)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/db
    ${CMAKE_SOURCE_DIR}/model
    ${CMAKE_SOURCE_DIR}/service
    ${CMAKE_SOURCE_DIR}/network
    ${CMAKE_SOURCE_DIR}/utils
    ${CMAKE_SOURCE_DIR}/gobang
    /usr/include/mysql
    /usr/include/nlohmann
)


add_executable(gobang_server ${SRC} )

# 链接MySQL客户端库和pthread
target_link_libraries(gobang_server mysqlclient pthread ssl crypto)
