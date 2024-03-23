cmake_minimum_required(VERSION 3.19)

# Activate C++ 11
set (CMAKE_CXX_STANDARD 11)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/library.json LIB_PROPERTIES)
string(JSON LIB_NAME GET ${LIB_PROPERTIES} "name")
string(JSON LIB_VERSION GET ${LIB_PROPERTIES} "version")
string(JSON LIB_DESCRIPTION GET ${LIB_PROPERTIES} "description")
string(JSON LIB_URL GET ${LIB_PROPERTIES} "repository" "url")

project(${LIB_NAME} VERSION ${LIB_VERSION} 
                    DESCRIPTION ${LIB_DESCRIPTION}
                    HOMEPAGE_URL ${LIB_URL})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/haier_message.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/haier_protocol.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/transport/haier_frame.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/transport/protocol_transport.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/haier_log.cpp")

add_library("${LIB_NAME}" STATIC "${SOURCE_FILES}")
