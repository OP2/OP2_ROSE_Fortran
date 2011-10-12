#include <CPPOpenCLHostSubroutineIndirectLoop.h>
#include <CPPPlan.h>
#include <CPPOpenCLStatementsAndExpressionsBuilder.h>
#include <CPPParallelLoop.h>
#include <CPPOpenCLModuleDeclarations.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <RoseHelper.h>
#include <Debug.h>

SgStatement *
CPPOpenCLHostSubroutineIndirectLoop::createKernelFunctionCallStatement ()
{
  using SageBuilder::buildDotExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildVarRefExp;

  SgExprListExp * actualParameters = buildExprListExp ();

  std::vector <std::pair <SgExpression *, SgExpression *> > kernelArguments;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isIndirect (i))
    {
      actualParameters->append_expression (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatDeviceName (i))));

      actualParameters->append_expression (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getLocalToGlobalMappingName (i))));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      actualParameters->append_expression (buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getGlobalToLocalMappingName (i))));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false && (parallelLoop->isDirect (
        i) || parallelLoop->isGlobal (i)))
    {
      actualParameters->append_expression (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatDeviceName (i))));
    }
  }

  actualParameters->append_expression (buildDotExp (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::actualPlan)), buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::PlanFunction::pindSizes))));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::pindOffs)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::pblkMap)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::poffset)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::pnelems)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::pnthrcol)));

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OP2::VariableNames::PlanFunction::pthrcol)));

  actualParameters->append_expression (
      buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::PlanFunction::blockOffset)));

  SgExprStatement * callStatement = buildFunctionCallStmt (
      calleeSubroutine->getSubroutineName () + "<<<"
          + RoseHelper::getFirstVariableName (variableDeclarations->get (
              OpenCL::CPP::blocksPerGrid)) + ", "
          + RoseHelper::getFirstVariableName (variableDeclarations->get (
              OpenCL::CPP::threadsPerBlock)) + ", "
          + RoseHelper::getFirstVariableName (variableDeclarations->get (
              OpenCL::CPP::sharedMemorySize)) + ">>>", buildVoidType (),
      actualParameters, subroutineScope);

  return callStatement;
}

void
CPPOpenCLHostSubroutineIndirectLoop::createPlanFunctionExecutionStatements ()
{
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildArrowExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildPlusPlusOp;
  using SageBuilder::buildForStatement;
  using SageBuilder::buildPlusAssignOp;
  using SageBuilder::buildMultiplyOp;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating plan function execution statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExpression * block_offset_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::blockOffset));
  SgExpression * col_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::colour1));
  SgExpression * Plan_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::actualPlan));
  SgExpression * nblocks_ref = buildVarRefExp (variableDeclarations->get (
      OpenCL::CPP::blocksPerGrid));
  SgExpression * nthreads_ref = buildVarRefExp (variableDeclarations->get (
      OpenCL::CPP::threadsPerBlock));
  SgExpression * nthreadstot_ref = buildVarRefExp (variableDeclarations->get (
      OpenCL::CPP::totalThreads));
  SgExpression * nshared_ref = buildVarRefExp (variableDeclarations->get (
      OpenCL::CPP::sharedMemorySize));
  //SgExpression * reduct_size_ref = buildVarRefExp (variableDeclarations->get (ReductionSubroutine::reductionArraySize));
  //SgExpression * offset_s_ref = buildVarRefExp (variableDeclarations->get (DirectLoop::CPP::KernelSubroutine::warpScratchpadSize));
  SgExpression * OP_block_size_ref = buildVarRefExp (variableDeclarations->get (
      OpenCL::CPP::totalThreads));

  SgStatement * tempStatement = NULL;

  /*
   * ======================================================
   * block_offset = 0
   * ======================================================
   */

  tempStatement = buildAssignStatement (block_offset_ref, buildIntVal (0));

  appendStatement (tempStatement, subroutineScope);

  /*
   * ======================================================
   * Plan = op_plan_get( name, set, part_size, nargs, args, ninds, inds )
   * ======================================================
   */

  /* 
   * ======================================================
   * BEGIN for ( col=0; col < Plan->ncolors; col++ )
   * ======================================================
   */

  SgStatement * initialisationExpression1 = buildExprStatement (buildAssignOp (
      col_ref, buildIntVal (0)));

  SgStatement * testExpression1 = buildExprStatement (buildLessThanOp (col_ref,
      buildArrowExp (Plan_ref, buildOpaqueVarRefExp ("ncolors"))));

  SgExpression * incrementExpression1 = buildPlusPlusOp (col_ref);

  SgBasicBlock * loopBody1 = buildBasicBlock ();

  SgForStatement * forStatement1 = buildForStatement (
      initialisationExpression1, testExpression1, incrementExpression1,
      loopBody1);

  appendStatement (forStatement1, subroutineScope);

  /*
   * ======================================================
   * nthread = 128 //TODO: change in order to wrap with OP_BLOCK_SIZE (how?)
   * ======================================================
   */

  tempStatement = buildAssignStatement (nthreads_ref, OP_block_size_ref);

  appendStatement (tempStatement, loopBody1);

  /*
   * ======================================================
   * nblocks = Plan->ncolblk[col]
   * ======================================================
   */
  tempStatement = buildAssignStatement (nblocks_ref, buildPntrArrRefExp (
      buildArrowExp (Plan_ref, buildOpaqueVarRefExp ("ncolblk")), col_ref));

  appendStatement (tempStatement, loopBody1);

  /*
   * ======================================================
   * ntotthread = nblocks * nthread
   * ======================================================
   */

  tempStatement = buildAssignStatement (nthreadstot_ref, buildMultiplyOp (
      nblocks_ref, nthreads_ref));

  appendStatement (tempStatement, loopBody1);

  /* ======================================================
   * nshared = Plan->nshared
   * ======================================================
   */
  tempStatement = buildAssignStatement (nshared_ref, buildArrowExp (Plan_ref,
      buildOpaqueVarRefExp ("nshared")));

  appendStatement (tempStatement, loopBody1);

  appendStatement (createKernelFunctionCallStatement (), loopBody1);

  appendStatement (
      CPPOpenCLStatementsAndExpressionsBuilder::generateBarrierStatement (),
      loopBody1);

  /* 
   * ======================================================
   * END for ( col=0; col < plan->ncolors; col++ )
   * ======================================================
   */

  /* 
   * ======================================================
   * block_offset += nblocks
   * ======================================================
   */

  tempStatement = buildExprStatement (buildPlusAssignOp (block_offset_ref,
      nblocks_ref));

  appendStatement (tempStatement, subroutineScope);

}

