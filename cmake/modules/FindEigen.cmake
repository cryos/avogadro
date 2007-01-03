# - Try to find eigen lib
# Once done this will define
#
#  EIGEN_FOUND - system has eigen lib
#  EIGEN_INCLUDE_DIR - the eigen include directory
#  EIGEN_LIBRARIES - Link these to use eigen lib
#
# Copyright (c) 2006, Montel Laurent, <montel@kde.org>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.



find_path(EIGEN_LIBRARIES NAMES libeigen.a
             PATHS
	     ${LIB_INSTALL_DIR}/
             /usr/lib/
             /usr/local/lib/
   )
message(STATUS "EIGEN_LIBRARIES :<${EIGEN_LIBRARIES}>")
find_path(EIGEN_INCLUDE_DIR NAMES eigen/matrix.h
     PATHS
     ${INCLUDE_INSTALL_DIR}
     /usr/include
     /usr/local/include
   )

   message(STATUS "EIGEN_INCLUDE_DIR <${EIGEN_INCLUDE_DIR}>")
if(EIGEN_INCLUDE_DIR AND EIGEN_LIBRARIES)
  set(EIGEN_FOUND TRUE)
endif(EIGEN_INCLUDE_DIR AND EIGEN_LIBRARIES)

if(EIGEN_FOUND)
   if(NOT eigen_FIND_QUIETLY)
      message(STATUS "Found eigen: ${EIGEN_LIBRARIES}")
   endif(NOT eigen_FIND_QUIETLY)
else(EIGEN_FOUND)
   if(eigen_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find eigen")
   endif(eigen_FIND_REQUIRED)
endif(EIGEN_FOUND)

mark_as_advanced(EIGEN_INCLUDE_DIR EIGEN_LIBRARIES )
 
