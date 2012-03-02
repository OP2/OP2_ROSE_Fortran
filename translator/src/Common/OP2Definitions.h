


/*  Open source copyright declaration based on BSD open source template:
 *  http://www.opensource.org/licenses/bsd-license.php
 * 
 * Copyright (c) 2011-2012, Adam Betts, Carlo Bertolli
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#pragma once
#ifndef OP2_DEFINITIONS_H
#define OP2_DEFINITIONS_H

#include <string>

class SgType;

class OP2Definition
{
    /*
     * ======================================================
     * Base class to model an OP2 variable definition
     * ======================================================
     */

  protected:

    std::string variableName;

    OP2Definition ();

    virtual
    ~OP2Definition ();

  public:

    std::string const &
    getVariableName () const;
};

class OpDatDefinition: public OP2Definition
{
    /*
     * ======================================================
     * Models an OP_DAT variable definition
     * ======================================================
     */

  protected:

    std::string opSetName;

    unsigned int dimension;

    SgType * baseType;

  public:

    std::string const &
    getOpSetName () const;

    unsigned int
    getDimension () const;

    SgType *
    getBaseType ();
};

class OpSetDefinition: public OP2Definition
{
    /*
     * ======================================================
     * Models an OP_SET variable definition
     * ======================================================
     */

  protected:

    std::string dimensionName;

  public:

    std::string const &
    getDimensionName () const;
    
    bool
    isOpSubSet () { return false; }
};

class OpSubSetDefinition: public OpSetDefinition
{
    /*
     * ======================================================
     * Models an OP_SUBSET variable definition
     * ======================================================
     */

  protected:
    std::string originSetName;
    std::string filterKernelName;
    int nbFilterArg;
    
  public:

    bool
    isOpSubSet () { return true; }
    
    std::string const &
    getOriginSetName () const;
    
    std::string const &
    getFilterKernelName () const;
    
    int
    getNbFilterArg ();
};

class OpSparsityDefinition: public OP2Definition
{
    /*
     * ======================================================
     * Models an OP_SPARSITY variable definition
     * ======================================================
     */

  protected:

    std::string map1Name;

    std::string map2Name;

  public:

    std::string const &
    getSetName () const;

    std::string const &
    getMap1Name () const;

    std::string const &
    getMap2Name () const;
};

class OpMapDefinition: public OP2Definition
{
    /*
     * ======================================================
     * Models an OP_MAP variable definition
     * ======================================================
     */

  protected:

    std::string sourceOpSetName;

    std::string destinationOpSetName;

    unsigned int dimension;

    std::string mappingName;

  public:

    std::string const &
    getSourceOpSetName () const;

    std::string const &
    getDestinationOpSetName () const;

    unsigned int
    getDimension () const;

    std::string const &
    getMappingName () const;
};

class OpMatDefinition: public OP2Definition
{
    /*
     * ======================================================
     * Models an OP_MAT variable definition
     * ======================================================
     */

  protected:

    std::string sparsityName;

  public:

    std::string const &
    getSparsityName () const;
};

class OpGblDefinition: public OP2Definition
{
    /*
     * ======================================================
     * Models an OP_GBL variable definition
     * ======================================================
     */

  protected:

    unsigned int dimension;

    SgType * baseType;

  public:

    unsigned int
    getDimension () const;

    SgType *
    getBaseType ();
};

class OpConstDefinition: public OP2Definition
{
    /*
     * ======================================================
     * Models an OP_CONST variable definition
     * ======================================================
     */

  protected:

    unsigned int dimension;

    SgType * baseType;

  public:

    SgType *
    getType ();

    unsigned int
    getDimension () const;
};

class OpArgMatDefinition: public OP2Definition
{

};

#endif
