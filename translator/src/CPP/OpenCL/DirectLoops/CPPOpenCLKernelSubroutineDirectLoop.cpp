#include <CPPOpenCLKernelSubroutineDirectLoop.h>
//#include <CPPTypesBuilder.h>
#include <CPPOpenCLStatementsAndExpressionsBuilder.h>
//#include <CPPOpenCLReductionSubroutine.h>
#include <RoseHelper.h>
#include <Debug.h>
#include <CPPOpenCLReductionSubroutine.h>
using namespace SageBuilder; //TODO: remove
/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
CPPOpenCLKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
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
  
  Debug::getInstance ()->debugMessage ( "Creating call to user device subroutine", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
  
  SgExpression * n_ref = buildVarRefExp(variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::setElementCounter));
  SgExpression * offset_ref = buildVarRefExp ( variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock));

  SgExprListExp * userDeviceSubroutineParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * argN_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatName(i)));
    SgExpression * argN_l_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatLocalName(i)));
    SgExpression * dimN_val = buildIntVal(parallelLoop->getOpDatDimension(i));

    int dim = parallelLoop->getOpDatDimension (i);
    
    SgExpression * parameterExpression = buildIntVal (1);
    
    if (parallelLoop->getOpMapValue (i) == GLOBAL)
    {
      if (parallelLoop->getOpAccessValue (i) == READ_ACCESS)
      {
        /*
         * ======================================================
         * Case of global variable accessed in read mode:
         * we directly access the device variable, by
         * passing the kernel the variable name in positions
         * 0:argSize%<devVarName>-1
         * ======================================================
         */
        parameterExpression = argN_ref;
      }
      else
      {
        /*
         * ======================================================
         * Case of global variable accessed NOT in read mode:
         * we access the corresponding local thread variable
         * ======================================================
         */
        parameterExpression = argN_l_ref;
      }
    }
    else if (parallelLoop->getOpMapValue (i) == DIRECT)
    {
      if (parallelLoop->getNumberOfIndirectOpDats () > 0)
      {
        parameterExpression = buildAddOp(
            argN_ref,
            buildMultiplyOp(
                buildAddOp(
                    n_ref,
                    offset_ref ),
                dimN_val ) );
      }
      else if (dim == 1)
      {
        parameterExpression = buildAddOp(
            argN_ref,
            n_ref );
      }
      else
      {
        parameterExpression = argN_l_ref;
      }
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

SgBasicBlock *
CPPOpenCLKernelSubroutineDirectLoop::createStageInFromDeviceMemoryToLocalThreadVariablesStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildNullExpression;
  using SageBuilder::buildForStatement;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildPlusPlusOp;
  using SageInterface::appendStatement;

  SgExpression * arg_s_ref = buildVarRefExp(variableDeclarations->get(CommonVariableNames::argShared));
  SgExpression * tid_ref = buildVarRefExp(variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::threadIDModulus));
  SgExpression * m_ref = buildVarRefExp(variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::dataPerElementCounter));
  SgExpression * nelems_ref = buildVarRefExp( variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::remainingElements ) );
  SgExpression * offset_ref = buildVarRefExp ( variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock));

  SgBasicBlock * outerBlock = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * argN_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatName(i)));
    SgExpression * argN_l_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatLocalName(i)));
    SgExpression * dimN_val = buildIntVal(parallelLoop->getOpDatDimension(i));
    
    if (parallelLoop->getOpMapValue (i) != GLOBAL
        && parallelLoop->getOpAccessValue (i) != WRITE_ACCESS
        && parallelLoop->getOpDatDimension (i) != 1)
    {
      /* ======================================================
       * for ( m=0; m<dimN; m++ ) {
       *        arg_s[tid + m * nelems] = argN[tid + m * nelems + offset * dimN]
       * }
       * ======================================================
       */
      SgStatement * initialisationExpression1 = buildExprStatement( buildAssignOp (
          m_ref,
          buildIntVal(0) ) );
      
      SgStatement * testExpression1 = buildExprStatement( buildLessThanOp(
          m_ref,
          dimN_val ) );
      
      SgExpression * incrementExpression1 = buildPlusPlusOp(
          m_ref );

      SgBasicBlock * loopBody1 = buildBasicBlock();
      
      SgForStatement * forStatement1 = buildForStatement(
          initialisationExpression1,
          testExpression1,
          incrementExpression1,
          loopBody1 );
      
      appendStatement( forStatement1, outerBlock );
      
      SgStatement * assignStatement1 = buildAssignStatement(
          buildPntrArrRefExp(
              arg_s_ref,
              buildAddOp(
                  tid_ref,
                  buildMultiplyOp(
                      m_ref,
                      nelems_ref ) ) ),
          buildPntrArrRefExp(
              argN_ref,
              buildAddOp(
                  tid_ref,
                  buildAddOp(
                      buildMultiplyOp(
                          m_ref,
                          nelems_ref ),
                      buildMultiplyOp(
                          offset_ref,
                          dimN_val ) ) ) ) );
      
      appendStatement( assignStatement1, forStatement1 );
      
      /* ======================================================
       * for ( m=0; m<dimN; m++ ) {
       *        argN_l[m] = arg_s[m + tid * dimN]
       * }
       * ======================================================
       */
      
      SgStatement * initialisationExpression2 = buildExprStatement( buildAssignOp (
          m_ref,
          buildIntVal(0) ) );
      
      SgStatement * testExpression2 = buildExprStatement( buildLessThanOp(
          m_ref,
          dimN_val ) );
      
      SgExpression * incrementExpression2 = buildPlusPlusOp(
          m_ref );

      SgBasicBlock * loopBody2 = buildBasicBlock();
      
      SgForStatement * forStatement2 = buildForStatement(
          initialisationExpression2,
          testExpression2,
          incrementExpression2,
          loopBody2 );
      
      appendStatement( forStatement2, outerBlock );
      
      SgStatement * assignStatement2 = buildAssignStatement(
          buildPntrArrRefExp(
              argN_l_ref,
              m_ref ),
          buildPntrArrRefExp(
              arg_s_ref,
              buildAddOp(
                  m_ref,
                  buildMultiplyOp(
                      tid_ref,
                      dimN_val ) ) ) );
      
      appendStatement( assignStatement2, forStatement2 );
      
    }
  }

  return outerBlock;
}

