#include <boost/algorithm/string/predicate.hpp>
#include <CommonNamespaces.h>
#include <Debug.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <FortranHostSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

void
FortranHostSubroutine::createFormalParameterDeclarations ()
{
  using boost::iequals;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntType;
  using SageInterface::appendStatement;
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

  string const & kernelVariableName =
      OP2::VariableNames::getUserSubroutineName ();

  variableDeclarations->add (
      kernelVariableName,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          kernelVariableName, FortranTypesBuilder::getString (
              userSubroutineName.length ()), subroutineScope, formalParameters,
          1, INTENT_IN));

  /*
   * ======================================================
   * Add OP_SET
   * ======================================================
   */

  string const & opSetVariableName = OP2::VariableNames::getOpSetName ();

  variableDeclarations->add (
      opSetVariableName,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          opSetVariableName, FortranTypesBuilder::buildClassDeclaration (
              OP2::OP_SET, subroutineScope)->get_type (), subroutineScope,
          formalParameters, 1, INTENT_IN));

  /*
   * ======================================================
   * Add OP_DAT, indirection, OP_MAP, access arguments for
   * each OP_DAT argument group
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & opDatvariableName = OP2::VariableNames::getOpDatName (i);

    variableDeclarations->add (
        opDatvariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            opDatvariableName, FortranTypesBuilder::buildClassDeclaration (
                OP2::OP_DAT, subroutineScope)->get_type (), subroutineScope,
            formalParameters, 1, INTENT_IN));

    string const & indirectionVariableName =
        OP2::VariableNames::getOpIndirectionName (i);

    variableDeclarations->add (
        indirectionVariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            indirectionVariableName,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope,
            formalParameters, 1, INTENT_IN));

    string const & opMapVariableName = OP2::VariableNames::getOpMapName (i);

    variableDeclarations->add (
        opMapVariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            opMapVariableName, FortranTypesBuilder::buildClassDeclaration (
                OP2::OP_MAP, subroutineScope)->get_type (), subroutineScope,
            formalParameters, 1, INTENT_IN));

    string const & accessVariableName = OP2::VariableNames::getOpAccessName (i);

    variableDeclarations->add (
        accessVariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            accessVariableName, FortranTypesBuilder::getFourByteInteger (),
            subroutineScope, formalParameters, 1, INTENT_IN));
  }
}
