#include <CPPOpenCLKernelSubroutineIndirectLoop.h>
#include <CPPOpenCLUserSubroutine.h>
#include <CPPParallelLoop.h>
#include <CommonNamespaces.h>
#include <RoseHelper.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <Plan.h>
#include <Debug.h>
#include <boost/lexical_cast.hpp>

SgStatement *
CPPOpenCLKernelSubroutineIndirectLoop::createUserSubroutineCallStatement ()
{
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprListExp;
  using std::string;
  using std::vector;

  //TODO: double check

  Debug::getInstance ()->debugMessage (
      "Creating call to user device subroutine", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExpression * n_ref = buildVarRefExp (variableDeclarations->get (
      CommonVariableNames::iterationCounter1));

  SgExprListExp * userDeviceSubroutineParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * argN_ref = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpDatName (i))); //FIXME: same as parameter
    SgExpression * argN_l_ref = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpDatLocalName (i)));
    SgExpression * ind_argN_s = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpIndirectionSharedName (i)));
    SgExpression * argN_maps_ref = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getGlobalToLocalMappingName (i)));
    SgExpression * dimN_val = buildIntVal (parallelLoop->getOpDatDimension (i));

    //int dim = parallelLoop->getOpDatDimension (i);

    SgExpression * parameterExpression = buildIntVal (1);

    if (parallelLoop->isGlobal (i))
    {
      if (parallelLoop->isRead (i))
      {
        Debug::getInstance ()->debugMessage ("OP_GBL with read access",
            Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        /*
         * ======================================================
         * Case of global variable accessed in read mode:
         * we directly access the device variable, by
         * passing the kernel the variable name in positions
         * 0:argSize%<devVarName>-1. The name of the proper field
         * is obtained by appending "argument", <i>, and "_Size"
         * ======================================================
         */
        parameterExpression = argN_ref;
      }
      else
      {
        Debug::getInstance ()->debugMessage ("OP_GBL with write access",
            Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        /*
         * ======================================================
         * Case of global variable accessed NOT in read mode:
         * we access the corresponding local thread variable
         * ======================================================
         */

        parameterExpression = argN_l_ref;
      }
    }
    else if (parallelLoop->isIndirect (i))
    {
      if (parallelLoop->isIncremented (i))
      {
        Debug::getInstance ()->debugMessage (
            "Indirect OP_DAT with increment access", Debug::OUTER_LOOP_LEVEL,
            __FILE__, __LINE__);

        parameterExpression = argN_l_ref;
      }
      else
      {
        Debug::getInstance ()->debugMessage (
            "Indirect OP_DAT with read/write access", Debug::OUTER_LOOP_LEVEL,
            __FILE__, __LINE__);
      }
    }
    else if (parallelLoop->isDirect (i))
    {
      Debug::getInstance ()->debugMessage ("Direct OP_DAT",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);
    }

    userDeviceSubroutineParameters->append_expression (parameterExpression);
  }

  //needed to have the constants accessible in the user kernel
  SgExpression * global_constants_ref = buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::globalConstants));

  userDeviceSubroutineParameters->append_expression (global_constants_ref);

  return buildFunctionCallStmt (userSubroutine->getSubroutineName (),
      buildVoidType (), userDeviceSubroutineParameters, subroutineScope);
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createPointeredIncrementsOrWritesStatements ()
{
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildModOp;
  using SageBuilder::buildPlusAssignOp;
  using SageBuilder::buildForStatement;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildWhileStmt;
  using SageInterface::appendStatement;

  unsigned int pindOffsOffset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isIndirect (i))
    {
      if (parallelLoop->isWritten (i) || parallelLoop->isReadAndWritten (i)
          || parallelLoop->isIncremented (i))
      {
        SgExpression * argN_ref = buildVarRefExp (variableDeclarations->get (
            OP2::VariableNames::getOpDatName (i)));
        SgExpression * ind_argN_size_ref = buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getIndirectionArgumentSizeName (i)));
        SgExpression * argN_maps_ref = buildVarRefExp (
            variableDeclarations->get (
                OP2::VariableNames::getGlobalToLocalMappingName (i)));
        SgExpression * n_ref = buildVarRefExp (variableDeclarations->get (
            CommonVariableNames::iterationCounter1));
        SgExpression * dimN_val = buildIntVal (parallelLoop->getOpDatDimension (
            i));

        SgStatement * testExpression = buildExprStatement (buildLessThanOp (
            n_ref, buildMultiplyOp (ind_argN_size_ref, dimN_val)));
      }
      pindOffsOffset++;
    }
  }
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createInnerExecutionLoopStatements (
    SgScopeStatement * scope)
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildPlusPlusOp;
  using SageBuilder::buildPlusAssignOp;
  using SageBuilder::buildForStatement;
  using SageInterface::appendStatement;

  SgExpression * col2_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::colour2));
  SgExpression * varCol = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::colour1));
  SgExpression * varNcolor = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::numberOfColours));

  SgStatement * outerLoopInitialisationExpression = buildAssignStatement (
      varCol, buildIntVal (0));

  SgStatement * outerLoopTestExpression = buildExprStatement (buildLessThanOp (
      varCol, varNcolor));

  SgExpression * outerLoopIncrementExpression = buildPlusPlusOp (varCol);

  SgBasicBlock * outerLoopBody = buildBasicBlock ();

  SgForStatement * outerLoopStatement = buildForStatement (
      outerLoopInitialisationExpression, outerLoopTestExpression,
      outerLoopIncrementExpression, outerLoopBody);

  appendStatement (outerLoopStatement, scope);

  SgExpression * ifGuardExpression = buildEqualityOp (col2_ref, varCol);

  SgBasicBlock * ifBody = buildBasicBlock ();

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  appendStatement (ifStatement, outerLoopBody);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      //SgExpression * argN_ref = buildVarRefExp(variableDeclarations->get(OP2::VariableNames::getOpDatName(i)));
      SgExpression * argN_l_ref = buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::getOpDatLocalName (i)));
      SgExpression * ind_argN_s_ref =
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatSharedName (i)));
      //SgExpression * ind_argN_size_ref = buildVarRefExp(variableDeclarations->get(OP2::VariableNames::getOpDatSizeName(i)));
      SgExpression * varLocalToGlobalMapping = buildVarRefExp (
          variableDeclarations->get (
              OP2::VariableNames::getLocalToGlobalMappingName (i)));
      SgExpression * dimN_val = buildIntVal (
          parallelLoop->getOpDatDimension (i));
      SgExpression * varIterationCounter2 = buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2));

      SgStatement * innerLoopInitialisationExpression = buildExprStatement (
          buildAssignOp (varIterationCounter2, buildIntVal (0)));

      SgStatement * innerLoopTestExpression = buildExprStatement (
          buildLessThanOp (varIterationCounter2, dimN_val));

      SgExpression * innerLoopIncrementExpression = buildPlusPlusOp (
          varIterationCounter2);

      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      SgForStatement * innerLoopStatement = buildForStatement (
          innerLoopInitialisationExpression, innerLoopTestExpression,
          innerLoopIncrementExpression, innerLoopBody);

      appendStatement (innerLoopStatement, outerLoopBody);

      SgStatement * innerLoopAssignmentStatement = buildExprStatement (
          buildPlusAssignOp (buildPntrArrRefExp (ind_argN_s_ref, buildAddOp (
              varIterationCounter2, buildMultiplyOp (varLocalToGlobalMapping, //FIXME
                  dimN_val))), buildPntrArrRefExp (argN_l_ref,
              varIterationCounter2)));

      appendStatement (innerLoopAssignmentStatement, innerLoopBody);

    }
  }
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createIncrementAdjustmentStatements (
    SgScopeStatement * scope)
{
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createInitialiseLocalOpDatStatements (
    SgScopeStatement * scope)
{
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildForStatement;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildPlusPlusOp;
  using SageInterface::appendStatement;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIncremented (i))
    {
      SgBasicBlock * loopBody = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              OP2::VariableNames::getOpDatLocalName (i))),
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter2)));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression, buildIntVal (0)); //FIXME 0.0f?

      appendStatement (assignmentStatement, loopBody);

      SgStatement * initialisationExpression = buildExprStatement (
          buildAssignOp (buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter2)), buildIntVal (0)));

      SgStatement * testExpression = buildExprStatement (buildLessThanOp (
          buildVarRefExp (variableDeclarations->get (
              CommonVariableNames::iterationCounter2)), buildIntVal (
              parallelLoop->getOpDatDimension (i))));

      SgExpression * incrementExpression = buildPlusPlusOp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::iterationCounter2)));

      SgForStatement * forStatement = buildForStatement (
          initialisationExpression, testExpression, incrementExpression,
          loopBody);

      appendStatement (forStatement, scope);
    }
  }
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createExecutionLoopStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildPlusAssignOp;
  using SageBuilder::buildPlusPlusOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildForStatement;
  using SageBuilder::buildWhileStmt;
  using SageInterface::appendStatement;

  SgExpression * colors_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::pthrcol));
  SgExpression * n_ref = buildVarRefExp (variableDeclarations->get (
      CommonVariableNames::iterationCounter1));
  SgExpression * nelems2_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::nelems2));
  SgExpression * col2_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::colour2));
  SgExpression * nelem_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::nelems));
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createAutoSharedWhileLoopStatements ()
{
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createInitialiseLocalVariablesStatements ()
{
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildSizeOfOp;
  using SageBuilder::buildFloatType;
  using SageBuilder::buildCastExp;
  using SageBuilder::buildFunctionRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildPlusAssignOp;
  using SageBuilder::buildIntVal;
  using SageInterface::appendStatement;
  using std::string;

  SgExpression * nbytes_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::nbytes));

  bool firstInitialization = true;
  //unsigned int previous_OP_DAT_Location;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const autosharedVariableName =
        OP2::VariableNames::getCUDASharedMemoryDeclarationName (
            parallelLoop->getOpDatBaseType (i),
            parallelLoop->getSizeOfOpDat (i));

    SgExpression * shared_ref = buildVarRefExp (variableDeclarations->get (
        autosharedVariableName));

    SgExpression * ind_argN_s_ref = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpIndirectionSharedName (i)));
    SgExpression * ind_argN_size_ref = buildVarRefExp (
        variableDeclarations->get (
            OP2::VariableNames::getIndirectionArgumentSizeName (i)));
    SgExpression * dimN_val = buildIntVal (parallelLoop->getOpDatDimension (i));

    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isIndirect (i))
    {
      SgExprStatement * incrementStatement;

      if (firstInitialization)
      {
        firstInitialization = false;

        incrementStatement = buildAssignStatement (nbytes_ref, buildIntVal (0));
      }
      else
      {
        incrementStatement = buildExprStatement (buildPlusAssignOp (nbytes_ref,
            buildFunctionCallExp (buildFunctionRefExp ("ROUND_UP"),
                buildExprListExp (buildMultiplyOp (ind_argN_size_ref,
                    buildMultiplyOp (buildSizeOfOp (buildFloatType ()),
                        dimN_val))))));
      }

      appendStatement (incrementStatement, subroutineScope);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          ind_argN_s_ref, buildCastExp (buildAddOp (shared_ref, nbytes_ref),
              buildFloatType ())); //FIXME (__local TYPE *)

      appendStatement (assignmentStatement, subroutineScope);
    }
  }
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createThreadZeroStatements ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildDivideOp;
  using SageInterface::appendStatement;

  SgExpression * blockId_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::blockID));
  SgExpression * blkmap_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::pblkMap));
  SgExpression * block_offset_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::blockOffset));
  SgExpression * nelem_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::nelems));
  SgExpression * nelems_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::pnelems));
  SgExpression * offset_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::poffset));
  SgExpression * nelems2_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::nelems2));
  SgExpression * ncolor_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::numberOfColours));
  SgExpression * ncolors_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::pnthrcol));

  /*
   * ======================================================
   * The body of the if statement
   * ======================================================
   */

  SgBasicBlock * ifBlock = buildBasicBlock ();

  /*
   * ======================================================
   * Build the if-guard
   * ======================================================
   */

  /*
   * ======================================================
   * 2nd statement: nelem = nelems[ blockId ]
   * ======================================================
   */

  SgStatement * statement2 = buildAssignStatement (nelem_ref,
      buildPntrArrRefExp (nelems_ref, blockId_ref));

  appendStatement (statement2, ifBlock);

  /*
   * ======================================================
   * 5th statement: assignment of ncolor
   * ======================================================
   */

  SgStatement * statement5 = buildAssignStatement (ncolor_ref,
      buildPntrArrRefExp (ncolors_ref, blockId_ref));

  appendStatement (statement5, ifBlock);

  SgExpression * ind_arg_sizes_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::pindSizes));
  SgExpression * ind_arg_offs_ref = buildVarRefExp (variableDeclarations->get (
      OP2::VariableNames::PlanFunction::pindOffs));

  unsigned int pindSizesArrayOffset;

  /*
   * ======================================================
   * assignment of ind_arg_size variables
   * ======================================================
   */

  pindSizesArrayOffset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * ind_argN_ref = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpIndirectionName (i)));
    SgExpression * ind_argN_map_ref = buildVarRefExp (
        variableDeclarations->get (
            OP2::VariableNames::getIncrementAccessMapName (i)));
    SgExpression * ind_argN_maps_ref = buildVarRefExp (
        variableDeclarations->get (
            OP2::VariableNames::getLocalToGlobalMappingName (i)));
    SgExpression * ind_argN_s_ref = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpIndirectionSharedName (i)));
    SgExpression * ind_argN_size_ref = buildVarRefExp (
        variableDeclarations->get (
            OP2::VariableNames::getIndirectionArgumentSizeName (i)));
    SgExpression * dimN_val = buildIntVal (parallelLoop->getOpDatDimension (i));

    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isIndirect (i))
    {
      SgStatement * statement6 = buildAssignStatement (ind_argN_size_ref,
          buildPntrArrRefExp (ind_arg_sizes_ref, buildAddOp (buildIntVal (
              pindSizesArrayOffset), buildMultiplyOp (blockId_ref, dimN_val))));

      appendStatement (statement6, ifBlock);

      ++pindSizesArrayOffset;
    }
  }

  /*
   * ======================================================
   * assignment of ind_arg_map variables
   * ======================================================
   */

  pindSizesArrayOffset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * ind_argN_ref = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpIndirectionName (i)));
    SgExpression * ind_argN_map_ref = buildVarRefExp (
        variableDeclarations->get (
            OP2::VariableNames::getIncrementAccessMapName (i)));
    SgExpression * ind_argN_maps_ref = buildVarRefExp (
        variableDeclarations->get (
            OP2::VariableNames::getLocalToGlobalMappingName (i)));
    SgExpression * ind_argN_s_ref = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpIndirectionSharedName (i)));
    SgExpression * ind_argN_size_ref = buildVarRefExp (
        variableDeclarations->get (
            OP2::VariableNames::getIndirectionArgumentSizeName (i)));
    SgExpression * dimN_val = buildIntVal (parallelLoop->getOpDatDimension (i));

    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isIndirect (i))
    {
      SgStatement * statement7 = buildAssignStatement (ind_argN_map_ref,
          buildAddOp (ind_argN_maps_ref, buildPntrArrRefExp (ind_arg_offs_ref,
              buildAddOp (buildIntVal (pindSizesArrayOffset), buildMultiplyOp (
                  blockId_ref, dimN_val)))));

      appendStatement (statement7, ifBlock);

      ++pindSizesArrayOffset;
    }
  }
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createPlanFormalParameterDeclarations ()
{
  using SageBuilder::buildIntType;
  using SageBuilder::buildPointerType;
  using std::string;
  using std::vector;

  vector <string> intPtrVariables;

  intPtrVariables.push_back (OP2::VariableNames::PlanFunction::pindSizes);

  intPtrVariables.push_back (OP2::VariableNames::PlanFunction::pindOffs);

  intPtrVariables.push_back (OP2::VariableNames::PlanFunction::pblkMap);

  intPtrVariables.push_back (OP2::VariableNames::PlanFunction::poffset);

  intPtrVariables.push_back (OP2::VariableNames::PlanFunction::pnelems);

  intPtrVariables.push_back (OP2::VariableNames::PlanFunction::pnthrcol);

  intPtrVariables.push_back (OP2::VariableNames::PlanFunction::pthrcol);

  for (vector <string>::const_iterator it = intPtrVariables.begin (); it
      != intPtrVariables.end (); ++it)
  {
    variableDeclarations->add (
        *it,
        RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            *it, buildPointerType (buildIntType ()), subroutineScope,
            formalParameters));
  }

  vector <string> intVariables;

  intVariables.push_back (OP2::VariableNames::PlanFunction::blockOffset);

  for (vector <string>::const_iterator it = intVariables.begin (); it
      != intVariables.end (); ++it)
  {
    variableDeclarations->add (
        *it,
        RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            *it, buildIntType (), subroutineScope, formalParameters));
  }
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createOpDatFormalParameterDeclarations ()
{
  using SageBuilder::buildPointerType;
  using SageBuilder::buildShortType;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildPointerType;
  using SageBuilder::buildIntType;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters",
      Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isIndirect (i))
    {
      /*
       * ======================================================
       * The base type of an OP_DAT must always be an array
       * ======================================================
       */

      SgType * opdatType = parallelLoop->getOpDatType (i);

      SgArrayType * isArrayType = isSgArrayType (opdatType);

      SgType * opDatBaseType = isArrayType->get_base_type ();

      SgPointerType * pointerType = buildPointerType (opDatBaseType);

      variableDeclarations->add (
          OP2::VariableNames::getOpDatName (i),
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              OP2::VariableNames::getOpDatName (i), pointerType,
              subroutineScope, formalParameters));

      variableDeclarations->add (
          OP2::VariableNames::getLocalToGlobalMappingName (i),
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              OP2::VariableNames::getLocalToGlobalMappingName (i),
              buildPointerType (buildIntType ()), subroutineScope,
              formalParameters));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {

      variableDeclarations->add (
          OP2::VariableNames::getGlobalToLocalMappingName (i),
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              OP2::VariableNames::getGlobalToLocalMappingName (i),
              buildPointerType (buildShortType ()), subroutineScope,
              formalParameters));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false && (parallelLoop->isDirect (
        i) || parallelLoop->isGlobal (i)))
    {
      string const variableName = OP2::VariableNames::getOpDatName (i);

      SgType * opDatType = parallelLoop->getOpDatType (i);

      SgArrayType * isArrayType = isSgArrayType (opDatType);

      SgType * opDatBaseType = isArrayType->get_base_type ();

      SgPointerType * pointerType = buildPointerType (opDatBaseType);

      variableDeclarations->add (
          variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName, pointerType, subroutineScope, formalParameters));
    }
  }
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createStatements ()
{
  createThreadZeroStatements ();

  createInitialiseLocalVariablesStatements ();

  createAutoSharedWhileLoopStatements ();

  createExecutionLoopStatements ();

  createPointeredIncrementsOrWritesStatements ();
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  using SageBuilder::buildIntType;
  using boost::lexical_cast;
  using std::string;
  using std::vector;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName =
        OP2::VariableNames::getNumberOfBytesVariableName (i);

    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      variableDeclarations->add (variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, buildIntType (), subroutineScope));

      positionOfNbytes[parallelLoop->getOpDatVariableName (i)] = i;
    }
    else
    {
      variableDeclarations->add (variableName, variableDeclarations->get (
          OP2::VariableNames::getNumberOfBytesVariableName (
              positionOfNbytes[parallelLoop->getOpDatVariableName (i)])));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = OP2::VariableNames::getRoundUpVariableName (
          i);

      variableDeclarations->add (variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, buildIntType (), subroutineScope));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIncremented (i))
    {
      string const variableName =
          OP2::VariableNames::getIncrementAccessMapName (i);

      variableDeclarations->add (variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, buildIntType (), subroutineScope));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false && parallelLoop->isDirect (
        i))
    {
      string const variableName =
          OP2::VariableNames::getIndirectionArgumentSizeName (i);

      variableDeclarations->add (variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, buildIntType (), subroutineScope));
    }
  }

  vector <string> sharedFourByteIntegerVariables;

  sharedFourByteIntegerVariables.push_back (OP2::VariableNames::blockID);

  sharedFourByteIntegerVariables.push_back (OP2::VariableNames::numberOfColours);

  sharedFourByteIntegerVariables.push_back (OP2::VariableNames::nelems);

  sharedFourByteIntegerVariables.push_back (OP2::VariableNames::nelems2);

  for (vector <string>::const_iterator it =
      sharedFourByteIntegerVariables.begin (); it
      != sharedFourByteIntegerVariables.end (); ++it)
  {
    variableDeclarations->add (*it,
        RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildIntType (), subroutineScope));
  }

  vector <string> integerVariables;

  integerVariables.push_back (OP2::VariableNames::colour1);

  integerVariables.push_back (OP2::VariableNames::colour2);

  integerVariables.push_back (CommonVariableNames::iterationCounter1);

  integerVariables.push_back (CommonVariableNames::iterationCounter2);

  integerVariables.push_back (CommonVariableNames::upperBound);

  integerVariables.push_back (OP2::VariableNames::nbytes);

  for (vector <string>::const_iterator it = integerVariables.begin (); it
      != integerVariables.end (); ++it)
  {
    variableDeclarations ->add (*it,
        RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildIntType (), subroutineScope));
  }
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createFormalParameterDeclarations ()
{
  using SageBuilder::buildPointerType;
  using SageBuilder::buildCharType;

  createOpDatFormalParameterDeclarations ();

  createPlanFormalParameterDeclarations ();

  variableDeclarations->add (
      OpenCL::CPP::globalConstants,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OpenCL::CPP::globalConstants, buildPointerType (buildCharType ()),
          subroutineScope, formalParameters));

}

CPPOpenCLKernelSubroutineIndirectLoop::CPPOpenCLKernelSubroutineIndirectLoop (
    SgScopeStatement * moduleScope, CPPOpenCLUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines) :
  CPPOpenCLKernelSubroutine (moduleScope, userSubroutine, parallelLoop,
      reductionSubroutines)
{
  Debug::getInstance ()->debugMessage ("<Kernel, Indirect, OpenCL>",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
