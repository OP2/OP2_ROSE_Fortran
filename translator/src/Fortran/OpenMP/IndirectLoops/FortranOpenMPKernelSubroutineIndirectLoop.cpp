#include <FortranOpenMPKernelSubroutineIndirectLoop.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <CommonNamespaces.h>
#include <ROSEHelper.h>
#include <Debug.h>
#include <Plan.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
FortranOpenMPKernelSubroutineIndirectLoop::createUserSubroutineCallStatement ()
{

}

void
FortranOpenMPKernelSubroutineIndirectLoop::createExecutionLoopStatements ()
{

}

void
FortranOpenMPKernelSubroutineIndirectLoop::createPlanFormalParameterDeclarations ()
{
  using SageBuilder::buildIntVal;
  using std::string;
  using std::vector;

  vector <string> fourByteIntegerArrayVariables;

  fourByteIntegerArrayVariables.push_back (PlanFunction::Fortran::pindSizes);

  fourByteIntegerArrayVariables.push_back (PlanFunction::Fortran::pindOffs);

  fourByteIntegerArrayVariables.push_back (PlanFunction::Fortran::pblkMap);

  fourByteIntegerArrayVariables.push_back (PlanFunction::Fortran::poffset);

  fourByteIntegerArrayVariables.push_back (PlanFunction::Fortran::pnelems);

  fourByteIntegerArrayVariables.push_back (PlanFunction::Fortran::pnthrcol);

  fourByteIntegerArrayVariables.push_back (PlanFunction::Fortran::pthrcol);

  for (vector <string>::const_iterator it =
      fourByteIntegerArrayVariables.begin (); it
      != fourByteIntegerArrayVariables.end (); ++it)
  {
    SgAsteriskShapeExp * upperBoundExpression = new SgAsteriskShapeExp (
        ROSEHelper::getFileInfo ());

    variableDeclarations->add (
        *it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            *it, FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
                upperBoundExpression), subroutineScope, formalParameters));
  }

  variableDeclarations->add (
      PlanFunction::Fortran::blockOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          PlanFunction::Fortran::blockOffset,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters));
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createOpDatFormalParameterDeclarations ()
{
  using SageBuilder::buildIntVal;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgAsteriskShapeExp * upperBoundExpression1 = new SgAsteriskShapeExp (
          ROSEHelper::getFileInfo ());

      SgType * opdatType = parallelLoop->getOpDatType (i);

      SgArrayType * baseArrayType = isSgArrayType (opdatType);

      variableDeclarations->add (
          VariableNames::getOpDatName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              VariableNames::getOpDatName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  baseArrayType->get_base_type (), buildIntVal (0),
                  upperBoundExpression1), subroutineScope, formalParameters));

      SgAsteriskShapeExp * upperBoundExpression2 = new SgAsteriskShapeExp (
          ROSEHelper::getFileInfo ());

      variableDeclarations->add (
          VariableNames::getLocalToGlobalMappingName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              VariableNames::getLocalToGlobalMappingName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
                  upperBoundExpression2), subroutineScope, formalParameters));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgAsteriskShapeExp * upperBoundExpression = new SgAsteriskShapeExp (
          ROSEHelper::getFileInfo ());

      variableDeclarations->add (
          VariableNames::getGlobalToLocalMappingName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              VariableNames::getGlobalToLocalMappingName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  FortranTypesBuilder::getTwoByteInteger (), buildIntVal (0),
                  upperBoundExpression), subroutineScope, formalParameters));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == DIRECT)
    {
      SgAsteriskShapeExp * upperBoundExpression = new SgAsteriskShapeExp (
          ROSEHelper::getFileInfo ());

      SgType * opdatType = parallelLoop->getOpDatType (i);

      SgArrayType * baseArrayType = isSgArrayType (opdatType);

      variableDeclarations->add (
          VariableNames::getOpDatName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              VariableNames::getOpDatName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  baseArrayType->get_base_type (), buildIntVal (0),
                  upperBoundExpression), subroutineScope, formalParameters));
    }
  }
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createStatements ()
{
  createExecutionLoopStatements ();
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  using std::string;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const & variableName =
          VariableNames::getNumberOfBytesVariableName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const & variableName =
          VariableNames::getNumberOfBytesVariableName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const & variableName = VariableNames::getRoundUpVariableName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
FortranOpenMPKernelSubroutineIndirectLoop::createFormalParameterDeclarations ()
{
  variableDeclarations->add (
      OpenMP::blockID,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OpenMP::blockID, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, formalParameters));

  createOpDatFormalParameterDeclarations ();

  createPlanFormalParameterDeclarations ();
}

FortranOpenMPKernelSubroutineIndirectLoop::FortranOpenMPKernelSubroutineIndirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranOpenMPKernelSubroutine (subroutineName, userSubroutineName,
      parallelLoop, moduleScope)
{
  Debug::getInstance ()->debugMessage ("<Kernel, Indirect, OpenMP>",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
