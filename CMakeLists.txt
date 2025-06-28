cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(VCPKG_TARGET_TRIPLET x64-windows-static)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(sim VERSION 0.1)

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "MSVC Runtime Library" FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
  message(STATUS "Setting build type to '${CMAKE_BUILD_TYPE}' as none was specified.")
endif()

file(GLOB SIM_SOURCES "src/*.cpp")

add_executable(sim ${SIM_SOURCES})

if(WIN32)
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Dependencies with fetch content
include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/_dependencies CACHE PATH "Base directory for FetchContent" FORCE)
file(MAKE_DIRECTORY ${FETCHCONTENT_BASE_DIR})

# miniaudio
FetchContent_Declare(miniaudio
GIT_REPOSITORY https://github.com/mackron/miniaudio.git
GIT_TAG 0.11.22
GIT_SHALLOW ON
)
# Configure miniaudio build options
set(MINIAUDIO_NO_LIBVORBIS ON CACHE BOOL "Disable libvorbis as we don't use it")
set(MINIAUDIO_NO_LIBOPUS                    ON CACHE BOOL "Disable libopus as we don't use it")
# Disable unneeded audio backends
set(MINIAUDIO_ENABLE_ONLY_SPECIFIC_BACKENDS ON CACHE BOOL "Disable all backends by default")
# And enable the needed ones only
set(MINIAUDIO_ENABLE_WASAPI ON CACHE BOOL "Enable wasapi backend")
# Disable unused APIs
set(MINIAUDIO_NO_ENCODING                   ON CACHE BOOL "Disable encoding")
set(MINIAUDIO_NO_MP3                        ON CACHE BOOL "Disable mp3 as we will not use it")
set(MINIAUDIO_NO_WAV ON CACHE BOOL "Disable wav as we use flac")
FetchContent_MakeAvailable(miniaudio)

# SRAL
FetchContent_Declare(SRAL
GIT_REPOSITORY https://github.com/m1maker/SRAL.git
GIT_TAG "f1aa26d"
GIT_SHALLOW ON
)
target_compile_definitions(sim PUBLIC SRAL_STATIC)
FetchContent_MakeAvailable(SRAL)

# spdlog
FetchContent_Declare(spdlog
GIT_REPOSITORY https://github.com/gabime/spdlog.git
GIT_TAG v1.15.3
GIT_SHALLOW ON
)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "Build spdlog as a static library" FORCE) # Ensure spdlog itself is a static lib
target_compile_definitions(sim PUBLIC SPDLOG_COMPILED_LIB) # This tells your code spdlog is a compiled lib
FetchContent_MakeAvailable(spdlog)

# Find and connect wxwidgets
set(wxBUILD_SHARED OFF)
set(wxBUILD_COMPATIBILITY 3.1)
set(wxBUILD_MONOLITHIC ON)
set(wxBUILD_SAMPLES OFF CACHE BOOL "Don't build wxWidgets samples")
set(wxBUILD_TESTS OFF CACHE BOOL "Don't build wxWidgets tests")
set(wxBUILD_DEMOS OFF CACHE BOOL "Don't build wxWidgets demos")
set(wxUSE_MEDIACTRL OFF CACHE BOOL "Disable media control if not needed")
set(wxUSE_WEBVIEW OFF CACHE BOOL "Disable webview if not needed")
set(wxUSE_RICHTEXT OFF CACHE BOOL "Disable rich text if not needed")

find_package(wxWidgets REQUIRED core base)
include(${wxWidgets_USE_FILE})

target_include_directories(sim PRIVATE
"${CMAKE_CURRENT_SOURCE_DIR}/src"
"${wxWidgets_INCLUDE_DIRS}"
)

target_link_libraries(sim PRIVATE miniaudio SRAL_static spdlog ${wxWidgets_LIBRARIES})
