# - Try to find GSL.
# Variables used by this module:
#  GSL_ROOT_DIR     - GSL root directory
# Variables defined by this module:
#  GSL_FOUND        - system has GSL
#  GSL_INCLUDE_DIR  - the GSL include directory (cached)
#  GSL_INCLUDE_DIRS - the GSL include directories
#                         (identical to GSL_INCLUDE_DIR)
#  GSL_LIBRARY      - the GSL library (cached)
#  GSL_LIBRARIES    - the GSL libraries
#                         (identical to GSL_LIBRARY)

#
# $Id$

if(NOT GSL_FOUND)

  find_path(GSL_INCLUDE_DIR gsl/gsl_math.h
    PATHS ${GSL_ROOT_DIR} PATH_SUFFIXES include include/gsl)
  find_library(GSL_LIBRARY gsl 
    PATHS ${GSL_ROOT_DIR} PATH_SUFFIXES lib)
  find_library(GSLCBLAS_LIBRARY gslcblas 
    PATHS ${GSL_ROOT_DIR} PATH_SUFFIXES lib)

  set(GSL_INCLUDE_DIRS ${GSL_INCLUDE_DIR})
  set(GSL_LIBRARIES ${GSL_LIBRARY} ${GSLCBLAS_LIBRARY})

endif(NOT GSL_FOUND)
