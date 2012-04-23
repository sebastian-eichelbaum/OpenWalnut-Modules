#---------------------------------------------------------------------------
#
# Project: OpenWalnut ( http://www.openwalnut.org )
#
# Copyright 2009 OpenWalnut Community, BSV@Uni-Leipzig and CNCF@MPI-CBS
# For more information see http:#www.openwalnut.org/copying
#
# This file is part of OpenWalnut.
#
# OpenWalnut is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# OpenWalnut is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with OpenWalnut. If not, see <http:#www.gnu.org/licenses/>.
#
#---------------------------------------------------------------------------

INCLUDE( OpenWalnutUtils )

# name of the core lib. Used when linking modules or other GUI
SET( OW_LIB_OPENWALNUT "openwalnut" )

# ---------------------------------------------------------------------------------------------------------------------------------------------------
#
# This is executed on inclusion. It sets up everything needed. Beginning with compiler and OpenWalnut options and the third-party dependencies.
#
# ---------------------------------------------------------------------------------------------------------------------------------------------------

# guard against in-source builds
IF( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
  MESSAGE( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. (you may need to remove CMakeCache.txt" )
ENDIF()

# the build types
IF( NOT CMAKE_BUILD_TYPE )
    SET( CMAKE_BUILD_TYPE Release
         CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo"
         FORCE
       )
ENDIF( NOT CMAKE_BUILD_TYPE )

# guard against typos in build-type strings
STRING( TOLOWER "${CMAKE_BUILD_TYPE}" cmake_build_type_tolower)
IF( NOT cmake_build_type_tolower STREQUAL "debug" AND
    NOT cmake_build_type_tolower STREQUAL "release" AND
    NOT cmake_build_type_tolower STREQUAL "relwithdebinfo" AND
    NOT cmake_build_type_tolower STREQUAL "")
  MESSAGE( SEND_ERROR "Unknown build type \"${CMAKE_BUILD_TYPE}\". Allowed values are Debug, Release, RelWithDebInfo  and \"\" (case-insensitive).")
ENDIF()

# ---------------------------------------------------------------------------------------------------------------------------------------------------
#
# External Building Support (find OpenWalnut if needed)
#
# ---------------------------------------------------------------------------------------------------------------------------------------------------

# if this is included for external module building, find OpenWalnut.
IF( NOT ${OW_EXTERNAL_MODULE} )
    # to allow non-core code to access core and ext absolutely
    MESSAGE( STATUS "This is OpenWalnut Build System." )   
    INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR} )
ELSE()
    MESSAGE( STATUS "This is OpenWalnut Build System configured for external use." )   

    FIND_PATH( OPENWALNUT_INCLUDE_DIR core/kernel/WKernel.h ${OPENWALNUT_INCLUDEDIR} $ENV{OPENWALNUT_INCLUDEDIR} /usr/include/openwalnut /usr/local/include/openwalnut )
    FIND_LIBRARY( OPENWALNUT_LIBRARIES NAMES ${OW_LIB_OPENWALNUT} lib${OW_LIB_OPENWALNUT} HINTS 
                  ${OPENWALNUT_LIBDIR}
                  $ENV{OPENWALNUT_LIBDIR} 
                  /usr/lib
                  /usr/local/lib
                  /usr
                  /usr/local
                  /usr/local/openwalnut
                  /usr/local/OpenWalnut
                  $ENV{PROGRAMFILES}/OpenWalnut )

    SET( OPENWALNUT_FOUND FALSE )

    # do not confuse the user with this
    MARK_AS_ADVANCED( FORCE OPENWALNUT_INCLUDE_DIR )
    MARK_AS_ADVANCED( FORCE OPENWALNUT_LIBRARIES )
    MARK_AS_ADVANCED( FORCE OPENWALNUT_FOUND )

    # provide some output
    IF( OPENWALNUT_INCLUDE_DIR )
      MESSAGE( STATUS "Found OpenWalnut include files in ${OPENWALNUT_INCLUDE_DIR}." )
    ENDIF()
    IF( OPENWALNUT_LIBRARIES )
      MESSAGE( STATUS "Found OpenWalnut libs in ${OPENWALNUT_LIBRARIES}." )
    ENDIF()

    # really found?
    IF( OPENWALNUT_INCLUDE_DIR AND OPENWALNUT_LIBRARIES )
        SET( OPENWALNUT_FOUND TRUE )
        MESSAGE( STATUS "Found OpenWalnut." )
    ELSE()
        MESSAGE( FATAL_ERROR "Could not find OpenWalnut." )
    ENDIF()

    # include
    INCLUDE_DIRECTORIES( ${OPENWALNUT_INCLUDE_DIR} )
    SET( OW_LIB_OPENWALNUT ${OPENWALNUT_LIBRARIES} )
ENDIF()

# ---------------------------------------------------------------------------------------------------------------------------------------------------
#
# This is mandatory. Defines the target paths for building all elements in OpenWalnut
#
# ---------------------------------------------------------------------------------------------------------------------------------------------------

# mimic layout of install dir for build:
# these dirs are the same for all parts of OW
SET( OW_RUNTIME_DIR_RELATIVE "bin" )
SET( OW_RUNTIME_DIR ${PROJECT_BINARY_DIR}/${OW_RUNTIME_DIR_RELATIVE} )
SET( OW_LIBRARY_DIR_RELATIVE "lib" )
SET( OW_LIBRARY_DIR ${PROJECT_BINARY_DIR}/${OW_LIBRARY_DIR_RELATIVE} )
SET( OW_ARCHIVE_DIR_RELATIVE "lib" )
SET( OW_ARCHIVE_DIR ${PROJECT_BINARY_DIR}/${OW_ARCHIVE_DIR_RELATIVE} )
SET( OW_MODULE_DIR_RELATIVE "lib/openwalnut" )
SET( OW_MODULE_DIR ${PROJECT_BINARY_DIR}/${OW_MODULE_DIR_RELATIVE} )
SET( OW_MAN_DIR_RELATIVE "share/man" )
SET( OW_MAN_DIR "${PROJECT_BINARY_DIR}/share/man" )

# One could also define this for every part of OW, but it does not make sense. It is "share" because it is shared among the different parts
SET( OW_SHARE_DIR_RELATIVE "share/openwalnut" )
SET( OW_SHARE_DIR ${PROJECT_BINARY_DIR}/${OW_SHARE_DIR_RELATIVE} )

# where to find the doxygen config
SET( OW_DOXYGEN_DIR ${PROJECT_SOURCE_DIR}/../doc/developer )

# set our paths for install targets
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OW_RUNTIME_DIR} )
SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OW_LIBRARY_DIR} )
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OW_ARCHIVE_DIR} )

