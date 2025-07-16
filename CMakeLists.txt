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

project(sim VERSION 1.0)

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
target_compile_definitions(sim PRIVATE _UNICODE=1)
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
set(MINIAUDIO_NO_ENCODING                   OFF CACHE BOOL "Disable encoding")
set(MINIAUDIO_NO_MP3                        ON CACHE BOOL "Disable mp3 as we will not use it")
set(MINIAUDIO_NO_WAV ON CACHE BOOL "Disable wav")
# Disable other features
set(MINIAUDIO_DEBUG_OUTPUT OFF CACHE BOOL "" FORCE) # switch to on in case of debug
set(MINIAUDIO_NO_DECODING OFF CACHE BOOL "" FORCE)
set(MINIAUDIO_NO_EXTRA_NODES ON CACHE BOOL "" FORCE)
set(MINIAUDIO_NO_FLAC ON CACHE BOOL "" FORCE)
set(MINIAUDIO_NO_GENERATION ON CACHE BOOL "" FORCE)
set(MINIAUDIO_NO_NODE_GRAPH OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(miniaudio)

# SRAL
FetchContent_Declare(SRAL
GIT_REPOSITORY https://github.com/m1maker/SRAL.git
GIT_TAG "48440dd"
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
set(wxBUILD_SHARED OFF CACHE BOOL "" FORCE)
set(wxBUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(wxBUILD_TESTS OFF CACHE BOOL "" FORCE)
set(wxBUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(wxBUILD_INSTALL OFF CACHE BOOL "" FORCE)

# Foundational features that are generally required and unsafe to disable
set(wxUSE_INTL ON CACHE BOOL "Internationalization is required by wxbase" FORCE)
set(wxUSE_XLOCALE ON CACHE BOOL "Required for C++ locale functions" FORCE)

# Safe-to-disable features if you don't use them
set(wxUSE_AUI OFF CACHE BOOL "" FORCE)
set(wxUSE_HTML OFF CACHE BOOL "" FORCE)
set(wxUSE_MEDIACTRL OFF CACHE BOOL "" FORCE)
set(wxUSE_OPENGL OFF CACHE BOOL "" FORCE)
set(wxUSE_PROPGRID OFF CACHE BOOL "" FORCE)
set(wxUSE_QA OFF CACHE BOOL "" FORCE)
set(wxUSE_RIBBON OFF CACHE BOOL "" FORCE)
set(wxUSE_RICHTEXT OFF CACHE BOOL "" FORCE)
set(wxUSE_STC OFF CACHE BOOL "" FORCE)
set(wxUSE_WEBVIEW OFF CACHE BOOL "" FORCE)
set(wxUSE_XRC OFF CACHE BOOL "" FORCE)
set(wxUSE_WXHTML_HELP OFF CACHE BOOL "" FORCE)

# Other options
set(wxBUILD_OPTIMISE ON CACHE BOOL "" FORCE)
set(wxBUILD_STRIPPED_RELEASE ON CACHE BOOL "" FORCE)
set(wxWidgets_USE_STATIC ON CACHE BOOL "" FORCE)
set(wxUSE_EXTENDED_RTTI OFF CACHE BOOL "" FORCE)
set(wxUSE_NO_RTTI OFF CACHE BOOL "" FORCE)

#target_compile_definitions(sim PRIVATE WXUSINGDLL=1)

FetchContent_Declare(
wxWidgets
GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
GIT_TAG        v3.3.0
GIT_SHALLOW     ON
)
FetchContent_GetProperties(wxWidgets)
FetchContent_MakeAvailable(wxWidgets)

#find_package(wxWidgets REQUIRED core base)
#include(${wxWidgets_USE_FILE})

target_include_directories(sim PRIVATE
"${CMAKE_CURRENT_SOURCE_DIR}/src"
"${wxWidgets_INCLUDE_DIRS}"
"${wxwidgets_SOURCE_DIR}/include"
"${wxwidgets_SOURCE_DIR}/include/msvc"
"${wxwidgets_SOURCE_DIR}"
"${wxwidgets_BINARY_DIR}"
"${CMAKE_BINARY_DIR}/lib/vc_x64_lib/"
"${CMAKE_CURRENT_SOURCE_DIR}/_dependencies/wxwidgets-build/lib/vc_x64_lib/mswud"
)

target_link_directories(sim PRIVATE
"${CMAKE_BINARY_DIR}/lib/vc_x64_lib"
"${CMAKE_BINARY_DIR}/lib/vc_x64_dll"
"${wxwidgets_BINARY_DIR}/lib/vc_x64_dll"
"${wxwidgets_BINARY_DIR}/lib/vc_x64_lib"
)

target_link_libraries(sim PRIVATE miniaudio SRAL_static spdlog wx::base wx::core)
