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

#endif
