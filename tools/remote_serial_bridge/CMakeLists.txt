cmake_minimum_required(VERSION 3.19)

set(APP_NAME "remote_serial_bridge")

project(${APP_NAME} VERSION "1.0.0" DESCRIPTION "remote serial bridge application")

set(LIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")

add_compile_options(-DHAIER_LOG_LEVEL=5)

include_directories("${LIB_ROOT}/include")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../utils")

list(APPEND SOURCE_FILES "${LIB_ROOT}/src/utils/haier_log.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../utils/console_log.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../utils/serial_stream.cpp")

add_executable("${APP_NAME}" "${SOURCE_FILES}")
