#include <boost/lexical_cast.hpp>
#include <Debug.h>
//#include <CPPTypesBuilder.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CPPStatementsAndExpressionsBuilder.h>
#include <CPPOpenCLKernelSubroutineIndirectLoop.h>
#include <CommonNamespaces.h>
#include <RoseHelper.h>
#include <Plan.h>

using namespace SageBuilder;
/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
CPPOpenCLKernelSubroutineIndirectLoop::createUserSubroutineCallStatement ()
{
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

  Debug::getInstance ()->debugMessage ( "Creating call to user device subroutine", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
  
  SgExpression * n_ref = buildVarRefExp (variableDeclarations->get( CommonVariableNames::iterationCounter1));
  SgExpression * offset_b_ref = buildVarRefExp ( variableDeclarations->get(IndirectLoop::CPP::KernelSubroutine::VariableNames::blockOffsetShared));

  SgExprListExp * userDeviceSubroutineParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * argN_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatName(i))); //FIXME: same as parameter
    SgExpression * argN_l_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatLocalName(i)));
    SgExpression * ind_argN_s = buildVarRefExp(variableDeclarations->get(VariableNames::getOpIndirectionSharedName(i))); 
    SgExpression * argN_maps_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getGlobalToLocalMappingName(i)));
    SgExpression * dimN_val = buildIntVal(parallelLoop->getOpDatDimension(i));

    int dim = parallelLoop->getOpDatDimension (i);

    SgExpression * parameterExpression = buildIntVal (1);
    
    if (parallelLoop->getOpMapValue (i) == GLOBAL)
    {
      if (parallelLoop->getOpAccessValue (i) == READ_ACCESS)
      {
        Debug::getInstance ()->debugMessage ("OP_GBL with read access", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

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
        Debug::getInstance ()->debugMessage ("OP_GBL with write access", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        /*
         * ======================================================
         * Case of global variable accessed NOT in read mode:
         * we access the corresponding local thread variable
         * ======================================================
         */

        parameterExpression = argN_l_ref;
      }
    }
    else if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      if ( parallelLoop->getOpAccessValue (i) == INC_ACCESS)
      {
        Debug::getInstance ()->debugMessage ( "Indirect OP_DAT with increment access", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);
  
        parameterExpression = argN_l_ref;
      }
      else
      {
        Debug::getInstance ()->debugMessage ( "Indirect OP_DAT with read/write access", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);
        /*
         * ind_argX_s + ARG_maps[n+offset_b]*DIM
         */
        parameterExpression = buildAddOp(
            ind_argN_s,
            buildMultiplyOp(
                buildPntrArrRefExp(
                    argN_maps_ref,
                    buildAddOp (
                        n_ref,
                        offset_b_ref ))),
                dimN_val );
      }
    }
    else if (parallelLoop->getOpMapValue (i) == DIRECT)
    {
      Debug::getInstance ()->debugMessage ("Direct OP_DAT", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      /*
        * ARG + (n+offset_b) * DIM
        */
       parameterExpression = buildAddOp(
           argN_ref,
           buildMultiplyOp(
               buildAddOp (
                   n_ref,
                   offset_b_ref ),
               dimN_val ) );
    }

    userDeviceSubroutineParameters->append_expression (parameterExpression);
  }
  
  //needed to have the constants accessible in the user kernel
  SgExpression * global_constants_ref = buildVarRefExp ( variableDeclarations->get ( OpenCL::CPP::globalConstants ) );
  
  userDeviceSubroutineParameters->append_expression( global_constants_ref );

  return buildFunctionCallStmt (
      userSubroutineName, 
      buildVoidType (),
      userDeviceSubroutineParameters, 
      subroutineScope);
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createPointeredIncrementsOrWritesStatements ()
{
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
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      if (parallelLoop->getOpAccessValue (i) == WRITE_ACCESS
          || parallelLoop->getOpAccessValue (i) == RW_ACCESS
          || parallelLoop->getOpAccessValue (i) == INC_ACCESS)
      {
        SgExpression * argN_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatName(i)));
        SgExpression * ind_argN_size_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getIndirectionArgumentSizeName(i)));
        SgExpression * argN_maps_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getGlobalToLocalMappingName(i)));
        SgExpression * n_ref = buildVarRefExp(variableDeclarations->get(CommonVariableNames::iterationCounter1));
        SgExpression * dimN_val = buildIntVal(parallelLoop->getOpDatDimension(i));

        
        SgAssignOp * initialisationExpression = buildAssignOp (
            n_ref,
            CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalId() );
        
        SgExpression * testExpression = buildLessThanOp(
            n_ref,
            buildMultiplyOp(
                ind_argN_size_ref,
                dimN_val) );
        
        SgExpression * incrementExpression = buildPlusAssignOp(
            n_ref,
            CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalSize() );
        
        SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement1);
       
        SgForStatement * loopStatement = buildForStatement(
            initialisationExpression,
            testExpression,
            incrementExpression,
            loopBody );
        
        SgStatement * assignStatement;
        
        if (parallelLoop->getOpAccessValue (i) == INC_ACCESS)
        {
          /*
           * ======================================================
           * Increment Statement
           * ======================================================
           */
          
          assignStatement = buildPlusAssignOp(
              buildPntrArrRefExp(
                  argN_ref,
                  buildAddOp(
                      buildModOp(
                          n_ref,
                          dimN_val ),
                      buildMultiplyOp(
                          buildPntrArrRefExp(
                              argN_maps_ref,
                              buildDivideOp(
                                  n_ref,
                                  dimN_val ) ),
                          dimN_val ) ) ) );
        } else if (parallelLoop->getOpAccessValue (i) == WRITE_ACCESS
            || parallelLoop->getOpAccessValue (i) == RW_ACCESS)
        {
          /*
           * ======================================================
           * Write or Read/Write Statement
           * ======================================================
           */
          assignStatement = buildAssignOp(
              buildPntrArrRefExp(
                  argN_ref,
                  buildAddOp(
                      buildModOp(
                          n_ref,
                          dimN_val ),
                      buildMultiplyOp(
                          buildPntrArrRefExp(
                              argN_maps_ref,
                              buildDivideOp(
                                  n_ref,
                                  dimN_val ) ),
                          dimN_val ) ) ) );
        }
        
        appendStatement( assignStatement, loopBody );

        appendStatement (loopStatement, subroutineScope);

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
  using SageBuilder::buildForStatement;
  using SageInterface::appendStatement;
  
  SgExpression * col2_ref = buildVarRefExp (variableDeclarations->get (CommonVariableNames::col2));
  SgExpression * varCol = buildVarRefExp ( variableDeclarations->get (CommonVariableNames::col));
  SgExpression * varNcolor = buildVarRefExp ( variableDeclarations->get (IndirectLoop::CPP::KernelSubroutine::VariableNames::ncolor));
  
  
  SgStatement * outerLoopInitialisationExpression = buildAssignStatement(
      varCol,
      buildIntVal(0) );
  
  SgStatement * outerLoopTestExpression = buildExprStatement( buildLessThanOp(
      varCol,
      varNcolor ) );
  
  SgExpression * outerLoopIncrementExpression = buildPlusPlusOp(
      varCol );
  
  SgBasicBlock * outerLoopBody = buildBasicBlock();
 
  SgForStatement * outerLoopStatement = buildForStatement(
      outerLoopInitialisationExpression,
      outerLoopTestExpression,
      outerLoopIncrementExpression,
      outerLoopBody );
  
  appendStatement (outerLoopStatement, scope);

  SgExpression * ifGuardExpression = buildEqualityOp (
      col2_ref, 
      varCol );

  SgBasicBlock * ifBody = buildBasicBlock ();
  
  SgIfStmt * ifStatement = RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
      ifGuardExpression, 
      ifBody);
  
  appendStatement( ifStatement, outerLoopBody );

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT
        && parallelLoop->getOpAccessValue (i) == INC_ACCESS)
    {
      SgExpression * argN_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatName(i)));
      SgExpression * argN_l_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatLocalName(i)));
      SgExpression * ind_argN_s_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatSharedName(i)));
      SgExpression * ind_argN_size_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatSizeName(i)));
      SgExpression * varLocalToGlobalMapping = buildVarRefExp(variableDeclarations->get(VariableNames::getLocalToGlobalMappingName(i)));
      SgExpression * dimN_val = buildIntVal(parallelLoop->getOpDatDimension(i));
      SgExpression * varIterationCounter2 = buildVarRefExp(variableDeclarations->get(CommonVariableNames::iterationCounter2));

      
      SgStatement * innerLoopInitialisationExpression = buildExprStatement( buildAssignOp (
          varIterationCounter2,
          buildIntVal(0) ) );
      
      SgStatement * innerLoopTestExpression = buildExprStatement( buildLessThanOp(
          varIterationCounter2,
          dimN_val ) );
      
      SgExpression * innerLoopIncrementExpression = buildPlusPlusOp(
          varIterationCounter2 );
      
      SgBasicBlock * innerLoopBody = buildBasicBlock();
     
      SgForStatement * innerLoopStatement = buildForStatement(
          innerLoopInitialisationExpression,
          innerLoopTestExpression,
          innerLoopIncrementExpression,
          innerLoopBody );
      
      appendStatement (innerLoopStatement, outerLoopBody);
      
      SgStatement * innerLoopAssignmentStatement = buildPlusAssignOp(
          buildPntrArrRefExp(
              ind_argN_s_ref,
              buildAddOp(
                  varIterationCounter2,
                  buildMultiplyOp(
                      varLocalToGlobalMapping, //FIXME
                      dimN_val ) ) ),
          buildPntrArrRefExp(
              argN_l_ref,
              varIterationCounter2 ) );
      
      appendStatement( innerLoopAssignmentStatement, innerLoopBody );
      
    }
  }
  
  SgStatement * barrier = CPPOpenCLStatementsAndExpressionsBuilder::generateBarrier(subroutineScope);
  
  appendStatement( barrier, outerLoopBody );
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createIncrementAdjustmentStatements (
    SgScopeStatement * scope)
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAddOp;
  using SageInterface::appendStatement;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT
        && parallelLoop->getOpAccessValue (i) == INC_ACCESS)
    {
      SgAddOp
          * addExpression =
              buildAddOp (
                  buildVarRefExp (variableDeclarations->get (
                      CommonVariableNames::iterationCounter1)),
                  buildVarRefExp (
                      variableDeclarations->get (
                          IndirectLoop::CPP::KernelSubroutine::VariableNames::blockOffsetShared)));

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getGlobalToLocalMappingName (i))), addExpression);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getIncrementAccessMapName (i))), arrayExpression);

      appendStatement (assignmentStatement, scope);
    }
  }
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
  using SageInterface::appendStatement;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpAccessValue (i) == INC_ACCESS)
    {
      SgBasicBlock * loopBody = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get(VariableNames::getOpDatLocalName(i))),
          buildVarRefExp (variableDeclarations->get(CommonVariableNames::iterationCounter2)));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression, 
          buildIntVal (0)); //FIXME 0.0f?

      appendStatement (assignmentStatement, loopBody);
      
      SgExpression * initialisationExpression = buildAssignOp (
          buildVarRefExp(variableDeclarations->get(CommonVariableNames::iterationCounter2)),
          buildIntVal (0) );
      
      SgExpression * testExpression = buildLessThanOp(
          buildVarRefExp(variableDeclarations->get(CommonVariableNames::iterationCounter2)),
          buildIntVal( parallelLoop->getOpDatDimension(i) ) );
      
      SgExpression * incrementExpression = buildPlusPlusOp(buildVarRefExp(variableDeclarations->get(CommonVariableNames::iterationCounter2)));
      
      SgForStatement * forStatement = buildForStatement(
          initialisationExpression,
          testExpression,
          incrementExpression,
          loopBody );

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
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildWhileStmt;
  using SageInterface::appendStatement;
  
  SgExpression * n_ref = buildVarRefExp (variableDeclarations->get (CommonVariableNames::iterationCounter1));
  SgExpression * nelems2_ref = buildVarRefExp (variableDeclarations->get(IndirectLoop::CPP::KernelSubroutine::VariableNames::nelems2));
  SgExpression * col2_ref = buildVarRefExp (variableDeclarations->get (CommonVariableNames::col2));
  SgExpression * nelem_ref = buildVarRefExp (variableDeclarations->get (IndirectLoop::CPP::KernelSubroutine::VariableNames::nelem));
  SgExpression * offset_b_ref = buildVarRefExp ( variableDeclarations->get ( IndirectLoop::CPP::KernelSubroutine::VariableNames::blockOffsetShared));

  /*
   * ======================================================
   * for loop construction
   * ======================================================
   */

  SgStatement * initialisationExpression = buildAssignStatement (
      n_ref, 
      CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalId() );

  SgStatement * testExpression = buildExprStatement( buildLessThanOp (
      n_ref,
      nelems2_ref ) );
  
  SgExpression * incrementExpression = buildPlusAssignOp(
      n_ref,
      CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalSize());
  
  SgBasicBlock * loopBody = buildBasicBlock ();

  SgWhileStmt * loopStatement = buildForStatement( 
      initialisationExpression,
      testExpression,
      incrementExpression,
      loopBody );

  appendStatement (loopStatement, subroutineScope);
  /*
   * ======================================================
   * Initialise inner loop counter
   * ======================================================
   */

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      col2_ref,
      buildIntVal (-1));

  appendStatement (assignmentStatement2, loopBody);


  /*
   * if guard construction
   */
  SgExpression * ifGuardExpression = buildLessThanOp (
      n_ref,
      nelem_ref );

  SgBasicBlock * ifBody = buildBasicBlock ();

  SgIfStmt * ifStatement = RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
      ifGuardExpression, 
      ifBody);
  
  appendStatement (ifStatement, loopBody);
  
  createInitialiseLocalOpDatStatements (ifBody);

  appendStatement (createUserSubroutineCallStatement (), ifBody);
  
  SgStatement * assignmentStatement3 = buildAssignStatement(
      col2_ref,
      buildPntrArrRefExp(
          colors_ref,
          buildAddOp(
              n_ref,
              offset_b_ref ) ) );

  appendStatement (assignmentStatement3, ifBody);

  createIncrementAdjustmentStatements (loopBody);

  createInnerExecutionLoopStatements (loopBody);
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createAutoSharedWhileLoopStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildWhileStmt;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageInterface::appendStatement;

  SgExpression * n_ref = buildVarRefExp (variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::setElementCounter));
  
  unsigned int pindOffsOffset = 0;
  
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * ind_argN_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getOpIndirectionName(i)));
    SgExpression * ind_argN_map_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getIncrementAccessMapName(i)));
    SgExpression * ind_argN_maps_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getLocalToGlobalMappingName(i)));
    SgExpression * ind_argN_s_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getOpIndirectionSharedName(i)));
    SgExpression * ind_argN_size_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getIndirectionArgumentSizeName(i)));
    SgExpression * dimN_val = buildIntVal(parallelLoop->getOpDatDimension(i));
    
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      
      SgStatement * initialisationExpression = buildExprStatement( buildAssignOp (
          n_ref,
          CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalId() ) );
      
      SgStatement * testExpression = buildExprStatement( buildLessThanOp(
          n_ref,
          buildMultiplyOp(
              ind_argN_size_ref,
              dimN_val ) ) );
      
      SgExpression * incrementExpression = buildPlusAssignOp(
          n_ref,
          CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalSize() );
      

      SgBasicBlock * loopBody = buildBasicBlock();
      
      SgForStatement * forStatement = buildForStatement(
          initialisationExpression,
          testExpression,
          incrementExpression,
          loopBody );
      
      appendStatement( forStatement, subroutineScope );
      
      SgStatement * assignStatement;

      if (parallelLoop->getOpAccessValue (i) != INC_ACCESS)
      {
        assignStatement = buildAssignStatement(
            buildPntrArrRefExp(
                ind_argN_s_ref,
                n_ref ),
            buildIntVal(0) ); //FIXME 0.0f?
      }
      else
      {
        assignStatement = buildAssignStatement(
            buildPntrArrRefExp(
                ind_argN_s_ref,
                n_ref ),
            buildPntrArrRefExp(
                ind_argN_ref,
                buildAddOp(
                    buildModOp(
                        n_ref,
                        dimN_val ),
                    buildMultiplyOp(
                        buildPntrArrRefExp(
                            ind_argN_map_ref,
                            buildDivideOp(
                                n_ref,
                                dimN_val ) ),
                        dimN_val ) ) ) ); 
      }
      
      appendStatement( assignStatement, loopBody );
    }
  }

  /*
   * ======================================================
   * All threads must synchronize before kernel execution
   * can proceed
   * ======================================================
   */

  SgStatement * subroutineCall = buildExprStatement ( CPPOpenCLStatementsAndExpressionsBuilder::generateBarrier() );

  appendStatement (subroutineCall, subroutineScope);
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
  using SageBuilder::buildIntVal;
  using SageInterface::appendStatement;

  
  SgExpression * nbytes_ref = buildVarRefExp ( variableDeclarations->get ( CommonVariableNames::numberofBytes ) );
  SgExpression * shared_ref = buildVarRefExp ( variableDeclarations->get ( CommonVariableNames::autoshared ) );

  bool firstInitialization = true;
  unsigned int previous_OP_DAT_Location;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * ind_argN_s_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getOpIndirectionSharedName(i)));
    SgExpression * ind_argN_size_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getIndirectionArgumentSizeName(i)));
    SgExpression * dimN_val = buildIntVal(parallelLoop->getOpDatDimension(i));
    
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgExprStatement * incrementStatement;
      
      if (firstInitialization)
      {
        firstInitialization = false;

        incrementStatement = buildAssignStatement (
            nbytes_ref, 
            buildIntVal(0));
      }
      else
      {
        incrementStatement = buildExprStatement( buildPlusAssignOp(
            nbytes_Reference, 
            buildFunctionCallExp(
                buildFunctionRefExp( "ROUND_UP" ),
                buildExprListExp(
                    buildMultiplyOp(
                        ind_argN_size_ref,
                        buildMultiplyOp(
                            buildSizeOfOp(
                                buildFloatType() ),
                            dimN_val ) ) ) ) ) );
      }
      
      appendStatement ( incrementStatement, subroutineScope );
      
      SgExprStatement * assignmentStatement = buildAssignStatement (
          ind_argN_s_ref, 
          buildCastExp(
              buildAddOp(
                  shared_ref,
                  nbytes_ref ),
              buildFloatType() ) ); //FIXME (__local TYPE *)
      
      appendStatement( assignmentStatement, subroutineScope );
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
  
  SgExpression * blockId_ref = buildVarRefExp (variableDeclarations->get ( IndirectLoop::CPP::KernelSubroutine::VariableNames::blockID ) );
  SgExpression * blkmap_ref = buildVarRefExp (variableDeclarations->get ( PlanFunction::CPP::pblkMap ) );
  SgExpression * block_offset_ref = buildVarRefExp( variableDeclarations->get (PlanFunction::CPP::blockOffset) );
  SgExpression * nelem_ref = buildVarRefExp( variableDeclarations-get( IndirectLoop::CPP::KernelSubroutine::VariableNames::nelem ) );
  SgExpression * nelems_ref = buildVarRefExp( variableDeclarations-get( PlanFunction::CPP::pnelems ) );
  SgExpression * offset_b_ref = buildVarRefExp( variableDeclarations-get( IndirectLoop::CPP::KernelSubroutine::VariableNames::blockOffsetShared ) );
  SgExpression * offset_ref = buildVarRefExp( variableDeclarations-get( PlanFunction::CPP::poffset ) );
  SgExpression * nelems2_ref = buildVarRefExp ( variableDeclarations-get( IndirectLoop::CPP::KernelSubroutine::VariableNames::nelems2 ) );
  SgExpression * ncolor_ref = buildVarRefExp ( variableDeclarations-get( IndirectLoop::CPP::KernelSubroutine::VariableNames::ncolor) );
  SgExpression * ncolors_ref = buildVarRefExp ( variableDeclarations-get( PlanFunction::CPP::pnthrcol ) );
  
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

  SgEqualityOp * ifGuardExpression = buildEqualityOp (
      CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalId(),
      buildIntVal (0));
  
  SgIfStmt * ifStatement = RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
      ifGuardExpression, 
      ifBlock);

  appendStatement (ifStatement, subroutineScope);

  /*
   * ======================================================
   * 1st statement: blockId = blkmap[ get_group_id(0) + block_offset ]
   * ======================================================
   */

  SgStatement * statement1 = buildAssignStatement (
      blockId_ref,
      buildPntrArrRefExp (
          blkmap_ref,
          buildAddOp (
              CPPOpenCLStatementsAndExpressionsBuilder::generateGetGroupId(),
              block_offset_ref)));
  
  appendStatement( statement1, ifBlock );

  /*
   * ======================================================
   * 2nd statement: nelem = nelems[ blockId ]
   * ======================================================
   */
  
  SgStatement * statement2 = buildAssignStatement(
      nelem_ref,
      buildPntrArrRefExp(
          nelems_ref,
          blockId_ref ) );
  
  appendStatement( statement2, ifBlock );

  /*
   * ======================================================
   * 3rd statement: offset_b = offset[ blockId ]
   * ======================================================
   */

  SgStatement * statement3 = buildAssignStatement(
      offset_b_ref,
      buildPntrArrRefExp(
          offset_ref,
          blockId_ref ) );
  
  appendStatement( statement3, ifBlock );

  /*
   * ======================================================
   * 4th statement: assignment of nelems2
   * ======================================================
   */
  
  SgStatement * statement4 = buildAssignStatement(
      nelems2_ref,
      buildMultiplyOp(
          CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalSize(),
          buildAddOp(
              buildIntVal(1),
              buildDivideOp(
                  buildSubtractOp(
                      nelem_ref,
                      buildIntVal(1) ),
                  CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalSize() ) ) ) );
  
  appendStatement( statement4, ifBlock );

  /*
   * ======================================================
   * 5th statement: assignment of ncolor
   * ======================================================
   */
  
  SgStatement * statement5 = buildAssignStatement(
      ncolor_ref,
      buildPntrArrRefExp(
          ncolors_ref,
          blockId_ref ) );
  
  appendStatement( statement5, ifBlock );


  /*
   * ======================================================
   * assignment of ind_arg_size variables
   * ======================================================
   */
  
  SgExpression * ind_arg_sizes_ref = buildVarRefExp ( variableDeclarations->get (PlanFunction::CPP::pindSizes) );
  SgExpression * ind_arg_offs_ref = buildVarRefExp ( variableDeclarations->get (PlanFunction::CPP::pindOffs) );

  unsigned int pindSizesArrayOffset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * ind_argN_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getOpIndirectionName(i)));
    SgExpression * ind_argN_map_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getIncrementAccessMapName(i)));
    SgExpression * ind_argN_maps_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getLocalToGlobalMappingName(i)));
    SgExpression * ind_argN_s_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getOpIndirectionSharedName(i)));
    SgExpression * ind_argN_size_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getIndirectionArgumentSizeName(i)));
    SgExpression * dimN_val = buildIntVal(parallelLoop->getOpDatDimension(i));
    
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgStatement * statement6 = buildAssignStatement(
          ind_argN_size_ref,
          buildPntrArrRefExp(
              ind_arg_sizes_ref,
              buildAddOp(
                  buildIntVal(pindSizesArrayOffset),
                  buildMultiplyOp(
                      blockId_ref,
                      dimN_val ) ) ) );
      
      appendStatement( statement6, ifBlock );

      ++pindSizesArrayOffset;
    }
  }
  
  /*
   * ======================================================
   * assignment of ind_arg_map variables
   * ======================================================
   */
  
  unsigned int pindSizesArrayOffset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * ind_argN_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getOpIndirectionName(i)));
    SgExpression * ind_argN_map_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getIncrementAccessMapName(i)));
    SgExpression * ind_argN_maps_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getLocalToGlobalMappingName(i)));
    SgExpression * ind_argN_s_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getOpIndirectionSharedName(i)));
    SgExpression * ind_argN_size_ref = buildVarRefExp (variableDeclarations->get (VariableNames::getIndirectionArgumentSizeName(i)));
    SgExpression * dimN_val = buildIntVal(parallelLoop->getOpDatDimension(i));
    
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      SgStatement * statement7 = buildAssignStatement(
          ind_argN_map_ref,
          buildAddOp(
              ind_argN_maps_ref,
              buildPntrArrRefExp(
                  ind_arg_offs_ref,
                  buildAddOp(
                      buildIntVal(pindSizesArrayOffset),
                      buildMultiplyOp(
                          blockId_ref,
                          dimN_val ) ) ) ) );
      
      appendStatement( statement7, ifBlock );

      ++pindSizesArrayOffset;
    }
  }


  /*
   * ======================================================
   * All threads must synchronize before kernel execution
   * can proceed. Add the statement
   * ======================================================
   */


  SgFunctionCallExp * subroutineCall = buildExprStatement ( CPPOpenCLStatementsAndExpressionsBuilder::generateBarrier() );

  appendStatement (subroutineCall, subroutineScope);
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createPlanFormalParameterDeclarations ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using std::string;
  using std::vector;

  vector<string> intPtrVariables;

  intPtrVariables.push_back (PlanFunction::CPP::pindSizes);

  intPtrVariables.push_back (PlanFunction::CPP::pindOffs);

  intPtrVariables.push_back (PlanFunction::CPP::pblkMap);

  intPtrVariables.push_back (PlanFunction::CPP::poffset);

  intPtrVariables.push_back (PlanFunction::CPP::pnelems);

  intPtrVariables.push_back (PlanFunction::CPP::pnthrcol);

  intPtrVariables.push_back (PlanFunction::CPP::pthrcol);

  for (vector <string>::const_iterator it = intPtrVariables.begin (); 
      it != intPtrVariables.end (); 
      ++it)
  {
#if 0
    SgIntVal * lowerBoundExpression = buildIntVal (0);

    SgVarRefExp * argsSizesReference = buildVarRefExp (
        variableDeclarations->get (
            VariableNames::getDataSizesVariableDeclarationName (
                userSubroutineName)));

    SgVarRefExp * fieldReference = buildVarRefExp (
        dataSizesDeclaration->getFieldDeclarations ()->get (*it + "Size"));

    SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
        fieldReference);

    SgSubtractOp * upperBoundExpression = buildSubtractOp (
        fieldSelectionExpression, buildIntVal (1));
