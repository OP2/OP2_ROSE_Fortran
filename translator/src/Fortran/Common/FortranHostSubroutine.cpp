


/*  Open source copyright declaration based on BSD open source template:
 *  http://www.opensource.org/licenses/bsd-license.php
 * 
 * Copyright (c) 2011-2012, Adam Betts, Carlo Bertolli
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <FortranHostSubroutine.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <FortranParallelLoop.h>
#include <OP2.h>
#include <CompilerGeneratedNames.h>
#include <PlanFunctionNames.h>
#include <Debug.h>
#include <boost/algorithm/string/predicate.hpp>
#include <rose.h>
#include <Globals.h>

/*
 * ======================================================
 * Maximum size of variable name for opDats in 
 * host stub
 * ======================================================
 */

int MAX_OPDAT_NAME_LEN = 50;

void
FortranHostSubroutine::createFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2;
  using namespace OP2VariableNames;
  using boost::iequals;
  using std::vector;
  using std::string;
  using std::map;

  Debug::getInstance ()->debugMessage (
      "Creating host subroutine formal parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Add name of user kernel
   * ======================================================
   */

  string const & kernelVariableName = getUserSubroutineName ();

  variableDeclarations->add (
      kernelVariableName,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          kernelVariableName, FortranTypesBuilder::getString (
              parallelLoop->getUserSubroutineName ().length () + 1),
          subroutineScope, formalParameters, 1, INTENT_IN));

  /*
   * ======================================================
   * Add OP_SET
   * ======================================================
   */

  string const & opSetVariableName = getOpSetName ();

  variableDeclarations->add (
      opSetVariableName,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          opSetVariableName, FortranTypesBuilder::buildClassDeclaration (
              OP_SET, subroutineScope)->get_type (), subroutineScope,
          formalParameters, 1, INTENT_IN));


  /*
   * ======================================================
   * Add OP_ARGs to formal parameters
   * ======================================================
   */
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {  
    string const & opArgVariableName = getOpArgName (i);

    variableDeclarations->add (
        opArgVariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            opArgVariableName, FortranTypesBuilder::buildClassDeclaration (
                OP2::OP_ARG, subroutineScope)->get_type (), subroutineScope,
            formalParameters, 1, INTENT_IN));
  }
    
}

void
FortranHostSubroutine::createEarlyExitStatement (SgScopeStatement * subroutineScope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::RunTimeVariableNames;
  using std::string;
  
  string const sizeField = "size";
  
  
  
  SgDotExp * setSizeField = buildDotExp (
    variableDeclarations->getReference (getOpSetName ()),
    buildDotExp ( buildOpaqueVarRefExp (Fortran::setPtr, subroutineScope),
      buildOpaqueVarRefExp (size, subroutineScope)));

  SgExpression * conditionSetZero = buildEqualityOp (setSizeField, buildIntVal (0) );

  SgBasicBlock * ifBody = buildBasicBlock ();
  
  SgReturnStmt * returnStatement = buildReturnStmt (buildNullExpression ());
  
  appendStatement (returnStatement, ifBody);
  
  SgIfStmt * ifSetIsZero =
    RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
    conditionSetZero , ifBody);

  appendStatement (ifSetIsZero, subroutineScope);
}

void
FortranHostSubroutine::createEarlyExitStatementNewLibrary (SgScopeStatement * subroutineScope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2;  

  using std::string;
  
  string const sizeField = RunTimeVariableNames::size;
  string const setPtr = RunTimeVariableNames::Fortran::setPtr;

  SgExpression * conditionSetZero = buildEqualityOp (variableDeclarations->getReference (
    returnMPIHaloExchange), buildIntVal (0));

  SgBasicBlock * ifBody = buildBasicBlock ();

  /*
   * ======================================================
   * In case of MPI we need to wait for halo exchanges and 
   * set the dirty bits
   * ======================================================
   */
  if ( Globals::getInstance ()->getIncludesMPI () )
    appendCallMPIWaitAll (ifBody);
  
  if ( Globals::getInstance ()->getIncludesMPI () )
    appendCallMPISetDirtyBit (ifBody);

  SgReturnStmt * returnStatement = buildReturnStmt (buildNullExpression ());
  
  appendStatement (returnStatement, ifBody);
  
  SgIfStmt * ifSetIsZero =
    RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
    conditionSetZero , ifBody);

  appendStatement (ifSetIsZero, subroutineScope);
}


