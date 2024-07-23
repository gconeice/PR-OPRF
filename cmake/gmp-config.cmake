# Try to find the GMP librairies:
# GMP_FOUND - System has GMP lib
# GMP_INCLUDE_DIR - The GMP include directory
# GMP_LIBRARIES - Libraries needed to use GMP
# GMPXX_INCLUDE_DIR - The GMP C++ interface include directory
# GMPXX_LIBRARIES - Libraries needed to use GMP's C++ interface

IF(GMP_INCLUDE_DIR AND GMP_LIBRARIES AND GMPXX_INCLUDE_DIR AND GMPXX_LIBRARIES)
	# Already in cache, be silent
	SET(GMP_FIND_QUIETLY TRUE)
ENDIF(GMP_INCLUDE_DIR AND GMP_LIBRARIES AND GMPXX_INCLUDE_DIR AND GMPXX_LIBRARIES)

FIND_PATH(GMP_INCLUDE_DIR NAMES gmp.h)
FIND_LIBRARY(GMP_LIBRARIES NAMES gmp)
FIND_PATH(GMPXX_INCLUDE_DIR NAMES gmpxx.h)
FIND_LIBRARY(GMPXX_LIBRARIES NAMES gmpxx)

IF(GMP_INCLUDE_DIR AND GMP_LIBRARIES AND GMPXX_INCLUDE_DIR AND GMPXX_LIBRARIES)
	SET(GMP_FOUND TRUE)
ENDIF(GMP_INCLUDE_DIR AND GMP_LIBRARIES AND GMPXX_INCLUDE_DIR AND GMPXX_LIBRARIES)

IF(GMP_FOUND)
	IF(NOT GMP_FIND_QUIETLY)
		MESSAGE(STATUS "Found GMP: ${GMP_LIBRARIES}, ${GMPXX_LIBRARIES}")
	ENDIF(NOT GMP_FIND_QUIETLY)
ELSE(GMP_FOUND)
	IF(GMP_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could NOT find GMP")
	ENDIF(GMP_FIND_REQUIRED)
ENDIF(GMP_FOUND)

MARK_AS_ADVANCED(GMP_INCLUDE_DIR GMP_LIBRARIES GMPXX_INCLUDE_DIR GMPXX_LIBRARIES)