#endif

    variableDeclarations->add (
        *it,
        CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            *it,
            buildPointerType( buildIntType() ), 
            subroutineScope,
            formalParameters, 
            1, 
            DEVICE ) );
  }

  vector<string> intVariables;

  intVariables.push_back (PlanFunction::CPP::blockOffset);

  for (vector <string>::const_iterator it = intVariables.begin (); it
      != intVariables.end (); ++it)
  {
    variableDeclarations->add (
        *it,
        CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            *it, 
            buildIntType(), 
            subroutineScope,
            formalParameters ) );
  }
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createOpDatFormalParameterDeclarations ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntVal;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters",
      Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      /*
       * ======================================================
       * The base type of an OP_DAT must always be an array
       * ======================================================
       */

      SgType * opdatType = parallelLoop->getOpDatType (i);
      
      SgArrayType * isArrayType = isSgArrayType (opdatType);

      SgType * opDatBaseType = isArrayType->get_base_type ();

      SgPointerType * pointerType = buildPointerType( opDatBaseType );

      variableDeclarations->add (
          VariableNames::getOpDatName(i),
          CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter(
              VariableNames::getOpDatName(i),
              pointerType, 
              subroutineScope, 
              formalParameters, 
              1,
              DEVICE ) );

      variableDeclarations->add (
          VariableNames::getLocalToGlobalMappingName (i),
          CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter(
              VariableNames::getLocalToGlobalMappingName (i),
              buildPointerType( buildIntType() ),
              subroutineScope, 
              formalParameters, 
              1, 
              DEVICE ) );
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) == INDIRECT)
    {

      variableDeclarations->add (
          VariableNames::getGlobalToLocalMappingName (i),
          CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              VariableNames::getGlobalToLocalMappingName (i),
              buildPointerType( buildShortType() ),
              subroutineScope,
              formalParameters, 
              1, 
              DEVICE));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && (parallelLoop->getOpMapValue (i) == DIRECT
            || parallelLoop->getOpMapValue (i) == GLOBAL))
    {
      string const variableName = VariableNames::getOpDatName (i);
      
      SgType * opDatType = parallelLoop->getOpDatType(i);

      SgArrayType * isArrayType = isSgArrayType (opDatType);

      SgType * opDatBaseType = isArrayType->get_base_type ();
       
      SgPointerType * pointerType = buildPointerType( opDatBaseType );

      variableDeclarations->add (
          variableName,
          CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName,
              pointerType, 
              subroutineScope, 
              formalParameters, 
              1,
              DEVICE ) );
    }
  }
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createStatements ()
{
  createThreadZeroStatements ();

  createInitialiseLocalVariablesStatements (); //FIXME: move to thread 0!

  createAutoSharedWhileLoopStatements ();

  createExecutionLoopStatements ();

  createPointeredIncrementsOrWritesStatements ();
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  using boost::lexical_cast;
  using std::string;
  using std::vector;

  // createLocalThreadDeclarations (); //XXX ???

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName = VariableNames::getNumberOfBytesVariableName(i);

    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      variableDeclarations->add (
          variableName,
          CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, 
              buildIntType(),
              subroutineScope ) );

      positionOfNbytes[parallelLoop->getOpDatVariableName (i)] = i;
    }
    else
    {
      variableDeclarations->add (
          variableName, 
          variableDeclarations->get (
              VariableNames::getNumberOfBytesVariableName(
              positionOfNbytes[parallelLoop->getOpDatVariableName (i)] ) ) );
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = VariableNames::getRoundUpVariableName (i);

      variableDeclarations->add( variableName,
          CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, 
              buildIntType(),
              subroutineScope ) );
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpAccessValue (i) == INC_ACCESS)
    {
      string const variableName = VariableNames::getIncrementAccessMapName (i);

      variableDeclarations->add( variableName,
          CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, 
              buildIntType(),
              subroutineScope ) );
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == INDIRECT)
    {
      string const variableName =
          VariableNames::getIndirectionArgumentSizeName (i);

      variableDeclarations->add (variableName,
          CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, 
              buildIntType (),
              subroutineScope, 
              1, 
              SHARED));
    }
  }

  vector <string> sharedFourByteIntegerVariables;

  sharedFourByteIntegerVariables.push_back (
      IndirectLoop::CPP::KernelSubroutine::VariableNames::blockID);

  sharedFourByteIntegerVariables.push_back (
      IndirectLoop::CPP::KernelSubroutine::VariableNames::blockOffsetShared);

  sharedFourByteIntegerVariables.push_back (
      IndirectLoop::CPP::KernelSubroutine::VariableNames::ncolor);

  sharedFourByteIntegerVariables.push_back (
      IndirectLoop::CPP::KernelSubroutine::VariableNames::nelem);

  sharedFourByteIntegerVariables.push_back (
      IndirectLoop::CPP::KernelSubroutine::VariableNames::nelems2);

  for (vector <string>::const_iterator it =
      sharedFourByteIntegerVariables.begin (); it
      != sharedFourByteIntegerVariables.end (); ++it)
  {
    variableDeclarations->add( *it,
        CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
            *it,
            buildIntType (), 
            subroutineScope, 
            1,
            SHARED ) );
  }

  /*
  variableDeclarations->add (CommonVariableNames::autoshared,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::autoshared,
          buildPointerType( buildFloatType() ),
          subroutineScope, 
          1, 
          SHARED)); */

  vector <string> integerVariables;

  integerVariables.push_back (CommonVariableNames::col);

  integerVariables.push_back (CommonVariableNames::col2);

  integerVariables.push_back (CommonVariableNames::iterationCounter1);

  integerVariables.push_back (CommonVariableNames::iterationCounter2);

  integerVariables.push_back (CommonVariableNames::upperBound);

  integerVariables.push_back (
      IndirectLoop::CPP::KernelSubroutine::VariableNames::moduloResult);

  integerVariables.push_back (
      IndirectLoop::CPP::KernelSubroutine::VariableNames::moduled);

  integerVariables.push_back (
      IndirectLoop::CPP::KernelSubroutine::VariableNames::nbytes);

  for (vector <string>::const_iterator it = integerVariables.begin (); it
      != integerVariables.end (); ++it)
  {
    variableDeclarations ->add (*it,
        CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildIntType(), 
            subroutineScope ) );
  }
}

