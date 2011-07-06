#ifndef FORTRAN_OP2_DEFINITIONS_H
#define FORTRAN_OP2_DEFINITIONS_H

#include <OP2Definitions.h>

class FortranOpDatDefinition: public OpDatDefinition
{
    /*
     * ======================================================
     * Models an OP_DAT definition in Fortran.
     *
     * The following style is assumed:
     * OP_DECL_DAT (OpSetName, setCardinality, data, OpDatName)
     * ======================================================
     */

  private:

    static int const index_OpSetName = 0;

    static int const index_dimension = 1;

    static int const index_data = 2;

    static int const index_OpDatName = 3;

  public:

    FortranOpDatDefinition (SgExpressionPtrList & parameters);
};

class FortranOpSetDefinition: public OpSetDefinition
{
    /*
     * ======================================================
     * Models an OP_SET definition in Fortran.
     *
     * The following style is assumed:
     * OP_DECL_SET (setCardinalityName, OpSetName)
     * ======================================================
     */

  private:

    static int const index_setCardinalityName = 0;

    static int const index_OpSetName = 1;

  public:

    FortranOpSetDefinition (SgExpressionPtrList & parameters);
};

class FortranOpMapDefinition: public OpMapDefinition
{
    /*
     * ======================================================
     * Models an OP_MAP definition in Fortran.
     *
     * The following style is assumed:
     * OP_DECL_MAP (source_OpSetName, destination_OpSetName, dimension, mappingCardinalityName, mappingName, OpMapName)
     * ======================================================
     */

  protected:

    std::string mappingCardinalityName;

  private:

    static int const index_sourceOpSetName = 0;

    static int const index_destinationOpSetName = 1;

    static int const index_dimension = 2;

    static int const index_mappingCardinalityName = 3;

    static int const index_mappingName = 4;

    static int const index_OpMapName = 5;

  public:

    std::string const &
    getMappingCardinalityName () const
    {
      return mappingCardinalityName;
    }

    FortranOpMapDefinition (SgExpressionPtrList & parameters);
};

class FortranOpGblDefinition: public OpGblDefinition
{
    /*
     * ======================================================
     * Models an OP_GBL definition in Fortran.
     *
     * The following style is assumed:
     * OP_DECL_GBL (data, dimension, OpDatName)
     * ======================================================
     */

  private:

    static int const index_data = 0;

    static int const index_dimension = 1;

    static int const index_OpDatName = 2;

  public:

    FortranOpGblDefinition (SgExpressionPtrList & parameters);
};

class FortranOpConstDefinition: public OpConstDefinition
{
    /*
     * ======================================================
     * Models an OP_CONST definition in Fortran.
     *
     * The following style is assumed:
     * OP_DECL_CONST (dimension, OpConstName)
     * ======================================================
     */

  private:

    static int const index_dimension = 0;

    static int const index_OpConstName = 1;

  public:

    FortranOpConstDefinition (SgExpressionPtrList & parameters);
};

#endif
