cmake_minimum_required(VERSION 3.19)

set(TEST_NAME "hon_simulator")

project(${TEST_NAME} VERSION "1.0.0" DESCRIPTION "hOn AC simulator")

set(LIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")

add_compile_options(-DHAIER_LOG_LEVEL=5)

if (UNIX)
    find_package(Curses REQUIRED)
    include_directories(${CURSES_INCLUDE_DIR})
    add_compile_options(-DUSE_CURSES)
endif (UNIX)

include_directories("${LIB_ROOT}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../utils")

list(APPEND SOURCE_FILES "${LIB_ROOT}/src/protocol/haier_message.cpp")
list(APPEND SOURCE_FILES "${LIB_ROOT}/src/protocol/haier_protocol.cpp")
list(APPEND SOURCE_FILES "${LIB_ROOT}/src/transport/haier_frame.cpp")
list(APPEND SOURCE_FILES "${LIB_ROOT}/src/transport/protocol_transport.cpp")
list(APPEND SOURCE_FILES "${LIB_ROOT}/src/utils/haier_log.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../utils/console_log.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../utils/serial_stream.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../utils/hon_server.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../utils/simulator_base.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

add_executable("${TEST_NAME}" "${SOURCE_FILES}")

if (UNIX)
    target_link_libraries(${TEST_NAME} ${CURSES_LIBRARY})
endif (UNIX)

