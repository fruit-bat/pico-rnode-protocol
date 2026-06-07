cmake_minimum_required(VERSION 3.12)
project(pico_rnode_protocol_monitor C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(rnode-monitor
  src/main.c
)

# Build and link the pico-serial-proxy helper library (sibling project)
add_subdirectory(../../../pico-serial-proxy ${CMAKE_CURRENT_BINARY_DIR}/pico_serial_proxy_build)
add_subdirectory(../../../pico-rnode-protocol ${CMAKE_CURRENT_BINARY_DIR}/pico_rnode_protocol_build)
add_subdirectory(../../../pico-kiss-protocol ${CMAKE_CURRENT_BINARY_DIR}/pico_kiss_protocol_build)

target_link_libraries(rnode-monitor
  PRIVATE pico_rnode_protocol
  PRIVATE pico_kiss_protocol
  PRIVATE pico_serial_proxy
)