void
CPPOpenCLHostSubroutineIndirectLoop::createExecutionPlanDeclarations ()
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildIntType;
  using SageBuilder::buildShortType;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildClassDeclaration;
  using SageBuilder::buildPointerType;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::map;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Creating execution plan variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Create pointer to the execution plan
   * ======================================================
   */

  variableDeclarations->add (OP2::VariableNames::PlanFunction::actualPlan,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::PlanFunction::actualPlan, buildPointerType (
              buildClassDeclaration ("op_plan", subroutineScope)->get_type ()),
          subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isIndirect (i))
    {
      string const variableName =
          OP2::VariableNames::getLocalToGlobalMappingName (i);

      variableDeclarations->add (variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, buildArrayType (buildIntType ()), subroutineScope));
    }
  }

  /*
   * ======================================================
   * Create arrays for OP_DAT, OP_INDIRECTION,
   * OP_MAP, OP_ACCESS arguments. These arrays are filled up
   * with the actual values of the OP_DAT, OP_INDIRECTION,
   * OP_MAP, OP_ACCESS that are passed to the OP_PAR_LOOP;
   * these arrays are then given to the plan function.
   * Note, therefore, that the size of the arrays is exactly
   * the number of OP_DAT argument groups.
   * There is an additional array 'inds' storing which
   * OP_DAT arguments are accessed through an indirection
   * ======================================================
   */

  vector <string> integerArrays;

  integerArrays.push_back (OP2::VariableNames::PlanFunction::args);

  integerArrays.push_back (OP2::VariableNames::PlanFunction::inds);

  for (vector <string>::iterator it = integerArrays.begin (); it
      != integerArrays.end (); ++it)
  {
    variableDeclarations->add (*it,
        RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildArrayType (buildIntType (), buildIntVal (
                parallelLoop->getNumberOfOpDatArgumentGroups ())),
            subroutineScope));
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

      variableDeclarations->add (
          variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, buildArrayType (buildShortType ()), subroutineScope));
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
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, buildIntType (), subroutineScope));
    }
  }

  /*
   * ======================================================
   * The plan function fills up a 'struct' which has a
   * number of integer fields. These fields need to be accessed
   * on the CPP side, so create local variables that
   * enable the data to be transferred accordingly
   * ======================================================
   */

  vector <string> integerVariables;

  integerVariables.push_back (OP2::VariableNames::colour1);

  integerVariables.push_back (CommonVariableNames::iterationCounter1);

  integerVariables.push_back (OP2::VariableNames::PlanFunction::argsNumber);

  integerVariables.push_back (OP2::VariableNames::PlanFunction::indsNumber);

  integerVariables.push_back (OP2::VariableNames::PlanFunction::blockOffset);

  integerVariables.push_back (OP2::VariableNames::PlanFunction::pindSizesSize);

  integerVariables.push_back (OP2::VariableNames::PlanFunction::pindOffsSize);

  integerVariables.push_back (OP2::VariableNames::PlanFunction::pblkMapSize);

  integerVariables.push_back (OP2::VariableNames::PlanFunction::poffsetSize);

  integerVariables.push_back (OP2::VariableNames::PlanFunction::pnelemsSize);

  integerVariables.push_back (OP2::VariableNames::PlanFunction::pnthrcolSize);

  integerVariables.push_back (OP2::VariableNames::PlanFunction::pthrcolSize);

  for (vector <string>::iterator it = integerVariables.begin (); it
      != integerVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildIntType (), subroutineScope));
  }

  /*
   * ======================================================
   * The plan function fills up a 'struct' which has a
   * number of integer array fields. These fields need to be
   * accessed on the CPP side ON THE HOST, so create local
   * variables that enable the data to be transferred accordingly
   * ======================================================
   */

  vector <string> integerPointerVariables;

  integerPointerVariables.push_back (OP2::VariableNames::PlanFunction::ncolblk);

  integerPointerVariables.push_back (
      OP2::VariableNames::PlanFunction::pnindirect);

  for (vector <string>::iterator it = integerPointerVariables.begin (); it
      != integerPointerVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildPointerType (buildArrayType (buildIntType ())),
            subroutineScope));
  }
}

void
CPPOpenCLHostSubroutineIndirectLoop::createStatements ()
{
  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionPrologueStatements ();
  }

  createPlanFunctionExecutionStatements ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionEpilogueStatements ();
  }
}

void
CPPOpenCLHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  createExecutionPlanDeclarations ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionLocalVariableDeclarations ();
  }
}

CPPOpenCLHostSubroutineIndirectLoop::CPPOpenCLHostSubroutineIndirectLoop (
    SgScopeStatement * moduleScope,
    CPPOpenCLKernelSubroutine * kernelSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPOpenCLHostSubroutine (moduleScope, kernelSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "Creating OpenCL host subroutine for indirect loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  plan = new CPPPlan (subroutineScope, parallelLoop, variableDeclarations);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
