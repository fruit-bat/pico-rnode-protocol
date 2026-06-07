add_library(pico_rnode_protocol INTERFACE)

target_sources(pico_rnode_protocol INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/src/pico-rnode-protocol-command-decoder.c
  ${CMAKE_CURRENT_LIST_DIR}/src/pico-rnode-protocol-command-decoder-text.c
  ${CMAKE_CURRENT_LIST_DIR}/src/pico-rnode-protocol-command-encoder.c
  ${CMAKE_CURRENT_LIST_DIR}/src/pico-rnode-protocol-stream.c
  ${CMAKE_CURRENT_LIST_DIR}/src/pico-rnode-protocol-frame.c
  ${CMAKE_CURRENT_LIST_DIR}/src/pico-rnode-protocol-encoder.c
  ${CMAKE_CURRENT_LIST_DIR}/src/pico-rnode-protocol-event-decoder.c
  ${CMAKE_CURRENT_LIST_DIR}/src/pico-rnode-protocol-event-encoder.c
)

target_include_directories(pico_rnode_protocol INTERFACE
    include
)
