/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef CPP_OP2_DEFINITIONS_H
#define CPP_OP2_DEFINITIONS_H

#include <OP2Definitions.h>

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

  public:

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

  public:

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

  public:

    static int const index_sourceOpSetName = 0;

    static int const index_destinationOpSetName = 1;

    static int const index_setCardinality = 2;

    static int const index_mappingName = 3;

  public:

    CPPImperialOpMapDefinition (SgExpressionPtrList & parameters,
        std::string const & variableName);
};

class CPPImperialOpConstDefinition: public OpConstDefinition
{
    /*
     * ======================================================
     * Models a constant definition in C++ (Imperial API).
     *
     * The following style is assumed:
     * OP_DECL_CONST (dimension, OpDatName)
     * ======================================================
     */

  public:

    static int const index_dimension = 0;

    static int const index_OpDatName = 1;

  public:

    CPPImperialOpConstDefinition (SgExpressionPtrList & parameters);
};

class CPPImperialOpArgDatCall
{
  public:

    static int const index_OpDatName = 0;

    static int const index_OpIndex = 1;

    static int const index_OpMapName = 2;

    static int const index_OpAccess = 3;
};

class CPPImperialOpArgGblCall
{
  public:

    static int const index_OpDatName = 0;

    static int const index_OpAccess = 1;
};

#endif
