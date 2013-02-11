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

# ---------------------------------------------------------------------------------------------------------------------------------------------------
#
# General CMake Setup
#
# ---------------------------------------------------------------------------------------------------------------------------------------------------

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT( FiberClusteringToolbox )

# ---------------------------------------------------------------------------------------------------------------------------------------------------
#
# Include the OpenWalnut build system.
#
# ---------------------------------------------------------------------------------------------------------------------------------------------------

# where to find our utils and cmake modules?
SET( OW_TOOLS_DIR ${PROJECT_SOURCE_DIR}/../tools )

# append search path for FindModules:
LIST( APPEND CMAKE_MODULE_PATH ${OW_TOOLS_DIR}/cmake )

# These scripts contains all the needed tools to setup the build:
# * Compiler Setup
# * Common OpenWalnut Options
# * Third-party Dependencies Setup
# * Unit Testing Setup if found
# * Doxygen Setup if found
INCLUDE( OpenWalnut )

# ---------------------------------------------------------------------------------------------------------------------------------------------------
#
# Setup your additional dependencies and tools
#
# ---------------------------------------------------------------------------------------------------------------------------------------------------

# IMPORTANT: NEVER add any commands bore the line INCLUDE( OpenWalnut ). This might cause problems!

FIND_PACKAGE( CUDA QUIET )
IF( CUDA_FOUND )
    OPTION( OW_USE_CUDA "Enable Cuda support." )
    IF( OW_USE_CUDA )
        # BugFix: Subsequent calls don't set CUDA_VERSION_MAJOR or .._MINOR variables, so we have to extract them from CUDA_VERSION variable
        STRING(REGEX REPLACE "([0-9]+)\\.([0-9]+).*" "\\1" CUDA_VERSION_MAJOR ${CUDA_VERSION})
        STRING(REGEX REPLACE "([0-9]+)\\.([0-9]+).*" "\\2" CUDA_VERSION_MINOR ${CUDA_VERSION})

        # Enforce at least CUDA version 2.3, since the detTractClustering is know to require this at least
        SET( OW_MIN_CUDA_MAJOR_VERSION 2 )
        SET( OW_MIN_CUDA_MINOR_VERSION 3 )

        IF( CUDA_VERSION_MAJOR LESS OW_MIN_CUDA_MAJOR_VERSION )
            SET( CUDA_FOUND_BUT_LOW_VERSION 1 )
        ELSEIF( CUDA_VERSION_MAJOR EQUAL OW_MIN_CUDA_MAJOR_VERSION )
            IF( CUDA_VERSION_MINOR LESS OW_MIN_CUDA_MINOR_VERSION )
                SET( CUDA_FOUND_BUT_LOW_VERSION 1 )
            ENDIF()
        ENDIF()

        # abort incase invalid version of CUDA was found.
        IF( CUDA_FOUND_BUT_LOW_VERSION )
            MESSAGE( FATAL_ERROR "You have selected to use CUDA but an insufficent version: ${CUDA_VERSION_MAJOR}.${CUDA_VERSION_MINOR} was found, but ${OW_MIN_CUDA_MAJOR_VERSION}.${OW_MIN_CUDA_MINOR_VERSION} is required." )
        ENDIF()
        MESSAGE( STATUS "CUDA version: ${CUDA_VERSION_MAJOR}.${CUDA_VERSION_MINOR} was found." )

        # set some common options
        ADD_DEFINITIONS( -DCUDA_FOUND )
        set( CUDA_ATTACH_VS_BUILD_RULE_TO_CUDA_FILE ON )
        set( BUILD_SHARED_LIBS ON )
        set( CUDA_NVCC_FLAGS --compiler-options -fno-inline --ptxas-options=-v -arch=sm_12 )
        set( CUDA_PROPAGATE_HOST_FLAGS OFF )
        INCLUDE_DIRECTORIES( ${CUDA_TOOLKIT_INCLUDE} )
        INCLUDE_DIRECTORIES( "${CUDA_SDK_ROOT_DIR}/common/inc" )
    ENDIF( OW_USE_CUDA )
ENDIF( CUDA_FOUND )

# ---------------------------------------------------------------------------------------------------------------------------------------------------
#
# Setup all modules
#
# ---------------------------------------------------------------------------------------------------------------------------------------------------

# SETUP_MODULE(
#     ${PROJECT_NAME}     # use project name as module(-toolbox) name
#     "."                 # where to find the sources
#     "${CUDA_LIBRARIES}" # no additonal dependencies
#     ""                  # no sources to exclude
# )

