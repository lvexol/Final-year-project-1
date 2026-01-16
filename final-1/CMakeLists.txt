#
# Setup
#
# include(cmake/BuildPackages.cmake)

cmake_minimum_required(VERSION 2.8)
if(POLICY CMP0022)
  cmake_policy(SET CMP0022 OLD)
endif()

# Internal cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
include(CheckCXXCompilerFlag)
include(CheckCSourceRuns)

include(CMakeMacroLibtoolFile)

project(vexovnc LANGUAGES C CXX)
if(NOT UNIX)
  message(FATAL_ERROR "vexoVNC is a UNIX-only project")
endif()
set(VERSION 0.9)

# The RC version must always be four comma-separated numbers
set(RCVERSION 0,9,0,0)

# Installation paths
set(BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin")
set(DATA_DIR "${CMAKE_INSTALL_PREFIX}/share")
set(MAN_DIR "${DATA_DIR}/man")
set(LOCALE_DIR "${DATA_DIR}/locale")
set(DOC_DIR "${CMAKE_INSTALL_PREFIX}/share/doc/${CMAKE_PROJECT_NAME}-${VERSION}")

if(NOT BUILD_TIMESTAMP)
  set(BUILD_TIMESTAMP "")
  execute_process(COMMAND "date" "+%Y-%m-%d %H:%M" OUTPUT_VARIABLE BUILD_TIMESTAMP)
  string(REGEX REPLACE "\n" "" BUILD_TIMESTAMP ${BUILD_TIMESTAMP})
endif()

# Default to optimised builds instead of debug ones. Our code has no bugs ;)
# (CMake makes it fairly easy to toggle this back to Debug if needed)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

message(STATUS "VERSION = ${VERSION}")
message(STATUS "BUILD_TIMESTAMP = ${BUILD_TIMESTAMP}")
add_definitions(-DBUILD_TIMESTAMP="${BUILD_TIMESTAMP}")

message(STATUS "WWWDIR = ${DATA_DIR}/vexovnc/www")
add_definitions(-DWWWDIR="${DATA_DIR}/vexovnc/www")

# We want to keep our asserts even in release builds so remove NDEBUG
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -UNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -UNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -UNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -UNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -UNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -UNDEBUG")

# Make sure we get a sane C version
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
set(CMAKE_CXX_STANDARD 20)

# Tell the compiler to be stringent
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wformat=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wformat=2")
# Make sure we catch these issues whilst developing
IF(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
ENDIF()

option(ENABLE_ASAN "Enable address sanitizer support" OFF)
if(ENABLE_ASAN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

option(ENABLE_TSAN "Enable thread sanitizer support" OFF)
if(ENABLE_TSAN AND CMAKE_SIZEOF_VOID_P MATCHES 8)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
endif()

# X11 (UNIX-only project, so always required)
find_package(X11 REQUIRED)

# Check for zlib
find_package(ZLIB REQUIRED)

# Check for libpng
find_package(PNG REQUIRED)

# Check for libjpeg
find_package(JPEG REQUIRED)

find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

# Use the JPEG library found by FindJPEG
include_directories(${JPEG_INCLUDE_DIR})

# Check for GNUTLS library
option(ENABLE_GNUTLS "Enable protocol encryption and advanced authentication" ON)
if(ENABLE_GNUTLS)
  find_package(GnuTLS)
  if (GNUTLS_FOUND)
    include_directories(${GNUTLS_INCLUDE_DIR})
    add_definitions("-DHAVE_GNUTLS")
    add_definitions(${GNUTLS_DEFINITIONS})
  endif()
endif()

# Check for PAM library
option(ENABLE_PAM "Enable PAM authentication support" ON)
if(ENABLE_PAM)
  check_include_files(security/pam_appl.h HAVE_PAM_H)
  set(CMAKE_REQUIRED_LIBRARIES -lpam)
  check_function_exists(pam_start HAVE_PAM_START)
  set(CMAKE_REQUIRED_LIBRARIES)
  if(HAVE_PAM_H AND HAVE_PAM_START)
    set(PAM_LIBS pam)
  else()
    set(ENABLE_PAM 0)
  endif()
endif()
set(HAVE_PAM ${ENABLE_PAM})

# Check for SSE2
check_cxx_compiler_flag(-msse2 COMPILER_SUPPORTS_SSE2)

# Generate config.h and make sure the source finds it
configure_file(config.h.in config.h)
add_definitions(-DHAVE_CONFIG_H)
include_directories(${CMAKE_BINARY_DIR})

include(cmake/StaticBuild.cmake)

add_subdirectory(third_party)
add_subdirectory(common)
add_subdirectory(unix)

if(ENABLE_NLS)
    add_subdirectory(po)
endif()

if (TESTS)
    add_subdirectory(tests)
endif()

# include(cmake/BuildPackages.cmake)

# uninstall
configure_file("${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "cmake_uninstall.cmake" IMMEDIATE @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P cmake_uninstall.cmake)
