#ifndef CPP_TYPES_BUILDER_H
#define CPP_TYPES_BUILDER_H


#include <vector>
#include <string>

class SgTypeBool;
class SgTypeInt;
class SgTypeFloat;
class SgArrayType;
class SgScopeStatement;
class SgClassDeclaration;
class SgClassDefinition;
class SgType;
class SgExpression;
class SgFunctionSymbol;
class SgModuleStatement;
class SgTypeString;

class CPPTypesBuilder
{
  private:

    /*
     * ======================================================
     * A helper function to initiate the construction of a
     * function by setting up ROSE-specific information
     * ======================================================
     */
    static SgFunctionSymbol *
    buildFunctionDeclaration (std::string const & functionName,
        SgScopeStatement * scope);

  public:
  
    /*
     * ======================================================
     * Creates a new CPP subroutine in the given scope with
     * the given name
     * ======================================================
     */

    static SgFunctionSymbol *
    buildNewCPPSubroutine (std::string const & functionName,
        SgScopeStatement * scope);
};

#endif

