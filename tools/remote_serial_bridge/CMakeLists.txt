cmake_minimum_required(VERSION 3.19)

set(APP_NAME "remote_serial_bridge")

project(${APP_NAME} VERSION "1.0.0" DESCRIPTION "remote serial bridge application")

set(LIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")

add_executable("${APP_NAME}")

target_compile_options("${APP_NAME}" PRIVATE  -DHAIER_LOG_LEVEL=5)

if (UNIX)
    OPTION(BUILD_WITH_CURSES "Define if application should use curses library" ON)
    if (BUILD_WITH_CURSES)
        message("Building with curses library")
        find_package(Curses REQUIRED)
        target_include_directories("${APP_NAME}" PRIVATE ${CURSES_INCLUDE_DIR})
        target_compile_options("${APP_NAME}" PRIVATE -DUSE_CURSES=true)
        target_compile_options("${APP_NAME}" PRIVATE -DPRESS_KEY_TO_CLOSE_CONSOLE=true)
        target_link_libraries("${APP_NAME}" ${CURSES_LIBRARY})
    endif(BUILD_WITH_CURSES)
endif (UNIX)

if(WIN32)
  target_link_libraries("${APP_NAME}" ws2_32)
endif (WIN32)

target_include_directories("${APP_NAME}" PRIVATE
    "${LIB_ROOT}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils"
)

target_sources("${APP_NAME}" PRIVATE
    "${LIB_ROOT}/src/utils/haier_log.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/console_log.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/serial_stream.cpp"
)