SgBasicBlock *
CPPOpenCLKernelSubroutineDirectLoop::createStageOutFromLocalThreadVariablesToDeviceMemoryStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildNullExpression;
  using SageInterface::appendStatement;

  SgExpression * arg_s_ref = buildVarRefExp(variableDeclarations->get(CommonVariableNames::argShared));
  SgExpression * tid_ref = buildVarRefExp(variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::threadIDModulus));
  SgExpression * m_ref = buildVarRefExp(variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::dataPerElementCounter));
  SgExpression * nelems_ref = buildVarRefExp( variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::remainingElements ) );
  SgExpression * offset_ref = buildVarRefExp ( variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock));

  SgBasicBlock * outerBlock = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * argN_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatName(i)));
    SgExpression * argN_l_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatLocalName(i)));
    SgExpression * dimN_val = buildIntVal(parallelLoop->getOpDatDimension(i));
    
    if (parallelLoop->getOpMapValue (i) != GLOBAL
        && parallelLoop->getOpAccessValue (i) != WRITE_ACCESS
        && parallelLoop->getOpDatDimension (i) != 1)
    {
      /* ======================================================
       * for ( m=0; m<dimN; m++ ) {
       *        arg_s[m + tid * dimN] = argN_l[m]
       * }
       * ======================================================
       */
      
      SgStatement * initialisationExpression1 = buildExprStatement( buildAssignOp (
          m_ref,
          buildIntVal(0) ) );
      
      SgStatement * testExpression1 = buildExprStatement( buildLessThanOp(
          m_ref,
          dimN_val ) );
      
      SgExpression * incrementExpression1 = buildPlusPlusOp(
          m_ref );

      SgBasicBlock * loopBody1 = buildBasicBlock();
      
      SgForStatement * forStatement1 = buildForStatement(
          initialisationExpression1,
          testExpression1,
          incrementExpression1,
          loopBody1 );
      
      appendStatement( forStatement1, outerBlock );
      
      SgStatement * assignStatement1 = buildAssignStatement(
          buildPntrArrRefExp(
              arg_s_ref,
              buildAddOp(
                  m_ref,
                  buildMultiplyOp(
                      tid_ref,
                      dimN_val ) ) ),
          buildPntrArrRefExp(
              argN_l_ref,
              m_ref ) );
      
      appendStatement( assignStatement1, forStatement1 );
      
      /* ======================================================
       * for ( m=0; m<dimN; m++ ) {
       *        argN[tid + m * nelems + offset * dimN] = arg_s[tid + m * nelems]
       * }
       * ======================================================
       */
      SgStatement * initialisationExpression2 = buildExprStatement( buildAssignOp (
          m_ref,
          buildIntVal(0) ) );
      
      SgStatement * testExpression2 = buildExprStatement( buildLessThanOp(
          m_ref,
          dimN_val ) );
      
      SgExpression * incrementExpression2 = buildPlusPlusOp(
          m_ref );

      SgBasicBlock * loopBody2 = buildBasicBlock();
      
      SgForStatement * forStatement2 = buildForStatement(
          initialisationExpression2,
          testExpression2,
          incrementExpression2,
          loopBody2 );
      
      appendStatement( forStatement2, outerBlock );
      
      SgStatement * assignStatement2 = buildAssignStatement(
          buildPntrArrRefExp(
              argN_ref,
              buildAddOp(
                  tid_ref,
                  buildAddOp(
                      buildMultiplyOp(
                          m_ref,
                          nelems_ref ),
                      buildMultiplyOp(
                          offset_ref,
                          dimN_val ) ) ) ),
          buildPntrArrRefExp(
              arg_s_ref,
              buildAddOp(
                  tid_ref,
                  buildMultiplyOp(
                      m_ref,
                      nelems_ref ) ) ) );
      
      appendStatement( assignStatement2, forStatement2 );
      
    }
  }

  return outerBlock;
}

