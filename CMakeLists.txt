cmake_minimum_required(VERSION 3.19)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/library.json LIB_PROPERTIES)
string(JSON LIB_NAME GET ${LIB_PROPERTIES} "name")
string(JSON LIB_VERSION GET ${LIB_PROPERTIES} "version")
string(JSON LIB_DESCRIPTION GET ${LIB_PROPERTIES} "description")
string(JSON LIB_URL GET ${LIB_PROPERTIES} "repository" "url")

project(${LIB_NAME} VERSION ${LIB_VERSION} 
                    DESCRIPTION ${LIB_DESCRIPTION}
                    HOMEPAGE_URL ${LIB_URL})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/Protocol/haier_message.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/Protocol/haier_protocol.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/Transport/haier_frame.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/Transport/protocol_transport.cpp")
list(APPEND SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/Utils/haier_log.cpp")

add_library("${LIB_NAME}" STATIC "${SOURCE_FILES}")
