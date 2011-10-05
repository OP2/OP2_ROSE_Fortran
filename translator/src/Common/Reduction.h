/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models a reduction
 */

#pragma once
#ifndef REDUCTION_H
#define REDUCTION_H

#include <string>

class SgType;

enum OPERATION
{
  INCREMENT, MINIMUM, MAXIMUM
};

class Reduction
{
  private:

    /*
     * ======================================================
     * The base type of the variable on which the reduction
     * operates
     * ======================================================
     */

    SgType * baseType;

    /*
     * ======================================================
     * The size of the variable on which the reduction operates
     * ======================================================
     */

    unsigned int variableSize;

  public:

    SgType *
    getBaseType ();

    unsigned int
    getVariableSize () const;

    std::string
    getSubroutineName () const;

    unsigned int
    hashKey ();

    bool
    isEquivalent (Reduction * reduction);

    Reduction (SgType * baseType, unsigned int variableSize);
};

#endif