void
CPPOpenCLKernelSubroutineDirectLoop::createExecutionLoopStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildIntType;
  using SageInterface::appendStatement;
  
  SgExpression * set_size_ref = buildVarRefExp (variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::setSize));
  SgExpression * nelems_ref = buildVarRefExp( variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::remainingElements ) );
  SgExpression * offset_ref = buildVarRefExp ( variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock));
  SgExpression * tid_ref = buildVarRefExp(variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::threadIDModulus));
  SgExpression * n_ref = buildVarRefExp (variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::setElementCounter));

  /* ======================================================
   * for ( n = get_global_id(0); n < set_size; n++ )
   * ======================================================
   */
  
  SgStatement * initialisationExpression = buildExprStatement( buildAssignOp (
      n_ref,
      CPPOpenCLStatementsAndExpressionsBuilder::generateGetGlobalId() ) );
  
  SgStatement * testExpression = buildExprStatement( buildLessThanOp(
      n_ref,
      set_size_ref ) );
  
  SgExpression * incrementExpression = buildPlusPlusOp(
      n_ref );
  

  SgBasicBlock * loopBody = buildBasicBlock();
  
  SgForStatement * forStatement = buildForStatement(
      initialisationExpression,
      testExpression,
      incrementExpression,
      loopBody );
  
  appendStatement( forStatement, subroutineScope );
  
  /* ======================================================
   * offset = n - tid
   * ======================================================
   */
  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      offset_ref,
      buildSubtractOp (
            n_ref,
            tid_ref ) );

  appendStatement (assignmentStatement1, loopBody);
  
  /* ======================================================
   * nelems = MIN( OP_WARPSIZE, set_size - offset )
   * ======================================================
   */
  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      nelems_ref,
      buildFunctionCallExp (
            "MIN", //FIXME
            buildIntType(),
            buildExprListExp (
                buildVarRefExp ( variableDeclarations->get (DirectLoop::CPP::KernelSubroutine::warpSize)),
                buildSubtractOp (
                    set_size_ref, 
                    offset_ref ) ) ) );


  appendStatement (assignmentStatement2, loopBody);
  
  appendStatement (
      createStageInFromDeviceMemoryToLocalThreadVariablesStatements (),
      loopBody);

  appendStatement (
      createUserSubroutineCallStatement (), 
      loopBody);

  appendStatement (
      createStageOutFromLocalThreadVariablesToDeviceMemoryStatements (),
      loopBody);

}

