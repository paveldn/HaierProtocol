cmake_minimum_required(VERSION 3.19)

set(TEST_NAME "simple_transport_test")

project(${TEST_NAME} VERSION "1.0.0" DESCRIPTION "Simple transport level test")

set(LIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")

add_compile_options(-DHAIER_LOG_LEVEL=5)

include_directories("${LIB_ROOT}/include")

list(APPEND SOURCE_FILES "${LIB_ROOT}/src/Protocol/haier_message.cpp")
list(APPEND SOURCE_FILES "${LIB_ROOT}/src/Protocol/haier_protocol.cpp")
list(APPEND SOURCE_FILES "${LIB_ROOT}/src/Transport/haier_frame.cpp")
list(APPEND SOURCE_FILES "${LIB_ROOT}/src/Transport/protocol_transport.cpp")
list(APPEND SOURCE_FILES "${LIB_ROOT}/src/Utils/haier_log.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test.cpp")

add_executable("${TEST_NAME}" "${SOURCE_FILES}")
