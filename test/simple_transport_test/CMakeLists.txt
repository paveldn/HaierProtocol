cmake_minimum_required(VERSION 3.19)

set(TEST_NAME "simple_transport_test")

project(${TEST_NAME} VERSION "1.0.0" DESCRIPTION "Simple transport level test")

set(LIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(TOOLS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../tools")

add_compile_options(-DHAIER_LOG_LEVEL=5)
add_compile_options(-DHAIER_LOG_TAG="haier")
add_compile_options(-DRUN_ALL_TESTS)

include_directories("${LIB_ROOT}/include" "${CMAKE_CURRENT_SOURCE_DIR}/../utils" "${TOOLS_PATH}/utils")

list(APPEND SOURCE_FILES "${TOOLS_PATH}/utils/console_log.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test.cpp")

add_executable("${TEST_NAME}" "${SOURCE_FILES}")

add_subdirectory(${LIB_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/HaierProtocol")

target_link_libraries("${TEST_NAME}" HaierProtocol)
