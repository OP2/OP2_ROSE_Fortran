#include <FortranOpenMPModuleDeclarationsIndirectLoop.h>
#include <FortranParallelLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <ScopedVariableDeclarations.h>
#include <Debug.h>
#include <CompilerGeneratedNames.h>
#include <rose.h>

void
FortranOpenMPModuleDeclarationsIndirectLoop::createOpDatSizeDeclarations ()
{
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT size declarations at module scope",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = getOpDatCardinalityName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              moduleScope));
    }
  }
}

void
FortranOpenMPModuleDeclarationsIndirectLoop::createExecutionPlanDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::map;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating execution plan variables at module scope",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Create pointers to the execution plan on the C and
   * Fortran side
   * ======================================================
   */

  SgType * c_ptrType = FortranTypesBuilder::buildClassDeclaration ("c_ptr",
      moduleScope)->get_type ();

  variableDeclarations ->add (planRet,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          planRet, c_ptrType, moduleScope));

  SgType * op_planType = FortranTypesBuilder::buildClassDeclaration ("op_plan",
      moduleScope)->get_type ();

  variableDeclarations->add (actualPlan,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          actualPlan, buildPointerType (op_planType), moduleScope));

  variableDeclarations->add (pindMaps,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          pindMaps, buildPointerType (FortranTypesBuilder::getArray_RankOne (
              c_ptrType)), moduleScope));

  variableDeclarations->add (pindMapsSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          pindMapsSize, FortranTypesBuilder::getArray_RankOne (
              FortranTypesBuilder::getFourByteInteger (), 1,
              parallelLoop->getNumberOfDistinctIndirectOpDatArguments ()),
          moduleScope));

  variableDeclarations->add (pmaps,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (pmaps,
          buildPointerType (FortranTypesBuilder::getArray_RankOne (c_ptrType)),
          moduleScope));

  variableDeclarations->add (
      pindMapsSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          pindMapsSize, FortranTypesBuilder::getFourByteInteger (), moduleScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const variableName = getLocalToGlobalMappingName (i);

        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, FortranTypesBuilder::getArray_RankOne (
                    FortranTypesBuilder::getFourByteInteger ()), moduleScope,
                1, ALLOCATABLE));
      }
    }
  }

  /*
   * ======================================================
   * Create variables modelling the indirect mappings to
   * local indices in shared memory
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      string const variableName = getGlobalToLocalMappingName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, buildPointerType (
                  FortranTypesBuilder::getArray_RankOne (
                      FortranTypesBuilder::getTwoByteInteger ())), moduleScope,
              1, ALLOCATABLE));
    }
  }

  /*
   * ======================================================
   * Create variables modelling the number of elements
   * in each block of an indirect mapping
   * ======================================================
   */
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      string const variableName = getGlobalToLocalMappingSizeName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              moduleScope));
    }
  }

  /*
   * ======================================================
   * The plan function fills up a 'struct' which has a
   * number of array integer fields. These fields need to be accessed
   * on the Fortran side, so create variables
   * that enable the data to be transferred accordingly
   * ======================================================
   */

  vector <string> planIntegerArrayVariables;

  planIntegerArrayVariables.push_back (ncolblk);
  planIntegerArrayVariables.push_back (pnindirect);
  planIntegerArrayVariables.push_back (pindSizes);
  planIntegerArrayVariables.push_back (pindOffs);
  planIntegerArrayVariables.push_back (pblkMap);
  planIntegerArrayVariables.push_back (poffset);
  planIntegerArrayVariables.push_back (pnelems);
  planIntegerArrayVariables.push_back (pnthrcol);
  planIntegerArrayVariables.push_back (pthrcol);

  for (vector <string>::iterator it = planIntegerArrayVariables.begin (); it
      != planIntegerArrayVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildPointerType (FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger ())), moduleScope));
  }

  /*
   * ======================================================
   * The plan function fills up a 'struct' which has a
   * number of integer fields. These fields need to be accessed
   * on the Fortran side, so create local variables that
   * enable the data to be transferred accordingly
   * ======================================================
   */

  vector <string> planSizeVariables;

  planSizeVariables.push_back (pindSizesSize);
  planSizeVariables.push_back (pindOffsSize);
  planSizeVariables.push_back (pblkMapSize);
  planSizeVariables.push_back (poffsetSize);
  planSizeVariables.push_back (pnelemsSize);
  planSizeVariables.push_back (pnthrcolSize);
  planSizeVariables.push_back (pthrcolSize);

  for (vector <string>::iterator it = planSizeVariables.begin (); it
      != planSizeVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), moduleScope));
  }
}

SgVarRefExp *
FortranOpenMPModuleDeclarationsIndirectLoop::getGlobalOpDatSizeDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace OP2VariableNames;

  return variableDeclarations->getReference (getOpDatCardinalityName (
      OP_DAT_ArgumentGroup));
}

SgVarRefExp *
FortranOpenMPModuleDeclarationsIndirectLoop::getGlobalToLocalMappingDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace OP2VariableNames;

  return variableDeclarations->getReference (getGlobalToLocalMappingName (
      OP_DAT_ArgumentGroup));
}

SgVarRefExp *
FortranOpenMPModuleDeclarationsIndirectLoop::getGlobalToLocalMappingSizeDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace OP2VariableNames;

  return variableDeclarations->getReference (getGlobalToLocalMappingSizeName (
      OP_DAT_ArgumentGroup));
}

SgVarRefExp *
FortranOpenMPModuleDeclarationsIndirectLoop::getLocalToGlobalMappingDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace OP2VariableNames;

  return variableDeclarations->getReference (getLocalToGlobalMappingName (
      OP_DAT_ArgumentGroup));
}

SgVarRefExp *
FortranOpenMPModuleDeclarationsIndirectLoop::getLocalToGlobalMappingSizeDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace OP2VariableNames;

  return variableDeclarations->getReference (getLocalToGlobalMappingSizeName (
      OP_DAT_ArgumentGroup));
}

SgVarRefExp *
FortranOpenMPModuleDeclarationsIndirectLoop::getPlanFunctionDeclaration (
    std::string const & variableName)
{
  return variableDeclarations->getReference (variableName);
}

FortranOpenMPModuleDeclarationsIndirectLoop::FortranOpenMPModuleDeclarationsIndirectLoop (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranOpenMPModuleDeclarations (parallelLoop, moduleScope)
{
  createExecutionPlanDeclarations ();

  createOpDatDeclarations ();

  createOpDatSizeDeclarations ();
}
