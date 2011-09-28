#include <FortranOpenMPModuleDeclarationsIndirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <Debug.h>
#include <CommonNamespaces.h>
#include <Plan.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranOpenMPModuleDeclarationsIndirectLoop::createOpDatSizeDeclarations ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildPointerType;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT size declarations at module scope",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = OP2::VariableNames::getOpDatCardinalityName (i);

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
  using SageBuilder::buildPointerType;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
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

  variableDeclarations ->add (OP2::VariableNames::PlanFunction::planRet,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::PlanFunction::planRet, c_ptrType, moduleScope));

  SgType * op_planType = FortranTypesBuilder::buildClassDeclaration ("op_plan",
      moduleScope)->get_type ();

  variableDeclarations->add (OP2::VariableNames::PlanFunction::actualPlan,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::PlanFunction::actualPlan, buildPointerType (
              op_planType), moduleScope));

  /*
   * ======================================================
   * Create pointer to the
   * ======================================================
   */

  variableDeclarations->add (OP2::VariableNames::PlanFunction::pindMaps,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::PlanFunction::pindMaps, buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_ptrType)), moduleScope));

  variableDeclarations->add (OP2::VariableNames::PlanFunction::pindMapsSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::PlanFunction::pindMapsSize,
          FortranTypesBuilder::getArray_RankOne (
              FortranTypesBuilder::getFourByteInteger (), 1,
              parallelLoop->getNumberOfDistinctIndirectOpDatArguments ()),
          moduleScope));

  variableDeclarations->add (OP2::VariableNames::PlanFunction::pmaps,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::PlanFunction::pmaps, buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_ptrType)), moduleScope));

  variableDeclarations->add (OP2::VariableNames::PlanFunction::pindMapsSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::PlanFunction::pindMapsSize,
          FortranTypesBuilder::getFourByteInteger (), moduleScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const variableName =
            OP2::VariableNames::getLocalToGlobalMappingName (i);

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
      string const variableName =
          OP2::VariableNames::getGlobalToLocalMappingName (i);

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
      string const variableName =
          OP2::VariableNames::getGlobalToLocalMappingSizeName (i);

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

  planIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::ncolblk);

  planIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pnindirect);

  planIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pindSizes);

  planIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pindOffs);

  planIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pblkMap);

  planIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::poffset);

  planIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pnelems);

  planIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pnthrcol);

  planIntegerArrayVariables.push_back (
      OP2::VariableNames::PlanFunction::pthrcol);

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

  planSizeVariables.push_back (OP2::VariableNames::PlanFunction::pindSizesSize);

  planSizeVariables.push_back (OP2::VariableNames::PlanFunction::pindOffsSize);

  planSizeVariables.push_back (OP2::VariableNames::PlanFunction::pblkMapSize);

  planSizeVariables.push_back (OP2::VariableNames::PlanFunction::poffsetSize);

  planSizeVariables.push_back (OP2::VariableNames::PlanFunction::pnelemsSize);

  planSizeVariables.push_back (OP2::VariableNames::PlanFunction::pnthrcolSize);

  planSizeVariables.push_back (OP2::VariableNames::PlanFunction::pthrcolSize);

  for (vector <string>::iterator it = planSizeVariables.begin (); it
      != planSizeVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), moduleScope));
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

SgVariableDeclaration *
FortranOpenMPModuleDeclarationsIndirectLoop::getGlobalOpDatSizeDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  return variableDeclarations->get (OP2::VariableNames::getOpDatCardinalityName (
      OP_DAT_ArgumentGroup));
}

SgVariableDeclaration *
FortranOpenMPModuleDeclarationsIndirectLoop::getGlobalToLocalMappingDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  return variableDeclarations->get (
      OP2::VariableNames::getGlobalToLocalMappingName (OP_DAT_ArgumentGroup));
}

SgVariableDeclaration *
FortranOpenMPModuleDeclarationsIndirectLoop::getGlobalToLocalMappingSizeDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  return variableDeclarations->get (
      OP2::VariableNames::getGlobalToLocalMappingSizeName (OP_DAT_ArgumentGroup));
}

SgVariableDeclaration *
FortranOpenMPModuleDeclarationsIndirectLoop::getLocalToGlobalMappingDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  return variableDeclarations->get (
      OP2::VariableNames::getLocalToGlobalMappingName (OP_DAT_ArgumentGroup));
}

SgVariableDeclaration *
FortranOpenMPModuleDeclarationsIndirectLoop::getLocalToGlobalMappingSizeDeclaration (
    unsigned int OP_DAT_ArgumentGroup)
{
  return variableDeclarations->get (
      OP2::VariableNames::getLocalToGlobalMappingSizeName (OP_DAT_ArgumentGroup));
}

SgVariableDeclaration *
FortranOpenMPModuleDeclarationsIndirectLoop::getPlanFunctionDeclaration (
    std::string const & variableName)
{
  return variableDeclarations->get (variableName);
}

FortranOpenMPModuleDeclarationsIndirectLoop::FortranOpenMPModuleDeclarationsIndirectLoop (
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope) :
  FortranOpenMPModuleDeclarations (parallelLoop, moduleScope)
{
  createExecutionPlanDeclarations ();

  createOpDatDeclarations ();

  createOpDatSizeDeclarations ();
}