void
FortranHostSubroutine::createDumpOfOutputDeclarations (SgScopeStatement * subroutineScope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  
  variableDeclarations->add (OP2VariableNames::calledTimes,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2VariableNames::calledTimes, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, 1, SAVE));  
          
  variableDeclarations->add (OP2VariableNames::returnDumpOpDat,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2VariableNames::returnDumpOpDat, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope, 0));  
          
}

SgBasicBlock *
FortranHostSubroutine::createPlanFunctionParametersPreparationStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to prepare plan function parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * Set up a mapping between OP_DATs and indirection
   * values. At the beginning everything is set to undefined
   * ======================================================
   */

  std::map <std::string, int> indexValues;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      indexValues[parallelLoop->getOpDatVariableName (i)] = -1;
    }
  }

  /*
   * ======================================================
   * Start at the value defined by Mike Giles in his
   * implementation
   * ======================================================
   */
  unsigned int nextIndex = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (indirectionDescriptorArray),
        buildIntVal (i));

    SgExprStatement * assignmentStatement;

    if (parallelLoop->isIndirect (i))
    {
      if (indexValues[parallelLoop->getOpDatVariableName (i)] == -1)
      {
        assignmentStatement = buildAssignStatement (arrayIndexExpression,
            buildIntVal (nextIndex));

        indexValues[parallelLoop->getOpDatVariableName (i)] = nextIndex;

        nextIndex++;
      }
      else
      {
        assignmentStatement = buildAssignStatement (arrayIndexExpression,
            buildIntVal (indexValues[parallelLoop->getOpDatVariableName (i)]));
      }
    }
    else
    {
      assignmentStatement = buildAssignStatement (arrayIndexExpression,
          buildIntVal (-1));
    }

    appendStatement (assignmentStatement, block);
  }

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (numberOfIndirectOpDats), buildIntVal (
          parallelLoop->getNumberOfDistinctIndirectOpDats ()));

  appendStatement (assignmentStatement3, block);

  return block;
}

