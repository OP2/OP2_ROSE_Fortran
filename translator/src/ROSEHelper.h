/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Helper functions to build ROSE expressions and statements
 */

#ifndef ROSE_HELPER_H
#define ROSE_HELPER_H

#include <rose.h>

class ROSEHelper
{
  private:

    static Sg_File_Info * fileInfo;

  public:

    /*
     * ======================================================
     * An Sg_File_Info object is often needed when building
     * ROSE expressions. Rather than build a fresh one for
     * each such expression, this returns a general one to
     * be used globally
     * ======================================================
     */
    static Sg_File_Info *
    getFileInfo ();

    /*
     * ======================================================
     * Returns the name of the first variable in this
     * variable declaration. Assumes the declaration does NOT
     * contain multiple variables
     * ======================================================
     */
    static std::string const
    getFirstVariableName (SgVariableDeclaration * variableDeclaration);
};

#endif
