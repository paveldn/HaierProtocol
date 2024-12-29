cmake_minimum_required(VERSION 3.19)

set(TEST_NAME "hon_test")

project(${TEST_NAME} VERSION "1.0.0" DESCRIPTION "hOn protocol test")

set(LIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(TOOLS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../tools")
set(TEST_UTILS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../utils")

add_compile_definitions(RUN_ALL_TESTS HAIER_LOG_LEVEL=5)

add_subdirectory(${LIB_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/HaierProtocol")

include_directories("${LIB_ROOT}/include" "${CMAKE_CURRENT_SOURCE_DIR}/../utils" "${TOOLS_PATH}/utils")

link_libraries(HaierProtocol)

add_executable("${TEST_NAME}"
    "${TOOLS_PATH}/utils/console_log.cpp"
    "${TOOLS_PATH}/utils/hon_server.cpp"
    "${TEST_UTILS_PATH}/virtual_stream.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${TOOLS_PATH}/utils/console_log.h"
    "${TOOLS_PATH}/utils/hon_server.h"
    "${TEST_UTILS_PATH}/virtual_stream.h"
)
