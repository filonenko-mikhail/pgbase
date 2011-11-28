cmake_minimum_required(VERSION 2.8)

# Declare project
project(pgBase)

# QT CUSTOMIZATION
find_package(Qt4 REQUIRED)
# qt definitions
# dont using gui
# using qt script
set(QT_USE_QTSCRIPT       TRUE)
set(QT_USE_QTSCRIPTTOOLS  TRUE)
set(QT_USE_QTSVG          TRUE)
set(QT_USE_QTSQL TRUE)
include(${QT_USE_FILE})

# PROJECT NAME
string(TOUPPER ${PROJECT_NAME} PROJECT_TO_UPPER)

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

# SOURCE FILES AND DIRECTORIES
# simple sources
set(SOURCES)

# sources for using automoc, with "#include "*.moc"
set(Q_SOURCES)

# simple headers
set(HEADERS)
#header contains private classes
set(P_HEADERS)
# headers contains Q_OBJECT
set(Q_HEADERS)

# application forms *.ui
set(FORMS)

# application resources
set(RESOURCES)

# windows resources
set(WIN_RESOURCES)

# generated source code from FORMS
set(UI_HEADERS)
# generated moc_*.cxx from Q_HEADERS
set(MOC_SOURCES)
# generated qtc_*.cxx from RESOURCES
set(QRC_SOURCES)

# project version
set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (VERSION_RELEASE 0)
set (VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_RELEASE}" )

# -----------------------------------------------------------------------------------------------------
# 3rd party libraries
# ----------------------------------------------------------------
# log4qt ---------------------------------------------------------
find_package(log4qt REQUIRED HINTS "${QT_MKSPECS_DIR}/cmake" NO_DEFAULT_PATHS)
include_directories(${LOG4QT_INCLUDE_DIRS})

# all 3rd party subdirectories ---------------------------------------------------------
file(GLOB 3RDPARTY_DIRS "3rdparty/*")
foreach(3RDPARTY_DIR ${3RDPARTY_DIRS})
	if (IS_DIRECTORY ${3RDPARTY_DIR})
		add_subdirectory(${3RDPARTY_DIR})
	endif(IS_DIRECTORY ${3RDPARTY_DIR})
endforeach(3RDPARTY_DIR)

# Get project sources
file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE Q_HEADERS "*[^_p].h")
file(GLOB_RECURSE P_HEADERS "*_p.h")
file(GLOB_RECURSE Q_RESOURCES "*.qrc")
file(GLOB_RECURSE Q_FORMS "*.ui")

# Append application logo to windows resources
list(APPEND WIN_RESOURCE )


# Append qt application resources with main application resource
list(APPEND Q_RESOURCES "${CMAKE_SOURCE_DIR}/share/pgBase.qrc")

# Walk throw file list to
foreach(HEADER_FILE ${Q_HEADERS})
  get_filename_component(H_PATH ${HEADER_FILE} PATH)
  include_directories(${H_PATH})
endforeach(HEADER_FILE)


# -----------------------------------------------------------------------------------------------------

# -----------------------------------------------------------------------------------------------------
# GENERATING C++ CLASSES-WRAPPERS FOR QT SCRIPT
# Qt Script Bindings Generator options
set (QS_PACKAGE_PATH_PREFIX "plugin/script")
set (QS_PACKAGE_NAME "com.asvil.pgbase")
string(REPLACE "." "_" QS_PACKAGE_NAME_UND ${QS_PACKAGE_NAME})

set (QS_GENERATED_PATH "${CMAKE_CURRENT_BINARY_DIR}/${QS_PACKAGE_PATH_PREFIX}/generated_cpp/${QS_PACKAGE_NAME_UND}")

set (QS_BUILD_IN "${CMAKE_CURRENT_SOURCE_DIR}/${QS_PACKAGE_PATH_PREFIX}/plugin_build.txt.in")
set (QS_BUILD "${CMAKE_CURRENT_BINARY_DIR}/${QS_PACKAGE_PATH_PREFIX}/plugin_build.txt")

set (QS_TYPESYSTEM_IN "${CMAKE_CURRENT_SOURCE_DIR}/${QS_PACKAGE_PATH_PREFIX}/plugin_typesystem.xml.in")
set (QS_TYPESYSTEM "${CMAKE_CURRENT_BINARY_DIR}/${QS_PACKAGE_PATH_PREFIX}/plugin_typesystem.xml")

set (QS_MASTER_H "${CMAKE_CURRENT_BINARY_DIR}/${QS_PACKAGE_PATH_PREFIX}/master_include.h")

# before compilation we must generate plugin source files by qt script bindings generator
# exec string like this, where QSBG_DIR environment variable contains path to qscript bindings generator source directory
# $QSBG_DIR/generator/generator --output-directory=. --include-paths=$QTDIR/include
configure_file(
  ${QS_BUILD_IN}
  ${QS_BUILD}
  IMMEDIATE @ONLY)

configure_file(
  ${QS_TYPESYSTEM_IN}
  ${QS_TYPESYSTEM}
  IMMEDIATE @ONLY)


set(QSBG_DIR $ENV{QSBG_DIR})
if(NOT QSBG_DIR)
  get_filename_component(QSBG_DIR "../../qtscriptgenerator" ABSOLUTE)
  message(WARNING " Please set environment variable \"QSBG_DIR\", which must contains path to qt script bindings generator project directory. Used default: \"${QSBG_DIR}\"")
endif(NOT QSBG_DIR)

# Find qt script bindings generator
find_program(QSBG_EXECUTABLE generator PATHS ${QSBG_DIR}/generator ${QSBG_DIR}/generator/debug ${QSBG_DIR}/generator/release NO_DEFAULT_PATH)