SgExprStatement *
FortranHostSubroutine::createPlanFunctionCallStatement ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage ("Creating plan function call statement",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVarRefExp * parameter1 = variableDeclarations->getReference (
      getUserSubroutineName ());

  SgDotExp * parameter2 = buildDotExp (variableDeclarations->getReference (
      getOpSetName ()), buildOpaqueVarRefExp (OP2::RunTimeVariableNames::Fortran::setCPtr,
      subroutineScope));

  SgVarRefExp * parameter3 = variableDeclarations->getReference (
      planPartitionSize);
            
  SgVarRefExp * parameter4 = variableDeclarations->getReference (numberOfOpDats);

  SgVarRefExp * parameter5 = variableDeclarations->getReference (opArgArray);

  SgVarRefExp * parameter6 = variableDeclarations->getReference (
      numberOfIndirectOpDats);

  SgVarRefExp * parameter7 = variableDeclarations->getReference (
      indirectionDescriptorArray);
      
  SgExprListExp * actualParameters = buildExprListExp (parameter1, parameter2,
      parameter3, parameter4, parameter5, parameter6, parameter7);

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction (fortranCplanFunction,
          subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  SgExprStatement * assignmentStatement = buildAssignStatement (
      variableDeclarations->getReference (getPlanReturnVariableName (
          parallelLoop->getUserSubroutineName ())), functionCall);

  return assignmentStatement;
}


void
FortranHostSubroutine::createDumpOfOutputStatements (SgScopeStatement * subroutineScope,
  std::string const dumpOpDatFunctionName)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  
  SgExpression * incrementCalledTimesExpr = buildAssignOp (
    variableDeclarations->getReference (OP2VariableNames::calledTimes),
    buildAddOp (variableDeclarations->getReference (OP2VariableNames::calledTimes),
      buildIntVal (1)));
  
  SgStatement * incrementCalledTimes = buildExprStatement(incrementCalledTimesExpr);

  appendStatement (incrementCalledTimes, subroutineScope);
  
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if ( !parallelLoop->isRead (i) && !parallelLoop->isGlobal (i) )
      {
        SgExprListExp * actualParameters = buildExprListExp ();
        
        SgStringVal * userSubroutineExpression = buildStringVal (parallelLoop->getUserSubroutineName ());

        SgStringVal * opDatLabel = buildStringVal (getOpArgName (i));
        
        SgFunctionSymbol * functionSymbol =
            FortranTypesBuilder::buildNewFortranFunction ("CHAR", subroutineScope);

        SgExprListExp * actualParametersSlashZero = buildExprListExp ();

        actualParametersSlashZero->append_expression (buildIntVal (0));

        SgFunctionCallExp * functionCallExpSlashZero = buildFunctionCallExp (
            functionSymbol, actualParametersSlashZero);
        
        SgConcatenationOp * concatenationExpression1 = buildConcatenationOp (
            userSubroutineExpression, opDatLabel);	 
          
        SgConcatenationOp * concatenationExpression2 = buildConcatenationOp (
          concatenationExpression1, functionCallExpSlashZero);
          
        SgVarRefExp * opDatReference = variableDeclarations->getReference (
          getOpArgName (i));
            
        actualParameters->append_expression (opDatReference);
        actualParameters->append_expression (concatenationExpression2);
        actualParameters->append_expression (
          variableDeclarations->getReference (OP2VariableNames::calledTimes));

        SgFunctionSymbol * functionSymbolDumpOpDat =
          FortranTypesBuilder::buildNewFortranFunction (
            dumpOpDatFunctionName, subroutineScope);

        SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbolDumpOpDat,
          actualParameters);
       
        SgExprStatement * assignStmt = buildAssignStatement (
          variableDeclarations->getReference (OP2VariableNames::returnDumpOpDat),
          functionCall);
        
        appendStatement (assignStmt, subroutineScope);
        
        /*
         * ======================================================
         * Checks if PRINT_OUTPUT_DAT macro is set using
         * pre-processor directives (#ifdef)
         * \warning must be placed here for correct unparsing
         * by ROSE
         * ======================================================
         */

        addTextForUnparser (assignStmt, OP2::PreprocessorDirectives::getIfPrintOpDatDirectiveString(),
          AstUnparseAttribute::e_before);

        addTextForUnparser (assignStmt, OP2::PreprocessorDirectives::getEndIfPrintOpDatDirectiveString  (),
          AstUnparseAttribute::e_after);        
      }
    }
  }
}

void
FortranHostSubroutine::appendCallMPIHaloExchangeFunction (SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2;
  using namespace OP2VariableNames;
  
  SgExprListExp * actualParameters = buildExprListExp ();

  SgExpression * setCPtrField = buildDotExp (
    variableDeclarations->getReference (getOpSetName ()),
    buildOpaqueVarRefExp (RunTimeVariableNames::Fortran::setCPtr, subroutineScope));
  
  actualParameters->append_expression (setCPtrField);
  actualParameters->append_expression (variableDeclarations->getReference (numberOfOpDats));
  actualParameters->append_expression (variableDeclarations->getReference (opArgArray));
  
  SgFunctionSymbol * functionSymbolSetDirtyBit = FortranTypesBuilder::buildNewFortranFunction (
      opMpiHaloExchanges, subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbolSetDirtyBit,
    actualParameters);

  SgExprStatement * assignHaloCall = buildAssignStatement (variableDeclarations->getReference (
    returnMPIHaloExchange), functionCall);
    
  appendStatement (assignHaloCall, scope);
}

void
FortranHostSubroutine::appendCallMPISetDirtyBit (SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2;
  using namespace OP2VariableNames;

  SgExprListExp * actualParameters = buildExprListExp ();    
        
  actualParameters->append_expression (variableDeclarations->getReference (numberOfOpDats));
  actualParameters->append_expression (variableDeclarations->getReference (opArgArray));

  SgFunctionSymbol * functionSymbolSetDirtyBit =
    FortranTypesBuilder::buildNewFortranSubroutine (
      opMpiSetDirtyBit, subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbolSetDirtyBit,
    actualParameters);

  appendStatement (buildExprStatement (functionCall), scope);
}

