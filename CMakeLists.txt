cmake_minimum_required (VERSION 3.2.0)
project (muscord)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (DEBUG)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "Debug")
endif()
set(DISCORD_RPC_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/discord-rpc)

include (GNUInstallDirs)

file(GLOB_RECURSE ALL_SOURCE_FILES
    src/*.cpp src/*.h    
)

find_file (DISCORD_RPC_LIB_TEST NAMES discord-rpc HINTS ${CMAKE_CURRENT_SOURCE_DIR} CMAKE_FIND_ROOT_PATH_BOTH)

if(NOT DISCORD_RPC_LIB_TEST)
    message("Unable to find discord-rpc")
    execute_process(COMMAND git submodule update --init -- discord-rpc WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else()
    message("Found discord-rpc: ${DISCORD_RPC_LIB_TEST}")
	set(BUILD_EXAMPLES OFF CACHE BOOL "Build example apps" FORCE)
    add_subdirectory(${DISCORD_RPC_LIB_TEST})
endif()

find_package (PkgConfig REQUIRED)
find_package (Threads REQUIRED)
pkg_check_modules (PLAYERCTL REQUIRED playerctl)

add_executable (muscord ${ALL_SOURCE_FILES})

add_dependencies (muscord discord-rpc)

target_link_libraries(muscord discord-rpc ${PLAYERCTL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories (muscord PUBLIC ${PLAYERCTL_INCLUDE_DIRS})
target_compile_options(muscord PUBLIC ${PLAYERCTL_CFLAGS_OTHER})
