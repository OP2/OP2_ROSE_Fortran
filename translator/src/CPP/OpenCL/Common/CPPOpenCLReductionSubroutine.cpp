#include <boost/lexical_cast.hpp>
#include <RoseHelper.h>
#include <Debug.h>
#include <CommonNamespaces.h>
#include <CPPOpenCLReductionSubroutine.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CPPOpenCLStatementsAndExpressionsBuilder.h>
//#include <CPPTypesBuilder.h>


using namespace SageBuilder;

void
CPPOpenCLReductionSubroutine::createStatements ()
{
  using SageInterface::appendStatement;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildGreaterThanOp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildWhileStmt;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildVoidType;
  
  SgExpression * tid_ref = buildVarRefExp ( variableDeclarations->get (ReductionSubroutine::threadID) );
  SgExpression * d_ref = buildVarRefExp ( variableDeclarations->get (CommonVariableNames::iterationCounter1) );
  SgExpression * temp_ref = buildVarRefExp ( variableDeclarations->get (CommonVariableNames::autoshared) );
  SgExpression * dat_l_ref = buildVarRefExp ( variableDeclarations->get (ReductionSubroutine::inputValue) );
  SgExpression * warpSize_ref = buildVarRefExp ( variableDeclarations->get (ReductionSubroutine::warpSize) );
  SgExpression * reduction_ref = buildVarRefExp ( variableDeclarations->get (ReductionSubroutine::reductionType) );
  SgExpression * vtemp_ref = buildVarRefExp ( variableDeclarations->get (ReductionSubroutine::autosharedV) ); //TODO: needed?
  SgExpression * dat_g_ref = buildVarRefExp ( variableDeclarations->get (ReductionSubroutine::reductionResultOnDevice) );
  
  SgStatement * tempStatement;
  SgStatement * initialisationStatement1;
  SgStatement * testExpression1;
  SgExpression * incrementExpression1;
  SgScopeStatement * loopBody1;
  SgStatement * forStatement1;
  SgExpression * ifGuardExpression1;
  SgScopeStatement * ifBody1;
  SgStatement * ifStatement1;
  SgScopeStatement * ifBody2;
  SgStatement * ifStatement2;
  SgExpression * switchExpression1;
  SgScopeStatement * switchBody1;
  SgStatement * switchStatement1;
  SgScopeStatement * caseBody;
  SgStatement * caseStatement;
  SgScopeStatement * caseBody1;
  SgStatement * caseStatement1;
  SgExpression * ifGuardExpression2;
  
  
  /*
   * ======================================================
   * tid = get_local_id(0)
   * ======================================================
   */
  
  tempStatement = buildAssignStatement(
      tid_ref,
      CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalId() );
  
  appendStatement( tempStatement, subroutineScope );
  
  /*
   * ======================================================
   * d = get_local_size(0)>>1
   * ======================================================
   */
  
  tempStatement = buildAssignStatement(
      d_ref,
      buildRshiftOp(
          CPPOpenCLStatementsAndExpressionsBuilder::generateGetLocalSize(),
          buildIntVal(1) ) );
  
  appendStatement( tempStatement, subroutineScope );
  
  /*
   * ======================================================
   * barrier ( CLK_LOCAL_MEM_FENCE )
   * ======================================================
   */
  
  tempStatement = CPPOpenCLStatementsAndExpressionsBuilder::generateBarrierStatement();
  
  appendStatement( tempStatement, subroutineScope );
  
  tempStatement = buildAssignStatement(
      buildPntrArrRefExp(
          temp_ref,
          tid_ref ),
      dat_l_ref );
  
  appendStatement( tempStatement, subroutineScope );
  
  tempStatement = buildAssignStatement(
      warpSize_ref,
      buildOpaqueVarRefExp("WARP_SIZE") );
  
  appendStatement( tempStatement, subroutineScope );
  
  /*
   * ======================================================
   * BEGIN for ( ; d>warpSize; d>>=1 )
   * ======================================================
   */
  
  initialisationStatement1 = buildBasicBlock();
  
  testExpression1 = buildExprStatement( buildGreaterThanOp(
      d_ref,
      warpSize_ref ) );
  
  incrementExpression1 = buildRshiftAssignOp(
      d_ref,
      buildIntVal(1) );
  
  loopBody1 = buildBasicBlock();
  
  forStatement1 = buildForStatement(
      initialisationStatement1,
      testExpression1,
      incrementExpression1,
      loopBody1 );
  
  appendStatement( forStatement1, subroutineScope );
  
  tempStatement = CPPOpenCLStatementsAndExpressionsBuilder::generateBarrierStatement();
  
  appendStatement( tempStatement, loopBody1 );
  
  /*
   * ======================================================
   * BEGIN if (tid<d)
   * ======================================================
   */
  
  ifGuardExpression1 = buildLessThanOp(
      tid_ref,
      d_ref );
  
  ifBody1 = buildBasicBlock();
  
  ifStatement1 = RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression1, 
          ifBody1);
  
  appendStatement( ifStatement1, loopBody1 );
  
  /*
   * ======================================================
   * BEGIN switch ( reduction )
   * ======================================================
   */
  
  switchExpression1 = reduction_ref;
  
  switchBody1 = buildBasicBlock();
  
  switchStatement1 = buildSwitchStatement(
      switchExpression1,
      switchBody1 );
  
  appendStatement( switchStatement1, ifBody1 );
  
  /*
   * ======================================================
   * case OP_INC
   * ======================================================
   */
  
  caseBody1 = buildBasicBlock();  
  
  caseStatement1 = buildCaseOptionStmt(
      buildOpaqueVarRefExp("OP_INC"),
      caseBody );
  
  appendStatement( caseStatement1, switchBody1 );
  
  tempStatement = buildAssignStatement(
      buildPntrArrRefExp(
          temp_ref,
          tid_ref ),
      buildAddOp(
          buildPntrArrRefExp( 
              temp_ref,
              tid_ref ),
          buildPntrArrRefExp( 
              temp_ref,
              buildAddOp(
                  tid_ref,
                  d_ref ) ) ) );
  
  appendStatement( tempStatement, caseBody1 );
  
  tempStatement = buildBreakStmt();
  
  appendStatement( tempStatement, caseBody1 );
  
  /*
   * ======================================================
   * case OP_MIN
   * ======================================================
   */
  
  caseBody = buildBasicBlock();  
  
  caseStatement = buildCaseOptionStmt(
      buildOpaqueVarRefExp("OP_MIN"),
      caseBody );
  
  appendStatement( caseStatement, switchBody1 );
  
  ifGuardExpression2 = buildLessThanOp(
      buildPntrArrRefExp( 
                    temp_ref,
                    buildAddOp(
                        tid_ref,
                        d_ref ) ),
      buildPntrArrRefExp(
          temp_ref,
          tid_ref ) );
  
  ifBody2 = buildBasicBlock();
  
  ifStatement2 = RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
      ifGuardExpression2, 
      ifBody2);
  
  appendStatement( ifStatement2, caseBody );
  
  tempStatement = buildAssignStatement(
      buildPntrArrRefExp(
          temp_ref,
          tid_ref ),
      buildAddOp(
          buildPntrArrRefExp( 
              temp_ref,
              tid_ref ),
          buildPntrArrRefExp( 
              temp_ref,
              buildAddOp(
                  tid_ref,
                  d_ref ) ) ) );
  
  appendStatement( tempStatement, ifBody2 );
  
  tempStatement = buildBreakStmt();
  
  appendStatement( tempStatement, caseBody );
  
  /*
   * ======================================================
   * case OP_MAX
   * ======================================================
   */
  
  caseBody = buildBasicBlock();  
  
  caseStatement = buildCaseOptionStmt(
      buildOpaqueVarRefExp("OP_MAX"),
      caseBody );
  
  appendStatement( caseStatement, switchBody1 );
  
  ifGuardExpression2 = buildGreaterThanOp(
      buildPntrArrRefExp( 
                    temp_ref,
                    buildAddOp(
                        tid_ref,
                        d_ref ) ),
      buildPntrArrRefExp(
          temp_ref,
          tid_ref ) );
  
  ifBody2 = buildBasicBlock();
  
  ifStatement2 = RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
      ifGuardExpression2, 
      ifBody2);
  
  appendStatement( ifStatement2, caseBody );
  
  tempStatement = buildAssignStatement(
      buildPntrArrRefExp(
          temp_ref,
          tid_ref ),
      buildAddOp(
          buildPntrArrRefExp( 
              temp_ref,
              tid_ref ),
          buildPntrArrRefExp( 
              temp_ref,
              buildAddOp(
                  tid_ref,
                  d_ref ) ) ) );
  
  appendStatement( tempStatement, ifBody2 );
  
  tempStatement = buildBreakStmt();
  
  appendStatement( tempStatement, caseBody );
  
  /*
   * ======================================================
   * END switch ( reduction )
   * ======================================================
   */
  
  /*
   * ======================================================
   * END if (tid<d)
   * ======================================================
   */
  
  /*
   * ======================================================
   * END for ( ; d>warpSize; d>>=1 )
   * ======================================================
   */
  
  /*
   * ======================================================
   * barrier ( CLK_LOCAL_MEM_FENCE )
   * ======================================================
   */
  
  tempStatement = CPPOpenCLStatementsAndExpressionsBuilder::generateBarrierStatement();
  
  appendStatement( tempStatement, subroutineScope );
  
  /*
   * ======================================================
   * vtemp = temp
   * ======================================================
   */
  
  tempStatement = buildAssignStatement(
      vtemp_ref,
      temp_ref );

  appendStatement( tempStatement, subroutineScope );
  
  /*
   * ======================================================
   * BEGIN if (tid<warpSize)
   * ======================================================
   */
  
  ifGuardExpression1 = buildLessThanOp(
      tid_ref,
      warpSize_ref );
  
  ifBody1 = buildBasicBlock();
  
  ifStatement1 = RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression1, 
          ifBody1);
  
  appendStatement( ifStatement1, subroutineScope );
  
  /*
   * ======================================================
   * BEGIN for ( ; d>0; d>>=1 )
   * ======================================================
   */
  
  initialisationStatement1 = buildBasicBlock();
  
  testExpression1 = buildExprStatement( buildGreaterThanOp(
      d_ref,
      buildIntVal(0) ) );
  
  incrementExpression1 = buildRshiftAssignOp(
      d_ref,
      buildIntVal(1) );
  
  loopBody1 = buildBasicBlock();
  
  forStatement1 = buildForStatement(
      initialisationStatement1,
      testExpression1,
      incrementExpression1,
      loopBody1 );
  
  appendStatement( forStatement1, ifBody1 );
  
  /*
   * ======================================================
   * BEGIN if (tid<d)
   * ======================================================
   */
  
  ifGuardExpression2 = buildLessThanOp(
      tid_ref,
      d_ref );
  
  ifBody2 = buildBasicBlock();
  
  ifStatement2 = RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
      ifGuardExpression2, 
      ifBody2);
  
  appendStatement( ifStatement2, loopBody1 );
  
  /*
   * ======================================================
   * BEGIN switch ( reduction )
   * ======================================================
   */
  
  switchExpression1 = reduction_ref;
  
  switchBody1 = buildBasicBlock();
  
  switchStatement1 = buildSwitchStatement(
      switchExpression1,
      switchBody1 );
  
  appendStatement( switchStatement1, ifBody2 );
  
  /*
   * ======================================================
   * case OP_INC
   * ======================================================
   */
  
  caseBody = buildBasicBlock();  
  
  caseStatement1 = buildCaseOptionStmt(
      buildOpaqueVarRefExp("OP_INC"),
      caseBody );
  
  appendStatement( caseStatement, switchBody1 );
  
  tempStatement = buildAssignStatement(
      buildPntrArrRefExp(
          vtemp_ref,
          tid_ref ),
      buildAddOp(
          buildPntrArrRefExp( 
              vtemp_ref,
              tid_ref ),
          buildPntrArrRefExp( 
              vtemp_ref,
              buildAddOp(
                  tid_ref,
                  d_ref ) ) ) );
  
  appendStatement( tempStatement, caseBody );
  
  tempStatement = buildBreakStmt();
  
  appendStatement( tempStatement, caseBody );
  
  /*
   * ======================================================
   * case OP_MIN
   * ======================================================
   */
  
  caseBody = buildBasicBlock();  
  
  caseStatement = buildCaseOptionStmt(
      buildOpaqueVarRefExp("OP_MIN"),
      caseBody );
  
  appendStatement( caseStatement, switchBody1 );
  
  ifGuardExpression2 = buildLessThanOp(
      buildPntrArrRefExp( 
          vtemp_ref,
          buildAddOp(
              vtemp_ref,
              d_ref ) ),
      buildPntrArrRefExp(
          vtemp_ref,
          tid_ref ) );
  
  ifBody2 = buildBasicBlock();
  
  ifStatement2 = RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
      ifGuardExpression2, 
      ifBody2);
  
  appendStatement( ifStatement2, caseBody );
  
  tempStatement = buildAssignStatement(
      buildPntrArrRefExp(
          vtemp_ref,
          tid_ref ),
      buildAddOp(
          buildPntrArrRefExp( 
              vtemp_ref,
              tid_ref ),
          buildPntrArrRefExp( 
              vtemp_ref,
              buildAddOp(
                  tid_ref,
                  d_ref ) ) ) );
  
  appendStatement( tempStatement, ifBody2 );
  
  tempStatement = buildBreakStmt();
  
  appendStatement( tempStatement, caseBody );
  
  /*
   * ======================================================
   * case OP_MAX
   * ======================================================
   */
  
  caseBody = buildBasicBlock();  
  
  caseStatement = buildCaseOptionStmt(
      buildOpaqueVarRefExp("OP_MAX"),
      caseBody );
  
  appendStatement( caseStatement, switchBody1 );
  
  ifGuardExpression2 = buildGreaterThanOp(
      buildPntrArrRefExp( 
          vtemp_ref,
          buildAddOp(
              vtemp_ref,
              d_ref ) ),
      buildPntrArrRefExp(
          vtemp_ref,
          tid_ref ) );
  
  ifBody2 = buildBasicBlock();
  
  ifStatement2 = RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
      ifGuardExpression2, 
      ifBody2);
  
  appendStatement( ifStatement2, caseBody );
  
  tempStatement = buildAssignStatement(
      buildPntrArrRefExp(
          vtemp_ref,
          tid_ref ),
      buildAddOp(
          buildPntrArrRefExp( 
              vtemp_ref,
              tid_ref ),
          buildPntrArrRefExp( 
              vtemp_ref,
              buildAddOp(
                  tid_ref,
                  d_ref ) ) ) );
  
  appendStatement( tempStatement, ifBody2 );
  
  tempStatement = buildBreakStmt();
  
  appendStatement( tempStatement, caseBody );
  
  /*
   * ======================================================
   * END switch ( reduction )
   * ======================================================
   */
  
  /*
   * ======================================================
   * END if (tid<d)
   * ======================================================
   */
  
  /*
   * ======================================================
   * END for ( ; d>0; d>>=1 )
   * ======================================================
   */
  
  /*
   * ======================================================
   * END if (tid<warpSize)
   * ======================================================
   */
  
  /*
   * ======================================================
   * BEGIN if (tid == 0)
   * ======================================================
   */
  
  ifGuardExpression1 = buildEqualityOp(
      tid_ref,
      buildIntVal(0) );
  
  ifBody1 = buildBasicBlock();
  
  ifStatement1 = RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression1, 
          ifBody1);
  
  appendStatement( ifStatement1, subroutineScope );
  
  /*
   * ======================================================
   * BEGIN switch ( reduction )
   * ======================================================
   */
  
  switchExpression1 = reduction_ref;
  
  switchBody1 = buildBasicBlock();
  
  switchStatement1 = buildSwitchStatement(
      switchExpression1,
      switchBody1 );
  
  appendStatement( switchStatement1, ifBody1 );
  
  /*
   * ======================================================
   * case OP_INC
   * ======================================================
   */
  
  caseBody = buildBasicBlock();  
  
  caseStatement1 = buildCaseOptionStmt(
      buildOpaqueVarRefExp("OP_INC"),
      caseBody );
  
  appendStatement( caseStatement, switchBody1 );
  
  tempStatement = buildAssignStatement(
      buildPointerDerefExp(
          dat_g_ref ),
      buildAddOp(
          buildPointerDerefExp(
              dat_g_ref ),
          buildPntrArrRefExp(
              vtemp_ref,
              buildIntVal(0) ) ) );
              
  appendStatement( tempStatement, caseBody );
  
  tempStatement = buildBreakStmt();
  
  appendStatement( tempStatement, caseBody );
  
  /*
   * ======================================================
   * case OP_MIN
   * ======================================================
   */
  
  caseBody = buildBasicBlock();  
  
  caseStatement = buildCaseOptionStmt(
      buildOpaqueVarRefExp("OP_MIN"),
      caseBody );
  
  appendStatement( caseStatement, switchBody1 );
  
  ifGuardExpression2 = buildLessThanOp(
      buildPntrArrRefExp( 
          temp_ref,
          buildIntVal(0) ),
      buildPointerDerefExp(
          dat_g_ref ) );
  
  ifBody2 = buildBasicBlock();
  
  ifStatement2 = RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
      ifGuardExpression2, 
      ifBody2);
  
  appendStatement( ifStatement2, caseBody );
  
  tempStatement = buildAssignStatement(
      buildPointerDerefExp(
          dat_g_ref ),
      buildPntrArrRefExp(
          vtemp_ref,
          buildIntVal(0) ) );
  
  appendStatement( tempStatement, ifBody2 );
  
  tempStatement = buildBreakStmt();
  
  appendStatement( tempStatement, caseBody );
  
  /*
   * ======================================================
   * case OP_MAX
   * ======================================================
   */
  
  caseBody = buildBasicBlock();  
  
  caseStatement = buildCaseOptionStmt(
      buildOpaqueVarRefExp("OP_MAX"),
      caseBody );
  
  appendStatement( caseStatement, switchBody1 );
  
  ifGuardExpression2 = buildGreaterThanOp(
      buildPntrArrRefExp( 
          temp_ref,
          buildIntVal(0) ),
      buildPointerDerefExp(
          dat_g_ref ) );
  
  ifBody2 = buildBasicBlock();
  
  ifStatement2 = RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
      ifGuardExpression2, 
      ifBody2);
  
  appendStatement( ifStatement2, caseBody );
  
  tempStatement = buildAssignStatement(
      buildPointerDerefExp(
          dat_g_ref ),
      buildPntrArrRefExp(
          vtemp_ref,
          buildIntVal(0) ) );
  
  appendStatement( tempStatement, ifBody2 );
  
  tempStatement = buildBreakStmt();
  
  appendStatement( tempStatement, caseBody );
  
  /*
   * ======================================================
   * END switch ( reduction )
   * ======================================================
   */
  
  /*
   * ======================================================
   * END if (tid==0)
   * ======================================================
   */
  
  
}

