cmake_minimum_required(VERSION 3.25)
project(sim VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

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
  target_compile_definitions(sim PRIVATE UNICODE _UNICODE)
endif()

target_compile_definitions(sim PRIVATE
  SRAL_STATIC
  # SPDLOG_COMPILED_LIB
  wxMSVC_VERSION_ABI_COMPAT # Important for wxWidgets v3.3.0+
)

# --- Dependencies with FetchContent ---
include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_dependencies)

# cli11
FetchContent_Declare(CLI11
GIT_REPOSITORY https://github.com/CLIUtils/CLI11
GIT_TAG v2.5.0
GIT_SHALLOW ON
SYSTEM
)
FetchContent_MakeAvailable(CLI11)

# miniaudio
FetchContent_Declare(miniaudio
  GIT_REPOSITORY https://github.com/mackron/miniaudio.git
  GIT_TAG 0.11.22
  GIT_SHALLOW ON
  SYSTEM
)
# Configure miniaudio build options
set(MINIAUDIO_NO_LIBVORBIS ON CACHE BOOL "Disable libvorbis as we don't use it")
set(MINIAUDIO_NO_LIBOPUS                    ON CACHE BOOL "Disable libopus as we don't use it")
# Disable unneeded audio backends
set(MINIAUDIO_ENABLE_ONLY_SPECIFIC_BACKENDS ON CACHE BOOL "Disable all backends by default")
# And enable the needed ones only
set(MINIAUDIO_ENABLE_WASAPI ON CACHE BOOL "Enable wasapi backend")
# Disable unused APIs
set(MINIAUDIO_NO_MP3                        ON CACHE BOOL "Disable mp3 as we will not use it")
set(MINIAUDIO_NO_WAV ON CACHE BOOL "Disable wav")
# Disable other features
set(MINIAUDIO_DEBUG_OUTPUT OFF CACHE BOOL "" FORCE) # switch to on in case of debug
set(MINIAUDIO_NO_EXTRA_NODES ON CACHE BOOL "" FORCE)
set(MINIAUDIO_NO_FLAC ON CACHE BOOL "" FORCE)
set(MINIAUDIO_NO_GENERATION ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(miniaudio)

# SRAL
FetchContent_Declare(SRAL
  GIT_REPOSITORY https://github.com/m1maker/SRAL.git
  GIT_TAG "0.4-RC"
  GIT_SHALLOW ON
  SYSTEM
)
set(BUILD_SRAL_TEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SRAL)
add_library(SRAL::SRAL_static ALIAS SRAL_static)

# spdlog
FetchContent_Declare(spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.15.3
  GIT_SHALLOW ON
  SYSTEM
)
# Ensure spdlog is built as a static library
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_NO_ATOMIC_LEVELS ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(spdlog)

# wxWidgets
FetchContent_Declare(wxWidgets
  GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
  GIT_TAG v3.3.0
  GIT_SHALLOW ON
  SYSTEM
)
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
FetchContent_MakeAvailable(wxWidgets)


target_include_directories(sim PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(sim PRIVATE
  miniaudio
  CLI11::CLI11
  SRAL::SRAL_static
  spdlog::spdlog_header_only
  wx::core
  wx::base
)