void
FortranHostSubroutine::appendCallMPIWaitAll (SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2;
  using namespace OP2VariableNames;

  SgExprListExp * actualParameters = buildExprListExp ();    
        
  actualParameters->append_expression (variableDeclarations->getReference (numberOfOpDats));
  actualParameters->append_expression (variableDeclarations->getReference (opArgArray));

  SgFunctionSymbol * functionSymbolSetDirtyBit = FortranTypesBuilder::buildNewFortranSubroutine (
      opMpiWaitAll, subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbolSetDirtyBit,
    actualParameters);

  appendStatement (buildExprStatement (functionCall), scope);
}

void
FortranHostSubroutine::appendPopulationOpArgArray (SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2;
  using namespace OP2VariableNames;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
     SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
         variableDeclarations->getReference (opArgArray), buildIntVal (i));

     SgExprStatement * assignmentStatement = buildAssignStatement (
         arrayIndexExpression, variableDeclarations->getReference (
          getOpArgName (i)));

     appendStatement (assignmentStatement, scope);
  }
}

void
FortranHostSubroutine::initialiseNumberOfOpArgs (SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2;
  using namespace OP2VariableNames;

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (numberOfOpDats), buildIntVal (
          parallelLoop->getNumberOfOpDatArgumentGroups ()));

  appendStatement (assignmentStatement2, scope);
}

void
FortranHostSubroutine::createCommonLocalVariableDeclarations (SgScopeStatement * scope)
{
  using namespace OP2;
  using namespace OP2VariableNames;
  using namespace std;
  
  string const & opArgArrayVariableName = opArgArray;
  
  variableDeclarations->add (
      opArgArrayVariableName,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          opArgArrayVariableName, FortranTypesBuilder::getArrayTypePlainDimension(FortranTypesBuilder::buildClassDeclaration (
              OP_ARG, subroutineScope)->get_type (), parallelLoop->getNumberOfArgumentGroups ()), scope));

  variableDeclarations->add (numberOfOpDats,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfOpDats, FortranTypesBuilder::getFourByteInteger (),
          scope));

  variableDeclarations->add (returnMPIHaloExchange,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          returnMPIHaloExchange, FortranTypesBuilder::getFourByteInteger (),
          scope));

  variableDeclarations->add (returnSetKernelTiming,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          returnSetKernelTiming, FortranTypesBuilder::getFourByteInteger (),
          scope));

}

void
FortranHostSubroutine::appendCallsToMPIReduce (SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace std;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i) == true)
    {
      SgExprListExp * actualParameters = buildExprListExp ();    

      actualParameters->append_expression (
        variableDeclarations->getReference (getOpArgName (i)));
      actualParameters->append_expression (
        buildDotExp (variableDeclarations->getReference (getOpArgName (i)),
          buildOpaqueVarRefExp (RunTimeVariableNames::data, subroutineScope)));

      string functionName;

      if ( isSgTypeInt (parallelLoop->getOpDatBaseType (i)) )
      
        functionName = opMpiReduceInt;
      
      else if ( isSgTypeDouble (parallelLoop->getOpDatBaseType (i)))
        
        functionName = opMpiReduceDouble;
      
      else if ( isSgTypeFloat (parallelLoop->getOpDatBaseType (i)))
      
        functionName = opMpiReduceFloat;
      
      else if ( isSgTypeBool (parallelLoop->getOpDatBaseType (i)))
      
        functionName = opMpiReduceBool;
        
      else
      {
        Debug::getInstance ()->debugMessage (
          "Type of global reduction unsupported in MPI", Debug::FUNCTION_LEVEL,
          __FILE__, __LINE__);
      }

      SgFunctionSymbol * functionSymbolMPIReduce = FortranTypesBuilder::buildNewFortranSubroutine (
          functionName, subroutineScope);

      SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbolMPIReduce,
        actualParameters);

      appendStatement (buildExprStatement (functionCall), scope);
    }
  } 
}

FortranHostSubroutine::FortranHostSubroutine (SgScopeStatement * moduleScope,
    Subroutine <SgProcedureHeaderStatement> * calleeSubroutine,
    FortranParallelLoop * parallelLoop) :
  HostSubroutine <SgProcedureHeaderStatement> (calleeSubroutine, parallelLoop)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  appendStatement (subroutineHeaderStatement, moduleScope);

  appendStatement (
      FortranStatementsAndExpressionsBuilder::buildImplicitNoneStatement (),
      subroutineScope);
}