# The follwoing code is taken from SETUP_MODULE OW-Cmake function as there was no way for integrating ADD_CUDA_LIBRARY over parameters
SET( MODULE_NAME "ClusteringToolbox" ) # as all modules will be integrated into this "module" we call it toolbox
SET( MODULE_TARGET_DIR_RELATIVE ${OW_MODULE_DIR_RELATIVE}/${MODULE_NAME} )
SET( MODULE_TARGET_RESOURCE_DIR_RELATIVE ${OW_SHARE_DIR_RELATIVE}/modules/${MODULE_NAME} )
SET( MODULE_TARGET_DIR ${PROJECT_BINARY_DIR}/${MODULE_TARGET_DIR_RELATIVE} )
SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MODULE_TARGET_DIR} )
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MODULE_TARGET_DIR} )
SET( MODULE_SOURCE_DIR "." ) # use current directory

COLLECT_COMPILE_FILES( "${MODULE_SOURCE_DIR}" TARGET_CPP_FILES TARGET_H_FILES TARGET_TEST_FILES )

IF( CUDA_FOUND AND OW_USE_CUDA )
    FILE( GLOB_RECURSE CUDA_SRC "*.cu" )
    CUDA_ADD_LIBRARY( ${MODULE_NAME} SHARED ${TARGET_CPP_FILES} ${TARGET_H_FILES} ${CUDA_SRC} )
    TARGET_LINK_LIBRARIES( ${MODULE_NAME} ${CMAKE_STANDARD_LIBRARIES} ${OW_LIB_OPENWALNUT} ${Boost_LIBRARIES} ${OPENGL_gl_LIBRARY} ${OPENSCENEGRAPH_LIBRARIES} ${CUDA_LIBRARIES} )
ELSE()
    FILE( GLOB_RECURSE CUDA_CPP_FILES ${MODULE_SOURCE_DIR}/*Cuda*.cpp )
    FILE( GLOB_RECURSE CUDA_H_FILES   ${MODULE_SOURCE_DIR}/*Cuda*.h )
    LIST( REMOVE_ITEM TARGET_H_FILES ${CUDA_H_FILES} )
    LIST( REMOVE_ITEM TARGET_CPP_FILES ${CUDA_CPP_FILES} )
    ADD_LIBRARY( ${MODULE_NAME} SHARED ${TARGET_CPP_FILES} ${TARGET_H_FILES} )
    TARGET_LINK_LIBRARIES( ${MODULE_NAME} ${CMAKE_STANDARD_LIBRARIES} ${OW_LIB_OPENWALNUT} ${Boost_LIBRARIES} ${OPENGL_gl_LIBRARY} ${OPENSCENEGRAPH_LIBRARIES} )
ENDIF()

SET_TARGET_PROPERTIES( ${MODULE_NAME} PROPERTIES VERSION ${OW_LIB_VERSION} SOVERSION ${OW_SOVERSION} )

SETUP_LIB_INSTALL( ${MODULE_NAME} ${MODULE_TARGET_DIR_RELATIVE} "MODULES" )

IF( CUDA_FOUND AND OW_USE_CUDA )
    SETUP_TESTS( "${TARGET_TEST_FILES}" "${MODULE_NAME}" "${CUDA_LIBRARIES}" )
ELSE()
    SETUP_TESTS( "${TARGET_TEST_FILES}" "${MODULE_NAME}" )
ENDIF()

COLLECT_SHADER_FILES( ${MODULE_SOURCE_DIR} TARGET_GLSL_FILES )
SETUP_SHADERS( "${TARGET_GLSL_FILES}" "${MODULE_TARGET_RESOURCE_DIR_RELATIVE}/shaders" "MODULES" )
SETUP_RESOURCES_GENERIC( "${MODULE_SOURCE_DIR}/../resources/FiberClusteringResources" ${MODULE_TARGET_RESOURCE_DIR_RELATIVE} "${MODULE_NAME}" "MODULES" )
SETUP_STYLECHECKER( "${MODULE_NAME}"
    "${TARGET_CPP_FILES};${TARGET_H_FILES};${TARGET_TEST_FILES};${TARGET_GLSL_FILES}"  # add all these files to the stylechecker
    "${_MODULE_STYLE_EXCLUDES}" )                                                      # exlude some ugly files


# ---------------------------------------------------------------------------------------------------------------------------------------------------
#
# Additional Setup
#
# ---------------------------------------------------------------------------------------------------------------------------------------------------

# Add the files under ../resources/MyGlobalResources as additional, global resources
# The first parameter is the directory name, the second parameter is the component name. This is used during installation to identify to which
# install component this belongs. The name is arbitrary but must be unique.
#
# NOTE: The first parameter of SETUP_MODULE is not allowed as component name here because SETUP_MODULE already registers this component. Simply
# append some string.
#SETUP_GLOBAL_RESOURCES( "FiberClusteringResources" ${MODULE_NAME}_Global )
