#include <CPPOpenCLKernelSubroutine.h>

void
FortranCUDAKernelSubroutine::createInitialiseLocalThreadVariablesStatements ()
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

  /*
   * ======================================================
   * We essentially do a similar thing to the one we make
   * when we declare local thread variables
   * ======================================================
   */
  SgExpression * d_ref = buildVarRefExp (variableDeclarations->get(DirectLoop::CPP::KernelSubroutine::setElementCounter));


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



CPPOpenCLKernelSubroutine::CPPOpenCLKernelSubroutine (
    std::string const & subroutineName, 
    std::string const & userSubroutineName,
    SgScopeStatement * moduleScope, 
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines) :
  CPPKernelSubroutine (subroutineName, userSubroutineName, moduleScope,
      parallelLoop, reductionSubroutines)
{
  using SageInterface::addTextForUnparser;

  addTextForUnparser (subroutineHeaderStatement, "__kernel ", AstUnparseAttribute::e_before);
}
