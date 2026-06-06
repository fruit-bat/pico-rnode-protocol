add_library(pico_rnode_protocol INTERFACE)

target_sources(pico_rnode_protocol INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/src/**.c
)

target_include_directories(pico_rnode_protocol INTERFACE
    include
)
