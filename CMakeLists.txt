cmake_minimum_required (VERSION 3.2.0)
project (muscord)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (DEBUG)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

include (GNUInstallDirs)

file(GLOB_RECURSE ALL_SOURCE_FILES
    src/*.cpp src/*.h    
)

#
# discord-rpc config
#

message("-- Looking for discord-rpc source files")

find_file (DISCORD_RPC_LIB_TEST NAMES discord-rpc HINTS ${CMAKE_CURRENT_SOURCE_DIR} CMAKE_FIND_ROOT_PATH_BOTH)

if(NOT DISCORD_RPC_LIB_TEST)
    message("-- Looking for discord-rpc source files - not found")
    execute_process(COMMAND git submodule update --init -- discord-rpc WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

message("-- Looking for discord-rpc source files - found")
set(BUILD_EXAMPLES OFF CACHE BOOL "Build example apps" FORCE)
add_subdirectory(${DISCORD_RPC_LIB_TEST})

#
# yaml-cpp config
#

message("-- Looking for yaml-cpp source files")

find_file (YAML_LIB_TEST NAMES yaml-cpp HINTS ${CMAKE_CURRENT_SOURCE_DIR} CMAKE_FIND_ROOT_PATH_BOTH)

if(NOT YAML_LIB_TEST)
    message("-- Looking for yaml-cpp source files - not found")
    execute_process(COMMAND git submodule update --init -- discord-rpc WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

message("-- Looking for yaml-cpp source files - found")
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Enable testing" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Enable parse tools" FORCE)
set(YAML_CPP_INSTALL OFF CACHE BOOL "Enable generation of install target" FORCE)
add_subdirectory(${YAML_LIB_TEST})


find_package (PkgConfig REQUIRED)
find_package (Threads REQUIRED)
pkg_check_modules (PLAYERCTL REQUIRED playerctl)

add_executable (muscord ${ALL_SOURCE_FILES})

add_dependencies (muscord discord-rpc yaml-cpp)

target_link_libraries(muscord discord-rpc yaml-cpp ${PLAYERCTL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories (muscord PUBLIC ${PLAYERCTL_INCLUDE_DIRS})
target_compile_options(muscord PUBLIC ${PLAYERCTL_CFLAGS_OTHER})
