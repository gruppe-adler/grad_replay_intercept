cmake_minimum_required (VERSION 3.0)


file(GLOB_RECURSE INTERCEPT_PLUGIN_SOURCES *.h *.hpp *.c *.cpp)
SOURCE_GROUP("src" FILES ${INTERCEPT_PLUGIN_SOURCES})

#If you want to split your source files into different directories you can do so here

#The SOURCE_GROUP string is the directory it will display as inside your visual studio.
#Here is a example of a "utilities" subdirectory.

#file(GLOB INTERCEPT_plugin_utilities_SOURCES "utilities/*.cpp" "utilities/*.hpp" "utilities/*.h")
#SOURCE_GROUP("src/utilities" FILES ${INTERCEPT_plugin_utilities_SOURCES})

# Version file
ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/../addons/main/script_version.hpp
           ${CMAKE_CURRENT_BINARY_DIR}/../addons/main/_script_version.hpp # fake file
           #${CMAKE_CURRENT_SOURCE_DIR}/_version.cpp
    COMMAND ${CMAKE_COMMAND} -DVERSIONFILE_PATH=${CMAKE_CURRENT_SOURCE_DIR}/../addons/main/script_version.hpp -DSRCDIR=${CMAKE_CURRENT_SOURCE_DIR}/.. -P
            ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/version.cmake)

# nlohmann json
find_package(nlohmann_json CONFIG REQUIRED)

# CPR
find_package(cpr CONFIG REQUIRED)

# Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)
FIND_PACKAGE(Boost 1.82.0 REQUIRED COMPONENTS program_options iostreams REQUIRED)

#include the Intercept headers from the submodule
set(INTERCEPT_CLIENT_PATH "${CMAKE_SOURCE_DIR}/intercept/src/client")

set(INTERCEPT_INCLUDE_PATH "${INTERCEPT_CLIENT_PATH}/headers" "${INTERCEPT_CLIENT_PATH}/headers/shared" "${INTERCEPT_CLIENT_PATH}/headers/client/" "${INTERCEPT_CLIENT_PATH}/headers/client/sqf")

if(USE_64BIT_BUILD)
    set(INTERCEPT_PLUGIN_NAME "${CMAKE_PROJECT_NAME}_x64")
else()
    set(INTERCEPT_PLUGIN_NAME "${CMAKE_PROJECT_NAME}")
endif()

add_definitions(/DINTERCEPT_NO_THREAD_SAFETY)

if(USE_ENGINE_TYPES)
    add_definitions(/DINTERCEPT_SQF_STRTYPE_RSTRING)
endif()

file(GLOB INTERCEPT_HOST_SOURCES "${INTERCEPT_CLIENT_PATH}/intercept/client/*.cpp"  "${INTERCEPT_CLIENT_PATH}/intercept/client/sqf/*.cpp" "${INTERCEPT_CLIENT_PATH}/intercept/shared/*.cpp")
SOURCE_GROUP("intercept" FILES ${INTERCEPT_HOST_SOURCES})

add_library(${INTERCEPT_PLUGIN_NAME} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/../addons/main/script_version.hpp ${INTERCEPT_PLUGIN_SOURCES} ${INTERCEPT_HOST_SOURCES})

# HEMTT Build
add_custom_command(TARGET ${INTERCEPT_PLUGIN_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -DSRC_PATH=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/hemtt.cmake)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${INTERCEPT_INCLUDE_PATH})
include_directories(${Boost_INCLUDE_DIR})

set_target_properties(${INTERCEPT_PLUGIN_NAME} PROPERTIES PREFIX "")
set_target_properties(${INTERCEPT_PLUGIN_NAME} PROPERTIES FOLDER "${CMAKE_PROJECT_NAME}")

target_link_libraries(${INTERCEPT_PLUGIN_NAME} nlohmann_json nlohmann_json::nlohmann_json)
target_link_libraries(${INTERCEPT_PLUGIN_NAME} cpr::cpr)

if(MSVC)
    target_link_libraries(${INTERCEPT_PLUGIN_NAME} CRYPT32.LIB)
endif()

target_link_libraries(${INTERCEPT_PLUGIN_NAME} ${Boost_LIBRARIES})

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++1z -O2 -s -fPIC -fpermissive -static-libgcc -static-libstdc++ -Wno-attributes")#-march=i686 -m32
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(CMAKE_SHARED_LINKER_FLAGS "-shared -static-libgcc -static-libstdc++")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1 /MP /EHsc")
    set(CMAKE_CXX_FLAGS_RELEASE "/MT /Zi /O2 /Ob1 /EHsc /MP") #with debug info
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:LIBCMT") # LIBCMT conflicts with POCO
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/OPT:REF /DEBUG:FULL /MANIFEST:NO") 
endif()

install(TARGETS ${INTERCEPT_PLUGIN_NAME} RUNTIME DESTINATION bin)
