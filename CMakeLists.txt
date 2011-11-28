cmake_minimum_required(VERSION 2.6)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")

# postfix for library
# if CMAKE_BUILD_TYPE == Debug then library_name become library_name + CMAKE_DEBUG_POSTFIX
set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

project(pgBase-application)

string(TOUPPER ${PROJECT_NAME} PROJECT_TO_UPPER)

set(VENDOR_NAME "org.asvil")

find_package(Qt4 REQUIRED)

# ADDITIONAL QT VARS, FUNCTIONS, MACROS

# COMPILER customization
# compilers flags
if (MSVC)
  add_definitions (/W3)
elseif (CMAKE_COMPILER_IS_GNUCXX)
  add_definitions (-Wall)
else ()
  message ("Unknown compiler")
endif ()

#compiler options
set(OPTIMIZATION_FLAGS "-O2" )

message("Building project ${PROJECT_NAME}")

message("
Configuration
-------------
CMAKE_BUILD_TYPE        = ${CMAKE_BUILD_TYPE}
CXXFLAGS                = $ENV{CXXFLAGS}
CMAKE_CXX_FLAGS         = ${CMAKE_CXX_FLAGS}
LDFLAGS                 = $ENV{LDFLAGS}
CMAKE_EXE_LINKER_FLAGS  = ${CMAKE_EXE_LINKER_FLAGS}
CMAKE_INSTALL_PREFIX    = ${CMAKE_INSTALL_PREFIX}
")

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  set(CMAKE_VERBOSE_MAKEFILE TRUE)
endif ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/*/")
foreach(INCLUDE_DIR ${INCLUDE_DIRS})
  include_directories(${INCLUDE_DIR})
endforeach(INCLUDE_DIR)

add_subdirectory(src)

# UNINSTALL TARGET
# create cmake uninstall files
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
