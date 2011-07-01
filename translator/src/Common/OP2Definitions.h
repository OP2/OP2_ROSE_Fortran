/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef OP2_DEFINITIONS_H
#define OP2_DEFINITIONS_H

#include <rose.h>

class OP2Definition
{
    /*
     * ======================================================
     * Base class to model an OP2 variable definition
     * ======================================================
     */

  protected:

    std::string variableName;

    OP2Definition ()
    {
    }

  public:

    std::string const &
    getVariableName () const
    {
      return variableName;
    }
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

    SgType * actualType;

  public:

    std::string const &
    getOpSetName () const
    {
      return opSetName;
    }

    unsigned int
    getDimension () const
    {
      return dimension;
    }

    SgType *
    getActualType ()
    {
      return actualType;
    }
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
    getDimensionName () const
    {
      return dimensionName;
    }
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
    getSourceOpSetName () const
    {
      return sourceOpSetName;
    }

    std::string const &
    getDestinationOpSetName () const
    {
      return destinationOpSetName;
    }

    unsigned int
    getDimension () const
    {
      return dimension;
    }

    std::string const &
    getMappingName () const
    {
      return mappingName;
    }
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

    SgType * actualType;

  public:

    unsigned int
    getDimension () const
    {
      return dimension;
    }

    SgType *
    getActualType ()
    {
      return actualType;
    }
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

  public:

    unsigned int
    getDimension () const
    {
      return dimension;
    }
};

#endif
