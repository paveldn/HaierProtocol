cmake_minimum_required(VERSION 3.19)

set(TEST_NAME "hon_test")

project(${TEST_NAME} VERSION "1.0.0" DESCRIPTION "hOn protocol test")

set(LIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(TOOLS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../tools")

add_compile_options(-DHAIER_LOG_LEVEL=5)

include_directories("${LIB_ROOT}/include" "${CMAKE_CURRENT_SOURCE_DIR}/../utils" "${TOOLS_PATH}/utils")

list(APPEND SOURCE_FILES "${LIB_ROOT}/src/protocol/haier_message.cpp")
list(APPEND SOURCE_FILES "${LIB_ROOT}/src/protocol/haier_protocol.cpp")
list(APPEND SOURCE_FILES "${LIB_ROOT}/src/transport/haier_frame.cpp")
list(APPEND SOURCE_FILES "${LIB_ROOT}/src/transport/protocol_transport.cpp")
list(APPEND SOURCE_FILES "${LIB_ROOT}/src/utils/haier_log.cpp")
list(APPEND SOURCE_FILES "${TOOLS_PATH}/utils/console_log.cpp")
list(APPEND SOURCE_FILES "${TOOLS_PATH}/utils/hon_server.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../utils/virtual_stream.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

add_executable("${TEST_NAME}" "${SOURCE_FILES}")