void
CPPOpenCLKernelSubroutineIndirectLoop::createFormalParameterDeclarations ()
{
  createOpDatFormalParameterDeclarations ();

  createPlanFormalParameterDeclarations ();
  
  /*
   * ======================================================
   * Shared memory formal parameter
   * ======================================================
   */

  variableDeclarations->add (
      CommonVariableNames::autoshared,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          CommonVariableNames::autoshared,
          buildPointerType( buildCharType() ), //TODO: char* vs float*
          subroutineScope,
          formalParameters,
          1,
          SHARED ) );
  
  /*
   * ======================================================
   * global constants formal parameter
   * ======================================================
   */
  
  variableDeclarations->add (
      CommonVariableNames::,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OpenCL::CPP::globalConstants,
          buildPointerType( buildCharType() ), //TODO: fix type
          subroutineScope,
          formalParameters,
          1,
          CONSTANT ) );
  
  
}

CPPOpenCLKernelSubroutineIndirectLoop::CPPOpenCLKernelSubroutineIndirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    CPPParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    CPPReductionSubroutines * reductionSubroutines,
    CPPOpenCLDataSizesDeclarationIndirectLoop * dataSizesDeclaration,
    CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
  CPPOpenCLKernelSubroutine (subroutineName, userSubroutineName,
      parallelLoop, moduleScope, reductionSubroutines,
      opDatDimensionsDeclaration), dataSizesDeclaration (dataSizesDeclaration)
{
  Debug::getInstance ()->debugMessage ("<Kernel, Indirect, OpenCL>", Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