# ---------------------------------------------------------------------------------------------------------------------------------------------------
#
# Basic Build Setup Helpers
#
# ---------------------------------------------------------------------------------------------------------------------------------------------------

# This method configures the needed compiler flags and system specific options. You probably do not need to call this explciitly. It is done in
# BUILD_SYSTEM_SETUP
FUNCTION( BUILD_SYSTEM_COMPILER )
    # Unfortunately libstdc++'s header files don't work with mingw in ansi mode (basically libstdc++'s fault)
    IF( CMAKE_HOST_SYSTEM MATCHES "Windows" )
        SET( CMAKE_CXX_FLAGS "-frtti -pedantic -Wall -Wno-long-long -Wextra " CACHE STRING "" FORCE )
    ELSE()
        SET( CMAKE_CXX_FLAGS "-frtti -pedantic -ansi -Wall -Wno-long-long -Wextra " CACHE STRING "" FORCE )
    ENDIF()

    # Darwin's ld isn't GNU and doesn't like the following
    IF( NOT CMAKE_SYSTEM_NAME MATCHES "Darwin" )
        ## The following allows us to prevent cyclic dependencies even on linux
        SET( CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined -Wl,--allow-shlib-undefined,--as-needed" CACHE STRING "" FORCE )
        SET( CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed" CACHE STRING "" FORCE )
    ENDIF()
    SET( CMAKE_CXX_FLAGS_RELEASE "-O3" CACHE STRING "" FORCE )
    SET( CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG -O0" CACHE STRING "" FORCE )
    SET( CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -DDEBUG -O2" CACHE STRING "" FORCE )
ENDFUNCTION( BUILD_SYSTEM_COMPILER )

# ---------------------------------------------------------------------------------------------------------------------------------------------------
# Compiler setup
# ---------------------------------------------------------------------------------------------------------------------------------------------------

BUILD_SYSTEM_COMPILER()

# ---------------------------------------------------------------------------------------------------------------------------------------------------
# OpenWalnut specific options
# ---------------------------------------------------------------------------------------------------------------------------------------------------

# OpenWalnut specific options
# other options
OPTION( OW_HANDLE_SHADERS "This ensures that shaders are available in build directory after build." ON )

# sorry, linking not available properly on windows, Cygwin supports this but we do not want special rules for thousands of environments.
# ==> keep it clean
IF( NOT CMAKE_HOST_SYSTEM MATCHES "Windows" )
    OPTION( OW_LINK_SHADERS "If turned on, shaders do not get copied. They get linked. This is a nice option for developers." OFF )
ENDIF()

# Provide several options to control some aspects of resource copy.
OPTION( OW_PACKAGE_BUILD "Enable this to get fine-grained control over several resources and files getting installed. This is very handy for package building." OFF )
IF( OW_PACKAGE_BUILD )
    SET( OW_PACKAGE_PACKAGER "cpack" CACHE STRING "Package builder. Set this to enable packager-specific options during install." )

    OPTION( OW_PACKAGE_NOCOPY_LICENSE "Disable to copy our licensing information. Enabling this can be useful for package maintainer since several packaging systems have their own licence mechanism (i.e. Debian)." OFF )
    OPTION( OW_PACKAGE_NOCOPY_COREFONTS "Enable this if you have liberation fonts installed on your system. They will be linked. If disabled, our fonts are copied." OFF )
ENDIF()

# ---------------------------------------------------------------------------------------------------------------------------------------------------
# The openwalnut executable should print the revision/tag
# ---------------------------------------------------------------------------------------------------------------------------------------------------

# Generate needed headers
# NOTE: add a line ADD_DEPENDENCIES( XYZ OW_generate_version_header ) to your target XYZ if you need the header!
SET( OW_VERSION_HEADER_DIRECTORY ${PROJECT_BINARY_DIR}/versionHeader )
SET( OW_VERSION_HEADER ${OW_VERSION_HEADER_DIRECTORY}/WVersion.h )
# to allow all those targets to find the header:
INCLUDE_DIRECTORIES( ${OW_VERSION_HEADER_DIRECTORY} )
# Setup the target
SETUP_VERSION_HEADER( ${OW_VERSION_HEADER} )

# Set the OW version string. This can be used by others for setting target versions during compilation.
GET_VERSION_STRING( OW_VERSION OW_LIB_VERSION )
MESSAGE( STATUS "OW Version: \"${OW_VERSION}\"; OW Lib Version: \"${OW_LIB_VERSION}\"." )

# We need a SOVERSION too. This somehow describes the API compatibility. We use the major number here.
SPLIT_VERSION_STRING( ${OW_LIB_VERSION} OW_VERSION_MAJOR OW_VERSION_MINOR OW_VERSION_PATCH )
SET( OW_SOVERSION ${OW_VERSION_MAJOR} )

# ---------------------------------------------------------------------------------------------------------------------------------------------------
#
# REQUIRED third party libs
#
# ---------------------------------------------------------------------------------------------------------------------------------------------------

# -----------------------------------------------------------------------------------------------------------------------------------------------
# Boost, at least 1.39
# See http://www.boost.org/
#
# To see, which boost libs we currently use, you may run the following command # in the src directory on a linux box to make some investigations:
# grep -i include `find . -type f` | grep boost | awk '{print $2}' | sort | uniq

# Setup boost options
SET( Boost_USE_MULTITHREAD ON )

# find the boost packages
FIND_PACKAGE( Boost 1.39.0 REQUIRED program_options thread filesystem date_time system signals regex )

# include the boost headers
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

# avoid filesystem 2 stuff
ADD_DEFINITIONS( "-DBOOST_FILESYSTEM_VERSION=3" )

# -----------------------------------------------------------------------------------------------------------------------------------------------
# OpenGL, at least 1.2
# See http://www.opengl.org
#
FIND_PACKAGE( OpenGL REQUIRED )

# include the OpenGL header paths
INCLUDE_DIRECTORIES( ${OPENGL_INCLUDE_DIR} )

# -----------------------------------------------------------------------------------------------------------------------------------------------
# OpenSceneGraph, at least 2.8.0
# See http://www.openscenegraph.org

# find the needed packages
SET( MIN_OSG_VERSION 2.8.0 )
FIND_PACKAGE( OpenSceneGraph ${MIN_OSG_VERSION} REQUIRED osgDB osgUtil osgGA osgViewer osgSim osgWidget osgText )
IF( OPENSCENEGRAPH_FOUND )
    INCLUDE_DIRECTORIES( ${OPENSCENEGRAPH_INCLUDE_DIRS} )
ENDIF()

# NOTE: sorry but this is needed since those vars spam the ccmake. The user should find the important options fast!

# Remove all of them
MARK_AS_ADVANCED( FORCE OSG_INCLUDE_DIR )
MARK_AS_ADVANCED( FORCE OSG_LIBRARY )
MARK_AS_ADVANCED( FORCE OSG_LIBRARY_DEBUG )
MARK_AS_ADVANCED( FORCE OSGDB_INCLUDE_DIR )
MARK_AS_ADVANCED( FORCE OSGDB_LIBRARY )
MARK_AS_ADVANCED( FORCE OSGDB_LIBRARY_DEBUG )
MARK_AS_ADVANCED( FORCE OSGGA_INCLUDE_DIR )
MARK_AS_ADVANCED( FORCE OSGGA_LIBRARY )
MARK_AS_ADVANCED( FORCE OSGGA_LIBRARY_DEBUG )
MARK_AS_ADVANCED( FORCE OSGSIM_INCLUDE_DIR )
MARK_AS_ADVANCED( FORCE OSGSIM_LIBRARY )
MARK_AS_ADVANCED( FORCE OSGSIM_LIBRARY_DEBUG )
MARK_AS_ADVANCED( FORCE OSGUTIL_INCLUDE_DIR )
MARK_AS_ADVANCED( FORCE OSGUTIL_LIBRARY )
MARK_AS_ADVANCED( FORCE OSGUTIL_LIBRARY_DEBUG )
MARK_AS_ADVANCED( FORCE OSGTEXT_INCLUDE_DIR )
MARK_AS_ADVANCED( FORCE OSGTEXT_LIBRARY )
MARK_AS_ADVANCED( FORCE OSGTEXT_LIBRARY_DEBUG )
MARK_AS_ADVANCED( FORCE OSGWIDGET_INCLUDE_DIR )
MARK_AS_ADVANCED( FORCE OSGWIDGET_LIBRARY )
MARK_AS_ADVANCED( FORCE OSGWIDGET_LIBRARY_DEBUG )
MARK_AS_ADVANCED( FORCE OSGVIEWER_INCLUDE_DIR )
MARK_AS_ADVANCED( FORCE OSGVIEWER_LIBRARY )
MARK_AS_ADVANCED( FORCE OSGVIEWER_LIBRARY_DEBUG )

MARK_AS_ADVANCED( FORCE OPENTHREADS_INCLUDE_DIR )
MARK_AS_ADVANCED( FORCE OPENTHREADS_LIBRARY )
MARK_AS_ADVANCED( FORCE OPENTHREADS_LIBRARY_DEBUG )

# -----------------------------------------------------------------------------------------------------------------------------------------------
# Eigen3, at least 3.0.0
# See http://eigen.tuxfamily.org

FIND_PACKAGE( eigen3 REQUIRED )
IF( EIGEN3_FOUND )
    INCLUDE_DIRECTORIES( ${EIGEN3_INCLUDE_DIR} )

    # NOTE: this is included in ext. But we need to set several definitions to make this work on 32 Bit machines due to alignment problems
    SET( EIGEN3_DEFINES -DEIGEN_DONT_VECTORIZE -DEIGEN_DONT_ALIGN -DEIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT )
    ADD_DEFINITIONS( ${EIGEN3_DEFINES} )
ENDIF()

# ---------------------------------------------------------------------------------------------------------------------------------------------------
#
# Unit Testing
#
#  - We use cxxtest. See http://cxxtest.tigris.org
#
# ---------------------------------------------------------------------------------------------------------------------------------------------------

# Try to find it
FIND_PACKAGE( CxxTest QUIET )

# If it is found, and OW_USE_TESTS is on, build the tests and activate CMake's test mechanisms
IF( CXXTEST_FOUND )
  # To enable testing
  OPTION( OW_USE_TESTS "This enables compilation of tests" ON )
  INCLUDE_DIRECTORIES( ${CXXTEST_INCLUDE_DIRS} )
  INCLUDE_DIRECTORIES( ${CXXTEST_INCLUDE_DIR} ) # NOTE: old FindCXXTest versions used this name
  IF( OW_USE_TESTS )
    SET( OW_COMPILE_TESTS ON ) #We need this variable because this is tested more often.
    # Package settings:
    SET( CXXTEST_USE_PYTHON 1 )
    # Activate CTest and "test" target
    ENABLE_TESTING()

    # we want a more verbose testing too.
    ADD_CUSTOM_TARGET( vtest
                       COMMAND $(MAKE) test ARGS="-V"
                       COMMENT "Runs the test in verboseness to see what actually went wrong"
                     )
  ELSE()
    SET( OW_COMPILE_TESTS OFF )
  ENDIF()
ELSE()
  SET( OW_COMPILE_TESTS OFF )
ENDIF()

# ---------------------------------------------------------------------------------------------------------------------------------------------------
#
# Documentation
#  - Call doxygen here
#
# ---------------------------------------------------------------------------------------------------------------------------------------------------

IF( EXISTS ${OW_DOXYGEN_DIR}/doxygenConfig )
    #let doxygen do the work
    ADD_CUSTOM_TARGET( doc
                       COMMAND doxygen ${OW_DOXYGEN_DIR}/doxygenConfig
                       WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/..
                       COMMENT "Build doxygen documentation"
                       VERBATIM
                     )
ENDIF()

# On Windows, we want the common doc (README, AUTHORS and COPYING) to be in the root install dir too:
IF( CMAKE_HOST_SYSTEM MATCHES "Windows" )
    SETUP_COMMON_DOC( "." "COMMON_DOC_ON_WINDOWS" )
ENDIF()

# ---------------------------------------------------------------------------------------------------------------------------------------------------
#
# Style
#  - We use brainlint for this
#
# ---------------------------------------------------------------------------------------------------------------------------------------------------

# SETUP_STYLECHECKER adds targets for each lib which then add theirself to this target
ADD_CUSTOM_TARGET( stylecheck )

