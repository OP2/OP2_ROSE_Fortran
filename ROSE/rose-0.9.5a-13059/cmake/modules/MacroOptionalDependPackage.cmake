# Search if cmake module is installed in computer
# cmake will not fail but signal that we must install depend package before.
# add as previously name of cmake module "_name" and define package needed "_module_needed"
# if return DEPEND_PACKAGE_${_name}

# Copyright (c) 2007, Montel Laurent <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


MACRO (MACRO_OPTIONAL_DEPEND_PACKAGE _name _module_needed)
   set(_packagename Find${_name}.cmake)
   find_file(_PACKAGE_DEPEND_FOUND ${_packagename} PATHS ${CMAKE_MODULE_PATH} )
   if(NOT _PACKAGE_DEPEND_FOUND)
	MESSAGE(STATUS "cmake package ${_packagename} was not found. This package needs ${_module_needed} to be compile all program")
	set(DEPEND_PACKAGE_${_name} FALSE)
   else(NOT _PACKAGE_DEPEND_FOUND)
   	set(DEPEND_PACKAGE_${_name} TRUE)	   
   endif(NOT _PACKAGE_DEPEND_FOUND)
ENDMACRO (MACRO_OPTIONAL_DEPEND_PACKAGE)

