cmake_minimum_required(VERSION 3.19)

set(TEST_NAME "smartair2_simulator")

project(${TEST_NAME} VERSION "1.0.0" DESCRIPTION "smartAir2 AC simulator")

set(LIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")

add_executable("${TEST_NAME}")

add_subdirectory(${LIB_ROOT} "${CMAKE_CURRENT_BINARY_DIR}/HaierProtocol")

target_compile_options("${TEST_NAME}" PRIVATE  -DHAIER_LOG_LEVEL=5)

if (UNIX)
    OPTION(BUILD_WITH_CURSES "Define if application should use curses library" ON)
    if (BUILD_WITH_CURSES)
        message("Building with curses library")
        find_package(Curses REQUIRED)
        target_include_directories("${TEST_NAME}" PRIVATE ${CURSES_INCLUDE_DIR})
        target_compile_options("${TEST_NAME}" PRIVATE -DUSE_CURSES=true)
        target_compile_options("${TEST_NAME}" PRIVATE -DPRESS_KEY_TO_CLOSE_CONSOLE=true)
        target_link_libraries("${TEST_NAME}" ${CURSES_LIBRARY})
    endif (BUILD_WITH_CURSES)
endif (UNIX)

target_include_directories("${TEST_NAME}" PRIVATE
    "${LIB_ROOT}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils"
)

target_sources("${TEST_NAME}" PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/console_log.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/serial_stream.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/simulator_base.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../utils/smartair2_server.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

target_link_libraries("${TEST_NAME}" HaierProtocol)