void
CPPOpenCLKernelSubroutineDirectLoop::createAutoSharedDisplacementInitialisationStatement ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;
  
  SgExpression * shared_ref = buildVarRefExp(variableDeclarations->get(CommonVariableNames::autoshared));
  SgExpression * arg_s_ref = buildVarRefExp(variableDeclarations->get(CommonVariableNames::argShared));
  SgExpression * offset_s_ref = buildVarRefExp (variableDeclarations->get (DirectLoop::CPP::KernelSubroutine::warpScratchpadSize));
  
  SgStatement * assignStatement = buildAssignStatement(
      arg_s_ref,
      buildCastExp(
          buildAddOp(
              shared_ref,
              buildMultiplyOp(
                  offset_s_ref,
                  buildDivideOp(
                      CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalId(),
                      buildVarRefExp (variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::warpSize) ) ) ) ),
          buildPointerType( 
              buildFloatType() ) ) ); //FIXME (__local float *)
  
  appendStatement ( assignStatement, subroutineScope);

  //TODO: what about this?
  /*
  if (parallelLoop->getSizeOfOpDat () == 8)
  {
    SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression,
        buildIntVal (8));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        buildVarRefExp (variableDeclarations->get (
            DirectLoop::CPP::KernelSubroutine::autosharedDisplacement)),
        divideExpression2);

    appendStatement (assignmentStatement, subroutineScope);
  }
  else
  {
    SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression,
        buildIntVal (4));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        buildVarRefExp (variableDeclarations->get (
            DirectLoop::CPP::KernelSubroutine::autosharedDisplacement)),
        divideExpression2);

    appendStatement (assignmentStatement, subroutineScope);
  }*/
}

void
CPPOpenCLKernelSubroutineDirectLoop::createThreadIDInitialisationStatement ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;
  
  SgExpression * tid_ref = buildVarRefExp(variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::threadIDModulus));

  
  /*
   * ======================================================
   * tid = get_local_id(0) % OP_WARP_SIZE
   * ======================================================
   */ 

  SgExprStatement * assignmentStatement = buildAssignStatement (
      tid_ref,
      buildModOp(
          CPPOpenCLStatementsAndExpressionsBuilder::generateGetGlobalSize(),
          buildVarRefExp (variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::warpSize) ) ) );

  appendStatement (assignmentStatement, subroutineScope);
}

void
CPPOpenCLKernelSubroutineDirectLoop::createOpDatFormalParameterDeclarations ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildDotExp;
  using std::string;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      /*
       * ======================================================
       * Obtain the base type of the OP_DAT argument
       * ======================================================
       */
      
      string const & variableName = VariableNames::getOpDatName(i);

      SgType * opDatType = parallelLoop->getOpDatType(i);

      SgArrayType * isArrayType = isSgArrayType (opDatType);

      SgType * opDatBaseType = isArrayType->get_base_type ();
      
      SgPointerType * pointerType = buildPointerType( opDatBaseType );

      variableDeclarations->add(
          variableName,
          CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName, 
              pointerType, 
              subroutineScope, 
              formalParameters, 
              1,
              DEVICE));
    }
  }
}

void
CPPOpenCLKernelSubroutineDirectLoop::createInitialiseLocalThreadVariablesStatements ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPlusPlusOp;
  using SageInterface::appendStatement;


  SgExpression * d_ref = buildVarRefExp (variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::setElementCounter));
  /*
   * ======================================================
   * We essentially do a similar thing to the one we make
   * when we declare local thread variables
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * argN = buildVarRefExp(variableDeclarations->get ( VariableNames::getOpDatName(i) ) );
    SgExpression * argN_l = buildVarRefExp(variableDeclarations->get ( VariableNames::getOpDatLocalName (i) ) );
    SgExpression * dimN_val = buildIntVal(parallelLoop->getOpDatDimension(i));
    
    if (parallelLoop->getOpMapValue (i) == GLOBAL
        && parallelLoop->getOpAccessValue (i) != READ_ACCESS)
    {
      /*
       * ======================================================
       * for ( d=0; d<dimN; d++)
       * ======================================================
       */
      
      SgStatement * initialisationExpression = buildExprStatement( buildAssignOp (
          d_ref,
          buildIntVal(0) ) );
      
      SgStatement * testExpression = buildExprStatement( buildLessThanOp(
          d_ref,
          dimN_val ) );
      
      SgExpression * incrementExpression = buildPlusPlusOp(
          d_ref );
      
      SgBasicBlock * loopBody = buildBasicBlock();
      
      SgForStatement * forStatement = buildForStatement(
          initialisationExpression,
          testExpression,
          incrementExpression,
          loopBody );
      
      appendStatement( forStatement, subroutineScope );
      
      SgExprStatement * assignmentStatement;

      if (parallelLoop->getOpAccessValue (i) == INC_ACCESS)
      {
        /*
         * ======================================================
         * argN_l[d] = 0
         * ======================================================
         */
        assignmentStatement = buildAssignStatement (
            buildPntrArrRefExp(
                argN_l,
                d_ref ),
            buildIntVal (0)); //FIXME 0.0f
      }
      else
      {
        /*
         * ======================================================
         * argN_l[d] = argN[d+get_group_id(0)*dimN]
         * ======================================================
         */
        assignmentStatement = buildAssignStatement (
            buildPntrArrRefExp(
                argN_l,
                d_ref ),
            buildPntrArrRefExp(
                argN,
                buildAddOp(
                    d_ref,
                    buildMultiplyOp(
                        CPPOpenCLStatementsAndExpressionsBuilder::generateGetGroupId(),
                        dimN_val ) ) ) );
       
      }
      
      appendStatement (assignmentStatement, loopBody);
      
    }
  }
}


