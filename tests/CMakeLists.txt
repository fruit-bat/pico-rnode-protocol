cmake_minimum_required(VERSION 3.12)
project(pico_kiss_protocol_host_tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

enable_testing()

add_executable(host_tests
  test_command_decoder.c
  test_command_encoder.c
  test_main.c
  test_utils.c
  ../src/pico-rnode-protocol-command-decoder.c
  ../src/pico-rnode-protocol-command-encoder.c
)

target_include_directories(host_tests PRIVATE
  ../src
  ${CMAKE_CURRENT_LIST_DIR}/tests
)

add_test(NAME host_tests COMMAND host_tests)
