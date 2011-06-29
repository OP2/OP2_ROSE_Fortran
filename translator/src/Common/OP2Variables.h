/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * These classes model OP2 variable declarations
 */

#ifndef OP2VARIABLES_H
#define OP2VARIABLES_H

#include <rose.h>

class OP2Declaration
{
    /*
     * ======================================================
     * Base class to model an OP2 variable declaration
     * ======================================================
     */

  protected:

    std::string variableName;

    OP2Declaration ()
    {
    }

  public:

    std::string const &
    getVariableName () const
    {
      return variableName;
    }
};

class OpDatDeclaration: public OP2Declaration
{
    /*
     * ======================================================
     * Models an OP_DAT declaration.
     *
     * The following style in Fortran is assumed:
     * OP_DECL_DAT (OpSet, setCardinality, inputData, OpDat)
     * ======================================================
     */

  private:

    static int const index_OP_SET = 0;

    static int const index_dimension = 1;

    static int const index_inputData = 2;

    static int const index_OP_DAT = 3;

    int dimension;

    SgType * actualType;

  public:

    OpDatDeclaration (SgExpressionPtrList & parameters);

    int
    getDimension ()
    {
      return dimension;
    }

    SgType *
    getActualType ()
    {
      return actualType;
    }
};

class OpSetDeclaration: public OP2Declaration
{
    /*
     * ======================================================
     * Models an OP_SET declaration.
     *
     * The following style in Fortran is assumed:
     * OP_DECL_SET (setCardinality, OP_SET)
     * ======================================================
     */

  private:

    static int const index_setCardinality = 0;

    static int const index_OP_SET = 1;

  public:

    OpSetDeclaration (SgExpressionPtrList & parameters);
};

class OpMapDeclaration: public OP2Declaration
{
    /*
     * ======================================================
     * Models an OP_MAP declaration.
     *
     * The following style in Fortran is assumed:
     * OP_DECL_MAP (source_OpSet, destination_OpSet, setCardinality, inputData, OpMap)
     * ======================================================
     */

  private:

    static int const index_Source_OP_SET = 0;

    static int const index_Destination_OP_SET = 1;

    static int const index_setCardinality = 2;

    static int const index_inputData = 3;

    static int const index_OP_MAP = 4;

  public:

    OpMapDeclaration (SgExpressionPtrList & parameters);
};

class OpGblDeclaration: public OP2Declaration
{
    /*
     * ======================================================
     * Models an OP_GBL declaration.
     *
     * The following style in Fortran is assumed:
     * OP_DECL_GBL (inputData, dimension, OpDat)
     * ======================================================
     */

  private:

    static int const index_inputData = 0;

    static int const index_dimension = 1;

    static int const index_OP_DAT = 2;

    int dimension;

    SgType * actualType;

  public:

    OpGblDeclaration (SgExpressionPtrList & parameters);

    int
    getDimension ()
    {
      return dimension;
    }

    SgType *
    getActualType ()
    {
      return actualType;
    }
};

#endif
