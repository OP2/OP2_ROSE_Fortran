#include "FortranHostSubroutine.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include "FortranParallelLoop.h"
#include "OP2.h"
#include "CompilerGeneratedNames.h"
#include "Debug.h"
#include <boost/algorithm/string/predicate.hpp>
#include <rose.h>

void
FortranHostSubroutine::createFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2;
  using namespace OP2VariableNames;
  using boost::iequals;
  using std::vector;
  using std::string;
  using std::map;

  Debug::getInstance ()->debugMessage (
      "Creating host subroutine formal parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Add name of user kernel
   * ======================================================
   */

  string const & kernelVariableName = getUserSubroutineName ();

  variableDeclarations->add (
      kernelVariableName,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          kernelVariableName, FortranTypesBuilder::getString (
              parallelLoop->getUserSubroutineName ().length ()),
          subroutineScope, formalParameters, 1, INTENT_IN));

  /*
   * ======================================================
   * Add OP_SET
   * ======================================================
   */

  string const & opSetVariableName = getOpSetName ();

  variableDeclarations->add (
      opSetVariableName,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          opSetVariableName, FortranTypesBuilder::buildClassDeclaration (
              OP_SET, subroutineScope)->get_type (), subroutineScope,
          formalParameters, 1, INTENT_IN));

  /*
   * ======================================================
   * Add OP_DAT, indirection, OP_MAP, access arguments for
   * each OP_DAT argument group
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & opDatvariableName = getOpDatName (i);

    variableDeclarations->add (
        opDatvariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            opDatvariableName, FortranTypesBuilder::buildClassDeclaration (
                OP2::OP_DAT, subroutineScope)->get_type (), subroutineScope,
            formalParameters, 1, INTENT_IN));

    string const & indirectionVariableName = getOpIndirectionName (i);

    variableDeclarations->add (
        indirectionVariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            indirectionVariableName,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope,
            formalParameters, 1, INTENT_IN));

    string const & opMapVariableName = getOpMapName (i);

    variableDeclarations->add (
        opMapVariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            opMapVariableName, FortranTypesBuilder::buildClassDeclaration (
                OP2::OP_MAP, subroutineScope)->get_type (), subroutineScope,
            formalParameters, 1, INTENT_IN));

    string const & accessVariableName = getOpAccessName (i);

    variableDeclarations->add (
        accessVariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            accessVariableName, FortranTypesBuilder::getFourByteInteger (),
            subroutineScope, formalParameters, 1, INTENT_IN));
  }
}

FortranHostSubroutine::FortranHostSubroutine (SgScopeStatement * moduleScope,
    Subroutine <SgProcedureHeaderStatement> * calleeSubroutine,
    FortranParallelLoop * parallelLoop) :
  HostSubroutine <SgProcedureHeaderStatement> (calleeSubroutine, parallelLoop)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  appendStatement (subroutineHeaderStatement, moduleScope);

  appendStatement (
      FortranStatementsAndExpressionsBuilder::buildImplicitNoneStatement (),
      subroutineScope);
}
