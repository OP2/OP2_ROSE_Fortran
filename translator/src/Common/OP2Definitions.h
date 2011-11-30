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

    SgType * primitiveType;

  public:

    std::string const &
    getOpSetName () const;

    unsigned int
    getDimension () const;

    SgType *
    getPrimitiveType ();
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

    SgType * primitiveType;

  public:

    unsigned int
    getDimension () const;

    SgType *
    getPrimitiveType ();
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

    void
    setType (SgType * baseType);

    SgType *
    getType ();

    unsigned int
    getDimension () const;
};

#endif