void
CPPOpenCLReductionSubroutine::createLocalVariableDeclarations ()
{
  using std::vector;
  using std::string;



  variableDeclarations->add (ReductionSubroutine::warpSize,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
          ReductionSubroutine::warpSize, 
          buildOpaqueType( "size_t", subroutineScope ), //FIXME
          subroutineScope ) );
  
  variableDeclarations->add (
      ReductionSubroutine::autosharedV,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
          ReductionSubroutine::autosharedV,
          buildPointerType(
              buildVolatileType(
                  reduction->getBaseType() ) ), 
          subroutineScope, 
          1,
          SHARED ) );

  /*
   * ======================================================
   * Create thread ID and iteration counter variables
   * ======================================================
   */

  vector <string> integers;

  integers.push_back (CommonVariableNames::iterationCounter1);
  integers.push_back (ReductionSubroutine::threadID);

  for (vector <string>::iterator it = integers.begin (); 
      it != integers.end (); 
      ++it)
  {
    variableDeclarations->add (*it,
        CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
            *it,
            buildIntType(), 
            subroutineScope ) );
  }
}

void
CPPOpenCLReductionSubroutine::createFormalParameterDeclarations ()
{
  using SageBuilder::buildVolatileType;
  Debug::getInstance ()->debugMessage (
      "Creating reduction procedure formal parameter", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Declare the device variable on which the result of local
   * reductions is stored by the first thread in the block
   * ======================================================
   */
  variableDeclarations->add (
      ReductionSubroutine::reductionResultOnDevice,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          ReductionSubroutine::reductionResultOnDevice,
          buildPointerType(
              buildVolatileType(
                  reduction->getBaseType() ) ), 
          subroutineScope, 
          formalParameters, 
          1,
          DEVICE));

  /*
   * ======================================================
   * Declare the value of the reduction variable produced by
   * each thread which is passed by value.
   * WARNING: the type of the input data is for now limited
   * to a scalar value (does not manage reduction on arrays)
   * ======================================================
   */

  variableDeclarations->add (
      ReductionSubroutine::inputValue,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          ReductionSubroutine::inputValue, 
          reduction->getBaseType(),
          subroutineScope, 
          formalParameters));

  
  variableDeclarations->add (
      ReductionSubroutine::reductionType,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          ReductionSubroutine::reductionType,
          buildOpaqueType("op_access",subroutineScope), //FIXME
          subroutineScope,
          formalParameters));
  
  variableDeclarations->add (
      CommonVariableNames::autoshared,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          CommonVariableNames::autoshared,
          buildPointerType(
              reduction->getBaseType() ), 
          subroutineScope, 
          formalParameters, 
          1,
          SHARED));
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

CPPOpenCLReductionSubroutine::CPPOpenCLReductionSubroutine (
    std::string const & subroutineAndVariableName,
    SgScopeStatement * moduleScope, Reduction * reduction) :
  Subroutine <SgProcedureHeaderStatement> (subroutineAndVariableName),
      reduction (reduction)
{
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildFunctionParameterList;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  Debug::getInstance ()->debugMessage ("Creating reduction subroutine",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(device) ",
      AstUnparseAttribute::e_before);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
