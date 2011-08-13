#include <CPPHostSubroutine.h>

using namespace SageBuilder;

SgStatement *
CPPHostSubroutine::createKernelFunctionCallStatement ()
{
  return NULL;

}

void
CPPHostSubroutine::createReductionEpilogueStatements ()
{
}

void
CPPHostSubroutine::createReductionPrologueStatements ()
{
  SgExpression * reduct_size_ref = buildVarRefExp (variableDeclarations->get (ReductionSubroutine::reductionArraySize));
  SgExpression * reduct_bytes_ref = buildVarRefExp (variableDeclarations->get (ReductionSubroutine::reductionArrayBytes));
  SgExpression * nblocks_ref = buildVarRefExp ( variableDeclarations->get (OpenCL::CPP::blocksPerGrid) );
  SgExpression * maxblocks_ref = buildVarRefExp (variableDeclarations->get (ReductionSubroutine::maximumNumberOfThreadBlocks));

  
  SgExprStatement * tempStatement = NULL;

#if 0
  /*
   * ======================================================
   * maxblocks = nblocks //TODO: ind case!
   * ======================================================
   */
  tempStatement = buildAssignStatement (
      maxblocks_ref,
      nblocks_ref );

  appendStatement ( tempStatement, subroutineScope );
#endif
  
  /*
   * ======================================================
   * reduct_bytes = 0
   * ======================================================
   */
  tempStatement = buildAssignStatement (
      reduct_bytes_ref,
      buildIntVal(0) );

  appendStatement ( tempStatement, subroutineScope );
  
  /*
   * ======================================================
   * reduct_size = 0
   * ======================================================
   */
  tempStatement = buildAssignStatement (
      reduct_size_ref,
      buildIntVal(0) );

  appendStatement ( tempStatement, subroutineScope );
  
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * dimN_val = buildIntVal ( parallelLoop->getOpDatDimension (i));
    
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->getOpMapValue (i) == GLOBAL &&
          parallelLoop->getOpAccessValue(i) != READ_ACCESS )
      {
        /*
         * ======================================================
         * reduct_bytes += ROUND_UP( maxblocks*dimN*sizeof(TYP) )
         * ======================================================
         */
        tempStatement = buildExprStatement( buildPlusAssignOp(
            reduct_bytes_ref,
            buildFunctionCallExp(
                "ROUND_UP",
                buildIntType(),
                buildExprListExp(
                    buildMultiplyOp(
                        maxblocks_ref,
                        buildMultiplyOp(
                            dimN_val,
                            buildSizeOfOp(
                                parallelLoop->getOpDatType(i) ) ) ) ) ) ) );

        appendStatement ( tempStatement, subroutineScope );
        
        /*
         * ======================================================
         * reduct_size = MAX(reduct_size, sizeof(TYP) )
         * ======================================================
         */
        tempStatement = buildAssignStatement (
            reduct_size_ref,
            buildFunctionCallExp(
                 "MAX",
                 buildIntType(),
                 buildExprListExp(
                     reduct_size_ref,
                     buildSizeOfOp(
                         parallelLoop->getOpDatType(i) ) ) ) );

        appendStatement ( tempStatement, subroutineScope );
      }
    }
  }
  
  /*
   * ======================================================
   * reallocReductArrays( reduct_bytes )
   * ======================================================
   */
  tempStatement = buildFunctionCallStmt(
       "reallocReductArrays",
       buildVoidType(),
       buildExprListExp(
           reduct_bytes_ref ) );
  
  appendStatement ( tempStatement, subroutineScope );
  
  /*
   * ======================================================
   * reduct_bytes = 0
   * ======================================================
   */
  tempStatement = buildAssignStatement (
      reduct_bytes_ref,
      buildIntVal(0) );

  appendStatement ( tempStatement, subroutineScope );
  
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * argN_ref = buildVarRefExp(variableDeclarations->get(VariableNames::getOpDatName(i)));
    SgExpression * dimN_val = buildIntVal ( parallelLoop->getOpDatDimension (i));
    
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->getOpMapValue (i) == GLOBAL &&
          parallelLoop->getOpAccessValue(i) != READ_ACCESS )
      {
        /*
         * ======================================================
         * argN.data = OP_reduct_h + reduct_bytes
         * ======================================================
         */
        tempStatement = buildAssignStatement(
            buildDotExp(
                argN_ref,
                buildOpaqueVarRefExp(
                    "data" ) ),
            buildPlusPlusOp(
                OP_reduct_h_ref,
                reduct_byes_ref ) );
            
        appendStatement ( tempStatement, subroutineScope );
        
        /*
         * ======================================================
         * argN.data_d = OP_reduct_h + reduct_bytes
         * ======================================================
         */
        tempStatement = buildAssignStatement(
            buildDotExp(
                argN_ref,
                buildOpaqueVarRefExp(
                    "data_d" ) ),
            buildPlusPlusOp(
                OP_reduct_h_ref,
                reduct_byes_ref ) );
            
        appendStatement ( tempStatement, subroutineScope );
        
        /* 
         * ======================================================
         * BEGIN for ( b=0; b<maxblocks; b++ ) 
         * ======================================================
         */
        
        SgStatement * initialisationExpression1 = buildExprStatement( buildAssignOp (
            b_ref,
            buildIntVal(0) ) );
        
        SgStatement * testExpression1 = buildExprStatement( buildLessThanOp(
            b_ref,
            maxblocks_ref ) );
        
        SgExpression * incrementExpression1 = buildPlusPlusOp(
            b_ref );
  
        SgBasicBlock * loopBody1 = buildBasicBlock();
        
        SgForStatement * forStatement1 = buildForStatement(
            initialisationExpression1,
            testExpression1,
            incrementExpression1,
            loopBody1 );
        
        appendStatement( forStatement1, subroutineScope );
        
        /* 
         * ======================================================
         * BEGIN for ( d=0; d<dimN; d++ ) 
         * ======================================================
         */
        
        SgStatement * initialisationExpression2 = buildExprStatement( buildAssignOp (
            d_ref,
            buildIntVal(0) ) );
        
        SgStatement * testExpression2 = buildExprStatement( buildLessThanOp(
            d_ref,
            dimN_val ) );
        
        SgExpression * incrementExpression2 = buildPlusPlusOp(
            d_ref );
  
        SgBasicBlock * loopBody2 = buildBasicBlock();
        
        SgForStatement * forStatement2 = buildForStatement(
            initialisationExpression2,
            testExpression2,
            incrementExpression2,
            loopBody2 );
        
        appendStatement( forStatement2, loopBody1 );
        
        if ( parallelLoop->getOpAccessValue (i) == INC_ACCESS ) 
        {
          /*
           * ======================================================
           * ((TYP *)argN.data)[d+b*dimN] = 0
           * ======================================================
           */
          tempStatement = buildAssignStatement(
              buildPntrArrRefExp(
                  buildCastExp(
                      buildDotExp(
                          argN_ref,
                          buildOpaqueVarRefExp(
                              "data" ) ),
                      buildPointerType(
                          parallelLoop->getOpDatType(i) ) ),
                  buildAddOp(
                      d_ref,
                      buildMultiplyOp(
                          b_ref,
                          dimN_val ) ) ),
              buildIntVal(0) );
              
        } else 
        {
          /*
           * ======================================================
           * ((TYP *)argN.data)[d+b*dimN] = argNh[d]
           * ======================================================
           */
          tempStatement = buildAssignStatement(
              buildPntrArrRefExp(
                  buildCastExp(
                      buildDotExp(
                          argN_ref,
                          buildOpaqueVarRefExp(
                              "data" ) ),
                      buildPointerType(
                          parallelLoop->getOpDatType(i) ) ),
                  buildAddOp(
                      d_ref,
                      buildMultiplyOp(
                          b_ref,
                          dimN_val ) ) ),
              buildPntrArrRefExp(
                  argNh_ref, //XXX ???
                  d_ref ) );
              
        }
        appendStatement ( tempStatement, loopBody2 );

      }
      /* 
       * ======================================================
       * END for ( d=0; d<dimN; d++ ) 
       * ======================================================
       */
      
      /* 
       * ======================================================
       * END for ( b=0; b<maxblocks; b++ ) 
       * ======================================================
       */
    }
  }

  
}

void
CPPHostSubroutine::createReductionLocalVariableDeclarations ()
{
}

void
CPPHostSubroutine::createStatements ()
{
}

void
CPPHostSubroutine::createLocalVariableDeclarations ()
{
}

void
CPPHostSubroutine::createFormalParameterDeclarations ()
{
}
