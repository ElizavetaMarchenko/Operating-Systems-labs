cmake_minimum_required(VERSION 3.28)
project(lab2_5)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall -Werror")
set(QT6_PATH "/usr/lib/x86_64-linux-gnu/cmake/Qt6")


project(wolf_game VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_PREFIX_PATH ${QT6_PATH})


find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt6_wrap_cpp(MOC_HEADERS_SERVER src/host/HostWindow.hpp)
qt6_wrap_cpp(MOC_HEADERS_CLIENT src/client/ClientWindow.hpp)


add_executable(host_mq src/connections/conn_mq.cpp src/host/host.cpp ${MOC_HEADERS_SERVER})
target_compile_definitions(host_mq PRIVATE MQ_Connection)
target_link_libraries(host_mq Qt6::Widgets)

add_executable(host_fifo src/connections/conn_fifo.cpp src/host/host.cpp ${MOC_HEADERS_SERVER})
target_compile_definitions(host_fifo PRIVATE FIFO_Connection)
target_link_libraries(host_fifo Qt6::Widgets)

add_executable(host_seg src/connections/conn_seg.cpp src/host/host.cpp ${MOC_HEADERS_SERVER})
target_compile_definitions(host_seg PRIVATE SEG_Connection)
target_link_libraries(host_seg Qt6::Widgets)



add_executable(client_mq src/connections/conn_mq.cpp src/client/client.cpp ${MOC_HEADERS_CLIENT})
target_compile_definitions(client_mq PRIVATE MQ_Connection)
target_link_libraries(client_mq Qt6::Widgets)

add_executable(client_fifo src/connections/conn_fifo.cpp src/client/client.cpp ${MOC_HEADERS_CLIENT})
target_compile_definitions(client_fifo PRIVATE FIFO_Connection)
target_link_libraries(client_fifo Qt6::Widgets)

add_executable(client_seg src/connections/conn_seg.cpp src/client/client.cpp ${MOC_HEADERS_CLIENT})
target_compile_definitions(client_seg PRIVATE SEG_Connection)
target_link_libraries(client_seg Qt6::Widgets)