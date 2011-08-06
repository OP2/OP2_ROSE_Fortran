#include <CPPPlan.h>


SgBasicBlock *
CPPPlan::createPlanFunctionCallStatement (
    SubroutineVariableDeclarations * variableDeclarations,
    SgScopeStatement * subroutineScope )
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildFunctionCallExp;

  SgBasicBlock * block = buildBasicBlock();

  SgVarRefExp * parameter1 = buildVarRefExp(
      variableDeclarations->get( VariableNames::getUserSubroutineName ) );

  SgDotExp * parameter2 = buildDotExp ( //TODO: set
      buildVarRefExp ( variableDeclarations->get ( VariableNames::getOpSetName () ) ),
      buildOpaqueVarRefExp (CommonVariableNames::index, block ) 
      );


  SgVarRefExp * parameter3 = buildVarRefExp (
      variableDeclarations->get ( PlanFunction::CPP::partSize)
      );
/*
  SgVarRefExp * parameter6 = buildVarRefExp (
      variableDeclarations->get ( PlanFunction::CPP::maps)
      );

  SgVarRefExp * parameter7 = buildVarRefExp (
      variableDeclarations->get ( PlanFunction::CPP::accesses)
      );
      */

  SgVarRefExp * parameter4 = buildVarRefExp(
      variableDeclarations->get( PlanFunction::CPP::argsNumber )
      );

  SgVarRefExp * parameter5 = buildVarRefExp(
      variableDeclarations->get( PlanFunction::CPP::args )
      );

  SgVarRefExp * parameter6 = buildVarRefExp (
      variableDeclarations->get ( PlanFunction::CPP::indsNumber)
      );

  SgVarRefExp * parameter7 = buildVarRefExp (
      variableDeclarations->get ( PlanFunction::CPP::inds)
      );

  SgExprListExp * actualParameters = buildExprListExp ( parameter1, parameter2,
      parameter3, parameter4, parameter5, parameter6, parameter7 );

  SgFunctionSymbol * functionSymbol = //FIXME
      CPPTypesBuilder::buildNewCPPFunction (
          PlanFunction::CPP::cplan, subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (
      functionSymbol,
      actualParameters
      );

  SgExprStatement * assignmentStatement = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          PlanFunction::CPP::planRet)), 
      functionCall);

  appendStatement ( assignmentStatement, block );

  return block;
}
