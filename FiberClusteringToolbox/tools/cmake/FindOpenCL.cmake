# This script searches cl.h and cl.hpp (the C++ binding header). Grab these headers form http://www.khronos.org/registry/cl.
#
# The following variables will be filled:
#   * OPENCL_FOUND - if cl.h, cl.hpp and OpenCL lib was found
#   * OPENCL_INCLUDE_DIR - the path of cl.h if found
#   * OPENCL_CPP_INCLUDE_DIR - the path of cl.hpp if found
#   * OPENCL_LIBRARY - the path to the OpenCL library
#

FIND_PATH( OPENCL_INCLUDE_DIR CL/cl.h /usr/include /usr/local/include )
FIND_PATH( OPENCL_CPP_INCLUDE_DIR CL/cl.hpp /usr/include /usr/local/include )
FIND_LIBRARY( OPENCL_LIBRARY NAMES OpenCL PATH /usr/lib /usr/local/lib /usr/local/cuda/lib64 /usr/local/cuda/lib )

message( STATUS ${OPENCL_CPP_INCLUDE_DIR} )

SET( OPENCL_FOUND FALSE )
IF ( OPENCL_INCLUDE_DIR AND OPENCL_CPP_INCLUDE_DIR AND OPENCL_LIBRARY )
    SET( OPENCL_FOUND TRUE )
ENDIF ( OPENCL_INCLUDE_DIR AND OPENCL_CPP_INCLUDE_DIR AND OPENCL_LIBRARY )

IF ( OPENCL_FOUND )
   IF ( NOT OpenCL_FIND_QUIETLY )
       MESSAGE( STATUS "Found OpenCL: ${OPENCL_LIBRARY} and include in ${OPENCL_INCLUDE_DIR}" )
   ENDIF()
ELSE ( OPENCL_FOUND )
   IF (OpenCL_FIND_REQUIRED)
      MESSAGE( FATAL_ERROR "Could not find OpenCL. Install an up-to-date graphics driver and grab the OpenCL headers from http://www.khronos.org/registry/cl." )
   ENDIF()
ENDIF ( OPENCL_FOUND )

