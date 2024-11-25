cmake_minimum_required(VERSION 3.20)
project(shm-lockfree-queue VERSION 1.2.0 LANGUAGES CXX)

# --- Project Options ---
option(SHM_IPC_BUILD_TESTS "Build unit tests" ON)
option(SHM_IPC_BUILD_BENCHMARKS "Build benchmarks" ON)
option(SHM_IPC_BUILD_TOOLS "Build cli tools" ON)
option(SHM_IPC_USE_HUGEPAGES "Enable HugeTLB support" ON)
option(SHM_IPC_ENABLE_ASAN "Enable AddressSanitizer" OFF)

# --- Compiler Settings ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall -Wextra -Wpedantic -Werror 
        -Wno-unused-parameter -march=native -O3
    )
endif()

# --- Dependencies ---
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
find_package(Threads REQUIRED)

# --- Library Definition ---
add_library(shm_ipc_core INTERFACE)
target_include_directories(shm_ipc_core INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(shm_ipc_core INTERFACE Threads::Threads)

if(SHM_IPC_USE_HUGEPAGES)
    target_compile_definitions(shm_ipc_core INTERFACE SHM_USE_HUGEPAGES)
endif()

# --- Subdirectories ---
if(SHM_IPC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(SHM_IPC_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(SHM_IPC_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

# --- Installation ---
include(GNUInstallDirs)
install(TARGETS shm_ipc_core EXPORT shm_ipc_targets)
install(DIRECTORY include/shm_ipc DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

message(STATUS "Build Configuration Summary:")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  HugePages: ${SHM_IPC_USE_HUGEPAGES}")
message(STATUS "  Benchmarks: ${SHM_IPC_BUILD_BENCHMARKS}")