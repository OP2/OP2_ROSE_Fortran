


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


#include "FortranHostSubroutine.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include "FortranParallelLoop.h"
#include "OP2.h"
#include "CompilerGeneratedNames.h"
#include "Debug.h"
#include <boost/algorithm/string/predicate.hpp>
#include <rose.h>

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
   * Add OP_DAT, indirection, OP_MAP, access arguments for
   * each OP_DAT argument group
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & opDatvariableName = getOpDatName (i);

    variableDeclarations->add (
        opDatvariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            opDatvariableName, FortranTypesBuilder::buildClassDeclaration (
                OP2::OP_DAT, subroutineScope)->get_type (), subroutineScope,
            formalParameters, 1, INTENT_IN));

    string const & indirectionVariableName = getOpIndirectionName (i);

    variableDeclarations->add (
        indirectionVariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            indirectionVariableName,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope,
            formalParameters, 1, INTENT_IN));

    string const & opMapVariableName = getOpMapName (i);

    variableDeclarations->add (
        opMapVariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            opMapVariableName, FortranTypesBuilder::buildClassDeclaration (
                OP2::OP_MAP, subroutineScope)->get_type (), subroutineScope,
            formalParameters, 1, INTENT_IN));

    string const & accessVariableName = getOpAccessName (i);

    variableDeclarations->add (
        accessVariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            accessVariableName, FortranTypesBuilder::getFourByteInteger (),
            subroutineScope, formalParameters, 1, INTENT_IN));
  }
}

void
FortranHostSubroutine::createEarlyExitStatement (SgScopeStatement * subroutineScope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;
  
  string const sizeField = "size";
  
  SgDotExp * setSizeField = buildDotExp (
    variableDeclarations->getReference (getOpSetName ()),
    buildOpaqueVarRefExp (sizeField, subroutineScope));

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

void
FortranHostSubroutine::createDumpOfOutputStatements (SgScopeStatement * subroutineScope,
  std::string const dumpOpDatFunctionName)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  
  SgExpression * incrementCalledTimesExpr = buildAssignOp (
    variableDeclarations->getReference (OP2VariableNames::calledTimes),
    buildAddOp (variableDeclarations->getReference (OP2VariableNames::calledTimes),
      buildIntVal (1)));
  
  SgStatement * incrementCalledTimes = buildExprStatement(incrementCalledTimesExpr);

  appendStatement (incrementCalledTimes, subroutineScope);
  
  bool firstCall = true;
  
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if ( !parallelLoop->isRead (i) && !parallelLoop->isGlobal (i) )
      {
        SgExprListExp * actualParameters = buildExprListExp ();
        
        SgStringVal * userSubroutineExpression = buildStringVal (parallelLoop->getUserSubroutineName ());

        SgStringVal * opDatLabel = buildStringVal (getOpDatName (i));
        
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
          getOpDatName (i));
            
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
