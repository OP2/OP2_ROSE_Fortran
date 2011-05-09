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

class OP_DAT_Declaration: public OP2Declaration
{
    /*
     * ======================================================
     * Models an OP_DAT declaration.
     *
     * The following style in Fortran is assumed:
     * OP_DECL_DAT (OP_SET, setCardinality, inputData, OP_DAT)
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

    OP_DAT_Declaration (SgExpressionPtrList & parameters);

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

class OP_SET_Declaration: public OP2Declaration
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

    OP_SET_Declaration (SgExpressionPtrList & parameters);
};

class OP_MAP_Declaration: public OP2Declaration
{
    /*
     * ======================================================
     * Models an OP_MAP declaration.
     *
     * The following style in Fortran is assumed:
     * OP_DECL_MAP (source_OP_SET, destination_OP_SET, setCardinality, inputData, OP_MAP)
     * ======================================================
     */

  private:

    static int const index_Source_OP_SET = 0;

    static int const index_Destination_OP_SET = 1;

    static int const index_setCardinality = 2;

    static int const index_inputData = 3;

    static int const index_OP_MAP = 4;

  public:

    OP_MAP_Declaration (SgExpressionPtrList & parameters);
};

class OP_GBL_Declaration: public OP2Declaration
{
    /*
     * ======================================================
     * Models an OP_GBL declaration.
     *
     * The following style in Fortran is assumed:
     * OP_DECL_GBL (inputData, dimension, OP_DAT)
     * ======================================================
     */

  private:

    static int const index_inputData = 0;

    static int const index_dimension = 1;

    static int const index_OP_DAT = 2;

    int dimension;

    SgType * actualType;

  public:

    OP_GBL_Declaration (SgExpressionPtrList & parameters);

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
