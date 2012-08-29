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
# Some common setup
# ---------------------------------------------------------------------------------------------------------------------------------------------------

# we use the directory name as module name
GET_FILENAME_COMPONENT( MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME )

# setup the target directories and names
SET( MODULE_TARGET_DIR_RELATIVE ${OW_MODULE_DIR_RELATIVE}/${MODULE_NAME} )
SET( MODULE_TARGET_DIR ${PROJECT_BINARY_DIR}/${MODULE_TARGET_DIR_RELATIVE} )
SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MODULE_TARGET_DIR} )
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MODULE_TARGET_DIR} )
SET( MODULE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

# -----------------------------------------------------------------------------------------------------------------------------------------------
# Add sources as target
# -----------------------------------------------------------------------------------------------------------------------------------------------

# Collect the compile-files for this target
COLLECT_COMPILE_FILES( "${MODULE_SOURCE_DIR}" TARGET_CPP_FILES TARGET_H_FILES TARGET_TEST_FILES )

# for cuda, we need to do some special stuff first
IF( CUDA_FOUND AND OW_USE_CUDA )
    FILE( GLOB_RECURSE CUDA_SRC "*.cu" )
    CUDA_ADD_LIBRARY( ${MODULE_NAME} SHARED ${TARGET_CPP_FILES} ${TARGET_H_FILES} ${CUDA_SRC} )
    TARGET_LINK_LIBRARIES( ${MODULE_NAME} ${OW_LIB_OPENWALNUT} ${CUDA_LIBRARIES} )
ELSE()
    # Remove files that do including cuda stuff
    LIST( REMOVE_ITEM TARGET_H_FILES ${CMAKE_CURRENT_SOURCE_DIR}/WCheckCudaError.h )
    LIST( REMOVE_ITEM TARGET_H_FILES ${CMAKE_CURRENT_SOURCE_DIR}/WMDetTractClusteringCudaKernel.h )
    LIST( REMOVE_ITEM TARGET_H_FILES ${CMAKE_CURRENT_SOURCE_DIR}/WMDetTractClusteringCudaInterface.h )
    LIST( REMOVE_ITEM TARGET_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/WCheckCudaError.cpp )
    LIST( REMOVE_ITEM TARGET_CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/WMDetTractClusteringCudaInterface.cpp )

    # Build the module without the cuda codes
    ADD_LIBRARY( ${MODULE_NAME} SHARED ${TARGET_CPP_FILES} ${TARGET_H_FILES} )
    TARGET_LINK_LIBRARIES( ${MODULE_NAME} ${OW_LIB_OPENWALNUT} )
ENDIF()

# Set the version of the library.
SET_TARGET_PROPERTIES( ${MODULE_NAME} PROPERTIES
    VERSION ${OW_LIB_VERSION}
    SOVERSION ${OW_SOVERSION}
)

# Do not forget the install targets
SETUP_LIB_INSTALL( ${MODULE_NAME} ${MODULE_TARGET_DIR_RELATIVE} "MODULES" )

# -----------------------------------------------------------------------------------------------------------------------------------------------
# Test Setup
# -----------------------------------------------------------------------------------------------------------------------------------------------

# Setup tests of this target
SETUP_TESTS( "${TARGET_TEST_FILES}" "${MODULE_NAME}" "${_MODULE_DEPENDENCIES}" )

# -----------------------------------------------------------------------------------------------------------------------------------------------
# Copy Shaders
# -----------------------------------------------------------------------------------------------------------------------------------------------

COLLECT_SHADER_FILES( ${MODULE_SOURCE_DIR} TARGET_GLSL_FILES )
SETUP_SHADERS( "${TARGET_GLSL_FILES}" "${MODULE_TARGET_DIR_RELATIVE}/shaders" "MODULES" )

# -----------------------------------------------------------------------------------------------------------------------------------------------
# Style Checker
# -----------------------------------------------------------------------------------------------------------------------------------------------

# setup the stylechecker. Ignore the platform specific stuff.
SETUP_STYLECHECKER( "${MODULE_NAME}"
                    "${TARGET_CPP_FILES};${TARGET_H_FILES};${TARGET_TEST_FILES};${TARGET_GLSL_FILES}"  # add all these files to the stylechecker
                    "${_MODULE_STYLE_EXCLUDES}" )                                                      # exlude some ugly files

