#include <FortranOpenMPKernelSubroutineIndirectLoop.h>
#include <FortranParallelLoop.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <CompilerGeneratedNames.h>
#include <RoseHelper.h>
#include <Debug.h>
#include <OpenMP.h>

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
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;
  using std::vector;

  vector <string> fourByteIntegerArrayVariables;

  fourByteIntegerArrayVariables.push_back (pindSizes);
  fourByteIntegerArrayVariables.push_back (pindOffs);
  fourByteIntegerArrayVariables.push_back (pblkMap);
  fourByteIntegerArrayVariables.push_back (poffset);
  fourByteIntegerArrayVariables.push_back (pnelems);
  fourByteIntegerArrayVariables.push_back (pnthrcol);
  fourByteIntegerArrayVariables.push_back (pthrcol);

  for (vector <string>::const_iterator it =
      fourByteIntegerArrayVariables.begin (); it
      != fourByteIntegerArrayVariables.end (); ++it)
  {
    SgAsteriskShapeExp * upperBoundExpression = new SgAsteriskShapeExp (
        RoseHelper::getFileInfo ());

    variableDeclarations->add (
        *it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            *it, FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                FortranTypesBuilder::getFourByteInteger (), buildIntVal (0),
                upperBoundExpression), subroutineScope, formalParameters));
  }

  variableDeclarations->add (
      blockOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          blockOffset, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, formalParameters));
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createOpDatFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgAsteriskShapeExp * upperBoundExpression1 = new SgAsteriskShapeExp (
            RoseHelper::getFileInfo ());

        variableDeclarations->add (
            getOpDatName (i),
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                getOpDatName (i),
                FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                    parallelLoop->getOpDatBaseType (i), buildIntVal (0),
                    upperBoundExpression1), subroutineScope, formalParameters));

        SgAsteriskShapeExp * upperBoundExpression2 = new SgAsteriskShapeExp (
            RoseHelper::getFileInfo ());

        variableDeclarations->add (
            getLocalToGlobalMappingName (i),
            FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                getLocalToGlobalMappingName (i),
                FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                    FortranTypesBuilder::getFourByteInteger (),
                    buildIntVal (0), upperBoundExpression2), subroutineScope,
                formalParameters));
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      SgAsteriskShapeExp * upperBoundExpression = new SgAsteriskShapeExp (
          RoseHelper::getFileInfo ());

      variableDeclarations->add (
          getGlobalToLocalMappingName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              getGlobalToLocalMappingName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  FortranTypesBuilder::getTwoByteInteger (), buildIntVal (0),
                  upperBoundExpression), subroutineScope, formalParameters));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false && parallelLoop->isDirect (
        i))
    {
      SgAsteriskShapeExp * upperBoundExpression = new SgAsteriskShapeExp (
          RoseHelper::getFileInfo ());

      variableDeclarations->add (
          getOpDatName (i),
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              getOpDatName (i),
              FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
                  parallelLoop->getOpDatBaseType (i), buildIntVal (0),
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
}

void
FortranOpenMPKernelSubroutineIndirectLoop::createFormalParameterDeclarations ()
{
  using namespace PlanFunctionVariableNames;

  variableDeclarations->add (
      blockID,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          blockID, FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters));

  createOpDatFormalParameterDeclarations ();

  createPlanFormalParameterDeclarations ();
}

FortranOpenMPKernelSubroutineIndirectLoop::FortranOpenMPKernelSubroutineIndirectLoop (
    SgScopeStatement * moduleScope,
    Subroutine <SgProcedureHeaderStatement> * userSubroutine,
    FortranParallelLoop * parallelLoop) :
  FortranOpenMPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "OpenMP kernel subroutine creation for indirect loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

 // createStatements ();
}
