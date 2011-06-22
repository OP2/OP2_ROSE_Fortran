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
      "Generating OP_DAT size declarations at module scope", 5);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = VariableNames::getOpDatSizeName (i);

      moduleDeclarations->add (variableName,
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
      "Creating execution plan variables at module scope", 5);

  /*
   * ======================================================
   * Create pointers to the execution plan on the C and
   * Fortran side
   * ======================================================
   */

  SgType * c_ptrType = FortranTypesBuilder::buildNewTypeDeclaration ("c_ptr",
      moduleScope)->get_type ();

  moduleDeclarations ->add (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::planRet,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::planRet,
          c_ptrType, moduleScope));

  SgType * op_planType = FortranTypesBuilder::buildNewTypeDeclaration (
      "op_plan", moduleScope)->get_type ();

  moduleDeclarations->add (
      IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectLoop::Fortran::HostSubroutine::VariableNames::actualPlan,
          buildPointerType (op_planType), moduleScope));

  /*
   * ======================================================
   * Create pointer to the
   * ======================================================
   */

  moduleDeclarations->add (PlanFunction::Fortran::pindMaps,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::Fortran::pindMaps, buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_ptrType)), moduleScope));

  moduleDeclarations->add (PlanFunction::Fortran::pindMapsSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::Fortran::pindMapsSize,
          FortranTypesBuilder::getArray_RankOne (
              FortranTypesBuilder::getFourByteInteger (), 1,
              parallelLoop->getNumberOfDistinctIndirectOpDatArguments ()),
          moduleScope));

  moduleDeclarations->add (PlanFunction::Fortran::pmaps,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::Fortran::pmaps, buildPointerType (
              FortranTypesBuilder::getArray_RankOne (c_ptrType)), moduleScope));

  moduleDeclarations->add (PlanFunction::Fortran::pindMapsSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          PlanFunction::Fortran::pindMapsSize,
          FortranTypesBuilder::getFourByteInteger (), moduleScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName =
          VariableNames::getLocalToGlobalMappingName (i);

      moduleDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getArray_RankOne (
                  FortranTypesBuilder::getFourByteInteger ()), moduleScope, 1,
              ALLOCATABLE));
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
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName =
          VariableNames::getGlobalToLocalMappingName (i);

      moduleDeclarations->add (variableName,
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
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName =
          VariableNames::getGlobalToLocalMappingSizeName (i);

      moduleDeclarations->add (variableName,
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
      IndirectLoop::Fortran::HostSubroutine::VariableNames::ncolblk);

  planIntegerArrayVariables.push_back (PlanFunction::Fortran::pnindirect);

  planIntegerArrayVariables.push_back (PlanFunction::Fortran::pindSizes);

  planIntegerArrayVariables.push_back (PlanFunction::Fortran::pindOffs);

  planIntegerArrayVariables.push_back (PlanFunction::Fortran::pblkMap);

  planIntegerArrayVariables.push_back (PlanFunction::Fortran::poffset);

  planIntegerArrayVariables.push_back (PlanFunction::Fortran::pnelems);

  planIntegerArrayVariables.push_back (PlanFunction::Fortran::pnthrcol);

  planIntegerArrayVariables.push_back (PlanFunction::Fortran::pthrcol);

  for (vector <string>::iterator it = planIntegerArrayVariables.begin (); it
      != planIntegerArrayVariables.end (); ++it)
  {
    moduleDeclarations->add (*it,
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

  planSizeVariables.push_back (PlanFunction::Fortran::pindSizesSize);

  planSizeVariables.push_back (PlanFunction::Fortran::pindOffsSize);

  planSizeVariables.push_back (PlanFunction::Fortran::pblkMapSize);

  planSizeVariables.push_back (PlanFunction::Fortran::poffsetSize);

  planSizeVariables.push_back (PlanFunction::Fortran::pnelemsSize);

  planSizeVariables.push_back (PlanFunction::Fortran::pnthrcolSize);

  planSizeVariables.push_back (PlanFunction::Fortran::pthrcolSize);

  for (vector <string>::iterator it = planSizeVariables.begin (); it
      != planSizeVariables.end (); ++it)
  {
    moduleDeclarations->add (*it,
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
  return moduleDeclarations->get (VariableNames::getOpDatSizeName (
      OP_DAT_ArgumentGroup));
}

VariableDeclarations *
FortranOpenMPModuleDeclarationsIndirectLoop::getAllDeclarations ()
{
  return moduleDeclarations;
}

FortranOpenMPModuleDeclarationsIndirectLoop::FortranOpenMPModuleDeclarationsIndirectLoop (
    std::string const & userSubroutineName, ParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  FortranOpenMPModuleDeclarations (userSubroutineName, parallelLoop,
      moduleScope)
{
  createExecutionPlanDeclarations ();

  createOpDatDeclarations ();

  createOpDatSizeDeclarations ();
}