# If plugin.cpp older than current CMakeLists.txt, than regenerate plugin sources by qt script bindings generator
if (UNIX)
  execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR} -newer ${QS_GENERATED_PATH}/plugin.cpp -regex ".*\\.h"
    RESULT_VARIABLE find_result1
    OUTPUT_VARIABLE find_outval1)
  execute_process(COMMAND find ${CMAKE_CURRENT_SOURCE_DIR}/${QS_PACKAGE_PATH_PREFIX} -newer ${QS_GENERATED_PATH}/plugin.cpp -regex ".*_typesystem\\.xml\\.in"
    RESULT_VARIABLE find_result2
    OUTPUT_VARIABLE find_outval2)
  if (NOT "${find_result1}" STREQUAL "0")
    set(GENERATE_CPP_WRAPPERS TRUE)
  endif (NOT "${find_result1}" STREQUAL "0")
  if (NOT "${find_result2}" STREQUAL "0")
    set(GENERATE_CPP_WRAPPERS TRUE)
  endif (NOT "${find_result2}" STREQUAL "0")
  if(find_outval1 OR find_outval2)
    set(GENERATE_CPP_WRAPPERS TRUE)
  endif(find_outval1 OR find_outval2)
else(UNIX)
  set(GENERATE_CPP_WRAPPERS TRUE)
endif (UNIX)

if (QSBG_EXECUTABLE AND GENERATE_CPP_WRAPPERS)
  ## DANGER Remove old master.h file
  file(REMOVE ${QS_MASTER_H})
	file(GLOB_RECURSE ALL_HEADERS "*[^_p].h")
  foreach(HEADER_FILE ${ALL_HEADERS})
    file(APPEND ${QS_MASTER_H} "#include \"${HEADER_FILE}\"\n")
  endforeach(HEADER_FILE)
  ## DANGER Remove directory with old generated sources
  # Walk throw file list to

  message("Qt script bindings generator executable founded")
  message("Generating c++ wrapper classes:")

  message("${QSBG_EXECUTABLE}\n"
    "${QS_MASTER_H}\n"
    "${QS_BUILD}\n"
    "--output-directory=${CMAKE_CURRENT_BINARY_DIR}/${QS_PACKAGE_PATH_PREFIX}")
  ## Execute qt script bindings generator executable
  execute_process(COMMAND ${QSBG_EXECUTABLE}
    ${QS_MASTER_H}
    ${QS_BUILD}
    --output-directory=${CMAKE_CURRENT_BINARY_DIR}/${QS_PACKAGE_PATH_PREFIX}
    WORKING_DIRECTORY ${QSBG_DIR}/generator
    RESULT_VARIABLE qsbg_retval
    OUTPUT_VARIABLE gsbg_outval
    ERROR_VARIABLE qsbg_errval)
  if (NOT "${qsbg_retval}" STREQUAL 0)
    message(FATAL_ERROR "Problem when generating c++ wrapper classes:"
      "${qsbg_errval}"
      "${qsbg_outval}"
      "${qsbg_retval}")
  else (NOT "${qsbg_retval}" STREQUAL 0)
    message("Generating output:"
      "${qsbg_errval}"
      "${qsbg_outval}")
    if(UNIX)
      execute_process(COMMAND touch ${QS_GENERATED_PATH}/plugin.cpp
        OUTPUT_VARIABLE touch_outval)
    endif(UNIX)
  endif (NOT "${qsbg_retval}" STREQUAL 0)
endif (QSBG_EXECUTABLE AND GENERATE_CPP_WRAPPERS)


include("${QS_GENERATED_PATH}/${QS_PACKAGE_NAME_UND}.cmake")

# Include sources generated by Qt Script Bindings Generator
include_directories(${QS_GENERATED_PATH})

# END CPP CLASSES-WRAPPERS FOR QT SCRIPT (OUTPUT SOURCES VARIABLE: QS_SOURCES)
# -----------------------------------------------------------------------------------------------------

# if you use Q_OBJECT
qt4_wrap_cpp(MOC_SOURCES ${Q_HEADERS})

qt4_automoc(${Q_SOURCES})

qt4_wrap_ui(UI_HEADERS ${Q_FORMS})

qt4_add_resources(QRC_SOURCES ${Q_RESOURCES})

set(TS_FILES ${CMAKE_SOURCE_DIR}/translations/pgbase_ru.ts)

#create app icon
set(WIN_RESOURCES)
if(MINGW AND WIN32)
# resource compilation for mingwtou
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/logo_rc.o
  COMMAND windres.exe -I"${CMAKE_SOURCE_DIR}/share/images/32x32/application/"
    -i "${CMAKE_SOURCE_DIR}/share/logo.rc"
    -o ${CMAKE_CURRENT_BINARY_DIR}/logo_rc.o)
  SET(WIN_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/logo_rc.o)
elseif((NOT MINGW) AND WIN32)
  SET(WIN_RESOURCES"${CMAKE_SOURCE_DIR}/share/logo.rc")
endif(MINGW AND WIN32)

# COMPILATION
set(RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
add_executable(${PROJECT_NAME} WIN32
  ${SOURCES}
  ${Q_SOURCES}
  ${MOC_SOURCES}
  ${QRC_SOURCES}
  ${HEADERS}
  ${Q_HEADERS}
  ${P_HEADERS}
  ${UI_HEADERS}
  ${QS_SOURCES}
  ${QS_HEADERS}
  ${WIN_RESOURCES}
  )

# LINKING
# main library
target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES}
  log4qt
  qtxmlsettings
  qtcsvparser
  qtmenuview
  qtwindowmenu
  )

# INSTALLATION
# install and export project target
# copy project file into qt_libdir and create project.cmake file for use this library in your cmake projects
install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

# install translations files
install(FILES ${QM_FILES} DESTINATION translations)
