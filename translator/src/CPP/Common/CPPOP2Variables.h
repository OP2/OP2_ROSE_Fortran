/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef CPP_OP2_VARIABLES_H
#define CPP_OP2_VARIABLES_H

#include <OP2Variables.h>

class CPPImperialOpDatDefinition: public OpDatDefinition
{
    /*
     * ======================================================
     * Models an OP_DAT definition in C++ (Imperial API).
     *
     * The following style is assumed:
     * OP_DECL_DAT (OpSetName, setCardinality, data)
     * ======================================================
     */

  private:

    static int const index_OpSetName = 0;

    static int const index_dimension = 1;

    static int const index_data = 2;

  public:

    CPPImperialOpDatDefinition (SgExpressionPtrList & parameters,
        std::string const & variableName);
};

class CPPImperialOpSetDefinition: public OpSetDefinition
{
    /*
     * ======================================================
     * Models an OP_SET definition in C++ (Imperial API).
     *
     * The following style is assumed:
     * OP_DECL_SET (setCardinalityName)
     * ======================================================
     */

  private:

    static int const index_setCardinalityName = 0;

  public:

    CPPImperialOpSetDefinition (SgExpressionPtrList & parameters,
        std::string const & variableName);
};

class CPPImperialOpMapDefinition: public OpMapDefinition
{
    /*
     * ======================================================
     * Models an OP_MAP definition in C++ (Imperial API).
     *
     * The following style is assumed:
     * OP_DECL_MAP (source_OpSetName, destination_OpSetName, setCardinality, mappingName)
     * ======================================================
     */

  private:

    static int const index_sourceOpSetName = 0;

    static int const index_destinationOpSetName = 1;

    static int const index_setCardinality = 2;

    static int const index_mappingName = 3;

  public:

    CPPImperialOpMapDefinition (SgExpressionPtrList & parameters,
        std::string const & variableName);
};

class CPPImperialOpConstDefinition: public OpGblDefinition
{
    /*
     * ======================================================
     * Models a constant definition in C++ (Imperial API).
     *
     * The following style is assumed:
     * OP_DECL_CONST (dimension, data)
     * ======================================================
     */

  private:

    static int const index_dimension = 0;

    static int const index_data = 1;

  public:

    CPPImperialOpConstDefinition (SgExpressionPtrList & parameters,
        std::string const & variableName);
};

#endif
