#include <FortranCUDAInitialiseConstantsSubroutine.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <RoseHelper.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranCUDAInitialiseConstantsSubroutine::declareConstants ()
{
  using std::map;
  using std::string;

  string const separator = "_";

  for (map <string, OpConstDefinition *>::const_iterator it =
      declarations->firstOpConstDefinition (); it
      != declarations->lastOpConstDefinition (); ++it)
  {
    string const variableName = it->first;

    string const moduleVariableName = variableNamePrefix + separator
        + variableName;

    constantVariableNames[variableName] = moduleVariableName;

    if (declarations->isTypeBoolean (variableName))
    {
      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              moduleVariableName, FortranTypesBuilder::getLogical (),
              moduleScope, 1, CONSTANT);

      variableDeclarations->add (moduleVariableName, variableDeclaration);
    }
    else if (declarations->isTypeShort (variableName))
    {
      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              moduleVariableName, FortranTypesBuilder::getTwoByteInteger (),
              moduleScope, 1, CONSTANT);

      variableDeclarations->add (moduleVariableName, variableDeclaration);
    }
    else if (declarations->isTypeInteger (variableName))
    {
      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              moduleVariableName, FortranTypesBuilder::getFourByteInteger (),
              moduleScope, 1, CONSTANT);

      variableDeclarations->add (moduleVariableName, variableDeclaration);
    }
    else if (declarations->isTypeLong (variableName))
    {
      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              moduleVariableName, FortranTypesBuilder::getEightByteInteger (),
              moduleScope, 1, CONSTANT);

      variableDeclarations->add (moduleVariableName, variableDeclaration);
    }
    else if (declarations->isTypeFloat (variableName))
    {
      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              moduleVariableName,
              FortranTypesBuilder::getSinglePrecisionFloat (), moduleScope, 1,
              CONSTANT);

      variableDeclarations->add (moduleVariableName, variableDeclaration);
    }
    else if (declarations->isTypeDouble (variableName))
    {
      SgVariableDeclaration * variableDeclaration =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              moduleVariableName,
              FortranTypesBuilder::getDoublePrecisionFloat (), moduleScope, 1,
              CONSTANT);

      variableDeclarations->add (moduleVariableName, variableDeclaration);
    }
  }
}

void
FortranCUDAInitialiseConstantsSubroutine::createStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;
  using std::string;
  using std::map;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating statements in initialise constants subroutine",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (map <string, OpConstDefinition *>::const_iterator it =
      declarations->firstOpConstDefinition (); it
      != declarations->lastOpConstDefinition (); ++it)
  {
    string const variableName = it->first;

    Debug::getInstance ()->debugMessage ("Analysing constant '" + variableName
        + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    string const moduleVariableName = constantVariableNames[variableName];

    vector <SgExpression *> initializers = declarations->getInitializers (
        variableName);

    ROSE_ASSERT (initializers.empty() == false);

    if (initializers.size () > 1)
    {
      Debug::getInstance ()->errorMessage (
          "Too many initializers found for the OP_CONST '" + variableName + "'");
    }
    else
    {
      SgExpression * initializer = *(initializers.begin ());

      SgExprStatement * assignmentStatement = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (moduleVariableName)),
          initializer);

      appendStatement (assignmentStatement, subroutineScope);
    }
  }
}

void
FortranCUDAInitialiseConstantsSubroutine::createLocalVariableDeclarations ()
{
}

void
FortranCUDAInitialiseConstantsSubroutine::createFormalParameterDeclarations ()
{
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
FortranCUDAInitialiseConstantsSubroutine::generateSubroutine ()
{
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildVoidType;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(host) ",
      AstUnparseAttribute::e_before);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createStatements ();
}

std::map <std::string, std::string>::const_iterator
FortranCUDAInitialiseConstantsSubroutine::getFirstConstantName ()
{
  return constantVariableNames.begin ();
}

std::map <std::string, std::string>::const_iterator
FortranCUDAInitialiseConstantsSubroutine::getLastConstantName ()
{
  return constantVariableNames.end ();
}

FortranCUDAInitialiseConstantsSubroutine::FortranCUDAInitialiseConstantsSubroutine (
    std::string const & subroutineName, SgScopeStatement * moduleScope,
    FortranProgramDeclarationsAndDefinitions * declarations) :
  Subroutine <SgProcedureHeaderStatement> (subroutineName
      + "_initialiseConstants"), variableNamePrefix (subroutineName),
      moduleScope (moduleScope), declarations (declarations)
{
  Debug::getInstance ()->debugMessage (
      "Creating initialise constants subroutine", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  declareConstants ();
}