void
CPPOpenCLKernelSubroutineDirectLoop::createStatements ()
{
  createThreadIDInitialisationStatement ();

  createAutoSharedDisplacementInitialisationStatement ();

  createInitialiseLocalThreadVariablesStatements ();

  createExecutionLoopStatements ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionLoopStatements ();
  }
}

void
CPPOpenCLKernelSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  using std::vector;
  using std::string;

  vector <string> fourByteIntegers;

  fourByteIntegers.push_back (
      DirectLoop::CPP::KernelSubroutine::setElementCounter);

  fourByteIntegers.push_back (
      DirectLoop::CPP::KernelSubroutine::dataPerElementCounter);

  fourByteIntegers.push_back (
      DirectLoop::CPP::KernelSubroutine::threadIDModulus);

  fourByteIntegers.push_back (
      DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock);

  fourByteIntegers.push_back (
      DirectLoop::CPP::KernelSubroutine::remainingElements);

  fourByteIntegers.push_back (
      DirectLoop::CPP::KernelSubroutine::autosharedDisplacement);

  for (vector <string>::iterator it = fourByteIntegers.begin (); it
      != fourByteIntegers.end (); ++it)
  {
    variableDeclarations->add (*it,
        buildVariableDeclaration(
            *it,
            buildIntType(),
            NULL,
            subroutineScope ) );
  }

  createLocalThreadDeclarations ();

  createAutoSharedDeclaration ();

  if (parallelLoop->isReductionRequired () == true)
  {
    variableDeclarations->add (ReductionSubroutine::offsetForReduction,
        CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
            ReductionSubroutine::offsetForReduction,
            buildIntType(), 
            subroutineScope));
  }
}

void
CPPOpenCLKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{

  /*
   * ======================================================
   * OP_DAT formal parameters
   * ======================================================
   */

  createOpDatFormalParameterDeclarations ();


  /*
   * ======================================================
   * Warp scratch pad size formal parameter. This is the offset
   * in the shared memory variable assigned to each
   * thread block (see Mike's Developers Guide, direct
   * loops section)
   * ======================================================
   */

  variableDeclarations->add (
      DirectLoop::CPP::KernelSubroutine::warpScratchpadSize,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          DirectLoop::CPP::KernelSubroutine::warpScratchpadSize,
          buildIntType(), 
          subroutineScope,
          formalParameters ) );


  /*
   * ======================================================
   * Set size formal parameter
   * ======================================================
   */

  variableDeclarations->add (
      DirectLoop::CPP::KernelSubroutine::setSize,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          DirectLoop::CPP::KernelSubroutine::setSize,
          buildIntType(), 
          subroutineScope,
          formalParameters ) );
  
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
  

  
  //TODO: add global constants parameter

}
/*
 * ======================================================
 * Public functions
 * ======================================================
 */

CPPOpenCLKernelSubroutineDirectLoop::CPPOpenCLKernelSubroutineDirectLoop (
    std::string const & subroutineName, 
    std::string const & userSubroutineName,
    CPPParallelLoop * parallelLoop, 
    SgScopeStatement * moduleScope,
    CPPReductionSubroutines * reductionSubroutines,
    CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
  CPPOpenCLKernelSubroutine (
      subroutineName, 
      userSubroutineName,
      parallelLoop,
      moduleScope,
      reductionSubroutines,
      opDatDimensionsDeclaration),
  dataSizesDeclaration (dataSizesDeclaration)
{
  Debug::getInstance ()->debugMessage ("<Kernel, Direct, OpenCL>",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
