# - Try to find LibXml2
# Once done this will define
#
#  LIBXML2_FOUND - System has LibXml2
#  LIBXML2_INCLUDE_DIR - The LibXml2 include directory
#  LIBXML2_LIBRARIES - The libraries needed to use LibXml2
#  LIBXML2_DEFINITIONS - Compiler switches required for using LibXml2
#  LIBXML2_XMLLINT_EXECUTABLE - The XML checking tool xmllint coming with LibXml2

# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (LIBXML2_INCLUDE_DIR AND LIBXML2_LIBRARIES)
   # in cache already
   SET(LibXml2_FIND_QUIETLY TRUE)
ENDIF (LIBXML2_INCLUDE_DIR AND LIBXML2_LIBRARIES)

IF (NOT WIN32)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   FIND_PACKAGE(PkgConfig)
   PKG_CHECK_MODULES(PC_LIBXML libxml-2.0)
   SET(LIBXML2_DEFINITIONS ${PC_LIBXML_CFLAGS_OTHER})
ENDIF (NOT WIN32)

FIND_PATH(LIBXML2_INCLUDE_DIR libxml/xpath.h
   HINTS
   ${PC_LIBXML_INCLUDEDIR}
   ${PC_LIBXML_INCLUDE_DIRS}
   PATH_SUFFIXES libxml2
   )

FIND_LIBRARY(LIBXML2_LIBRARIES NAMES xml2 libxml2
   HINTS
   ${PC_LIBXML_LIBDIR}
   ${PC_LIBXML_LIBRARY_DIRS}
   )

FIND_PROGRAM(LIBXML2_XMLLINT_EXECUTABLE xmllint)
# for backwards compat. with KDE 4.0.x:
SET(XMLLINT_EXECUTABLE "${LIBXML2_XMLLINT_EXECUTABLE}")

IF( NOT LIBXML2_XMLLINT_EXECUTABLE )
   MESSAGE(STATUS "xmllint program not found. Install it if you want validate generated doc file.")
ENDIF(NOT LIBXML2_XMLLINT_EXECUTABLE )


INCLUDE(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LIBXML2_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibXml2 DEFAULT_MSG LIBXML2_LIBRARIES LIBXML2_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBXML2_INCLUDE_DIR LIBXML2_LIBRARIES LIBXML2_XMLLINT_EXECUTABLE)

