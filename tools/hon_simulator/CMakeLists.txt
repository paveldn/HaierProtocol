cmake_minimum_required(VERSION 3.19)

set(APP_NAME "hon_simulator")

project(${APP_NAME} VERSION "1.1.0" DESCRIPTION "hOn AC simulator")

set(LIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")

add_compile_definitions(HAIER_LOG_LEVEL=5)

add_subdirectory(${LIB_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/HaierProtocol")

set(INCLUDE_DIRS 
    "${LIB_ROOT}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils"
)

set(LINK_LIBS HaierProtocol)

if (UNIX)
    OPTION(BUILD_WITH_CURSES "Define if application should use curses library" ON)
    if (BUILD_WITH_CURSES)
        message("Building with curses library")
        find_package(Curses REQUIRED)
        set(INCLUDE_DIRS ${INCLUDE_DIRS} ${CURSES_INCLUDE_DIR})
        add_compile_definitions(USE_CURSES=true)
        add_compile_definitions(PRESS_KEY_TO_CLOSE_CONSOLE=true)
        set(LINK_LIBS ${LINK_LIBS} ${CURSES_LIBRARY})
    endif(BUILD_WITH_CURSES)
endif (UNIX)

include_directories(${INCLUDE_DIRS})

link_libraries(${LINK_LIBS})

add_executable("${APP_NAME}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/console_log.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/serial_stream.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/hon_server.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/simulator_base.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/console_log.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/serial_stream.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/hon_server.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/simulator_base.h"
)

