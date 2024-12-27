cmake_minimum_required(VERSION 3.19)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/library.json LIB_PROPERTIES)
string(JSON LIB_NAME GET ${LIB_PROPERTIES} "name")
string(JSON LIB_VERSION GET ${LIB_PROPERTIES} "version")
string(JSON LIB_DESCRIPTION GET ${LIB_PROPERTIES} "description")
string(JSON LIB_URL GET ${LIB_PROPERTIES} "repository" "url")

project(${LIB_NAME} VERSION ${LIB_VERSION} 
                    DESCRIPTION ${LIB_DESCRIPTION}
                    HOMEPAGE_URL ${LIB_URL})

set(LIB_INCLUDE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_SRC_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/src")

include_directories("${LIB_INCLUDE_FOLDER}")

list(APPEND LIB_FOLDERS "protocol" "transport" "utils")

foreach (LIB_FOLDER ${LIB_FOLDERS})
	file(GLOB FILES_INC "${LIB_INCLUDE_FOLDER}/${LIB_FOLDER}/*.h")
	file(GLOB FILES_SRC "${LIB_SRC_FOLDER}/${LIB_FOLDER}/*.cpp")
	list(APPEND SOURCE_FILES ${FILES_SRC})
	list(APPEND INCLUDE_FILES ${FILES_INC})
	source_group("${LIB_FOLDER}" FILES ${FILES_SRC} ${FILES_INC})
endforeach()

add_library("${LIB_NAME}" STATIC ${SOURCE_FILES} ${INCLUDE_FILES})



