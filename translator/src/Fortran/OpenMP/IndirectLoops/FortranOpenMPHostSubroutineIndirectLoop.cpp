


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


#include "FortranOpenMPHostSubroutineIndirectLoop.h"
#include "FortranParallelLoop.h"
#include "FortranOpenMPKernelSubroutine.h"
#include "FortranOpenMPModuleDeclarationsIndirectLoop.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "RoseHelper.h"
#include "CompilerGeneratedNames.h"
#include "PlanFunctionNames.h"
#include "OP2.h"
#include "OpenMP.h"
#include "Debug.h"

void
FortranOpenMPHostSubroutineIndirectLoop::createKernelFunctionCallStatement (
    SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace ReductionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating kernel function call statement", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i) == false)
      {
        actualParameters->append_expression (
            variableDeclarations->getReference (getOpDatLocalName (i)));
      }
      else
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (OpenMP::threadID),
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgAddOp * addOpExpression = buildAddOp (
          multiplyExpression, buildIntVal (1));
          
        SgPntrArrRefExp * parameterExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (getReductionArrayHostName (i)),
            addOpExpression);

        actualParameters->append_expression (parameterExpression);
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        actualParameters->append_expression (
            variableDeclarations->getReference (getLocalToGlobalMappingName (i,
                parallelLoop->getUserSubroutineName ())));
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      actualParameters->append_expression (variableDeclarations->getReference (
          getGlobalToLocalMappingName (i,
              parallelLoop->getUserSubroutineName ())));
    }
  }

  actualParameters->append_expression (variableDeclarations->getReference (
      getIndirectOpDatsNumberOfElementsArrayName (
          parallelLoop->getUserSubroutineName ())));

  actualParameters->append_expression (
      variableDeclarations->getReference (getIndirectOpDatsOffsetArrayName (
          parallelLoop->getUserSubroutineName ())));

  actualParameters->append_expression (variableDeclarations->getReference (
      getColourToBlockArrayName (parallelLoop->getUserSubroutineName ())));

  actualParameters->append_expression (variableDeclarations->getReference (
      getOffsetIntoBlockArrayName (parallelLoop->getUserSubroutineName ())));

  actualParameters->append_expression (variableDeclarations->getReference (
      getNumberOfSetElementsPerBlockArrayName (
          parallelLoop->getUserSubroutineName ())));

  actualParameters->append_expression (variableDeclarations->getReference (
      getNumberOfThreadColoursPerBlockArrayName (
          parallelLoop->getUserSubroutineName ())));

  actualParameters->append_expression (variableDeclarations->getReference (
      getThreadColourArrayName (parallelLoop->getUserSubroutineName ())));

  actualParameters->append_expression (variableDeclarations->getReference (
      blockOffset));

  actualParameters->append_expression (variableDeclarations->getReference (
      getIterationCounterVariableName (2)));

  SgExprStatement * callStatement = buildFunctionCallStmt (
      calleeSubroutine->getSubroutineName (), buildVoidType (),
      actualParameters, subroutineScope);

  appendStatement (callStatement, scope);
}

SgBasicBlock *
FortranOpenMPHostSubroutineIndirectLoop::createConvertGlobalToLocalMappingStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;
  using namespace OP2::RunTimeVariableNames;  
  using namespace std;
  
  Debug::getInstance ()->debugMessage (
      "Creating statements to convert global-to-local mapping arrays",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      SgBasicBlock * ifBody = buildBasicBlock ();

      SgPntrArrRefExp * parameterExpression1 = buildPntrArrRefExp (
          variableDeclarations->getReference (
              getOpDatsGlobalToLocalMappingName (
                  parallelLoop->getUserSubroutineName ())), buildIntVal (i));

      SgVarRefExp * parameterExpression2 = variableDeclarations->getReference (
          getGlobalToLocalMappingName (i,
              parallelLoop->getUserSubroutineName ()));
              
      string const sizeField = "size";
      
      SgDotExp * setSizeField = buildDotExp (
        variableDeclarations->getReference (getOpSetName ()),
        buildDotExp ( buildOpaqueVarRefExp (Fortran::setPtr, subroutineScope),
          buildOpaqueVarRefExp (size, subroutineScope)));

      SgAggregateInitializer * parameterExpression3 =
          FortranStatementsAndExpressionsBuilder::buildShapeExpression (
              setSizeField);

      SgStatement
          * callStatement =
              FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                  subroutineScope, parameterExpression1, parameterExpression2,
                  parameterExpression3);

      appendStatement (callStatement, ifBody);

      /*
       * ======================================================
       * If statement
       * ======================================================
       */

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (indirectionDescriptorArray),
          buildIntVal (i));

      SgGreaterOrEqualOp * ifGuardExpression = buildGreaterOrEqualOp (
          arrayExpression, buildIntVal (0));

      SgIfStmt * ifStatement =
          RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
              ifGuardExpression, ifBody);

      appendStatement (ifStatement, block);
    }
  }

  return block;
}

SgBasicBlock *
FortranOpenMPHostSubroutineIndirectLoop::createConvertLocalToGlobalMappingStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to convert local-to-global mapping arrays",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  unsigned int index = 1;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgPntrArrRefExp * parameterExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (
                getIndirectOpDatsLocalToGlobalMappingName (
                    parallelLoop->getUserSubroutineName ())), buildIntVal (
                index));

        SgVarRefExp * parameterExpression2 =
            variableDeclarations->getReference (getLocalToGlobalMappingName (i,
                parallelLoop->getUserSubroutineName ()));

        SgPntrArrRefExp * dotExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (getIndirectOpDatsArrayName (
                parallelLoop->getUserSubroutineName ())), buildIntVal (index));

        SgAggregateInitializer * parameterExpression3 =
            FortranStatementsAndExpressionsBuilder::buildShapeExpression (
                dotExpression);

        SgStatement
            * callStatement =
                FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                    subroutineScope, parameterExpression1,
                    parameterExpression2, parameterExpression3);

        appendStatement (callStatement, block);

        index++;
      }
    }
  }

  return block;
}

SgBasicBlock *
FortranOpenMPHostSubroutineIndirectLoop::createPlanFunctionEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;
  using namespace OP2::RunTimeVariableNames;  
  using namespace std;

  Debug::getInstance ()->debugMessage (
      "Creating plan function epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * Convert plan structure
   * ======================================================
   */

  SgVarRefExp * parameterExpressionA1 = variableDeclarations->getReference (
      getPlanReturnVariableName (parallelLoop->getUserSubroutineName ()));

  SgVarRefExp * parameterExpressionA2 = variableDeclarations->getReference (
      getActualPlanVariableName (parallelLoop->getUserSubroutineName ()));

  SgStatement
      * callStatementA =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionA1, parameterExpressionA2);

  appendStatement (callStatementA, block);

  /*
   * ======================================================
   * Convert nindirect
   * ======================================================
   */

  SgDotExp * parameterExpressionB1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nindirect, block));

  SgVarRefExp * parameterExpressionB2 = variableDeclarations->getReference (
      getIndirectOpDatsArrayName (parallelLoop->getUserSubroutineName ()));

  SgAggregateInitializer * parameterExpressionB3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (numberOfIndirectOpDats));

  SgStatement
      * callStatementB =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionB1, parameterExpressionB2,
              parameterExpressionB3);

  appendStatement (callStatementB, block);

  /*
   * ======================================================
   * Convert ind_maps
   * ======================================================
   */

  SgDotExp * parameterExpressionC1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          ind_maps, block));

  SgVarRefExp * parameterExpressionC2 = variableDeclarations->getReference (
      getIndirectOpDatsLocalToGlobalMappingName (
          parallelLoop->getUserSubroutineName ()));

  SgAggregateInitializer * parameterExpressionC3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (numberOfIndirectOpDats));

  SgStatement
      * callStatementC =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionC1, parameterExpressionC2,
              parameterExpressionC3);

  appendStatement (callStatementC, block);

  /*
   * ======================================================
   * Convert maps
   * ======================================================
   */

  SgDotExp * parameterExpressionD1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (maps,
          block));

  SgVarRefExp * parameterExpressionD2 =
      variableDeclarations->getReference (getOpDatsGlobalToLocalMappingName (
          parallelLoop->getUserSubroutineName ()));

  SgAggregateInitializer * parameterExpressionD3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          variableDeclarations->getReference (numberOfOpDats));

  SgStatement
      * callStatementD =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionD1, parameterExpressionD2,
              parameterExpressionD3);

  appendStatement (callStatementD, block);

  /*
   * ======================================================
   * Convert ncolblk
   * ======================================================
   */

  SgDotExp * parameterExpressionE1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          ncolblk, block));

  SgVarRefExp * parameterExpressionE2 = variableDeclarations->getReference (
      getColourToNumberOfBlocksArrayName (
          parallelLoop->getUserSubroutineName ()));

  string const sizeField = "size";
  
  SgDotExp * setSizeField = buildDotExp (
    variableDeclarations->getReference (getOpSetName ()),
    buildDotExp ( buildOpaqueVarRefExp (Fortran::setPtr, subroutineScope),
      buildOpaqueVarRefExp (size, subroutineScope)));

  SgAggregateInitializer * parameterExpressionE3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          setSizeField);

  SgStatement
      * callStatementE =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionE1, parameterExpressionE2,
              parameterExpressionE3);

  appendStatement (callStatementE, block);

  /*
   * ======================================================
   * Convert ind_sizes
   * ======================================================
   */

  SgDotExp * parameterExpressionF1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          ind_sizes, block));

  SgVarRefExp * parameterExpressionF2 = variableDeclarations->getReference (
      getIndirectOpDatsNumberOfElementsArrayName (
          parallelLoop->getUserSubroutineName ()));

  SgDotExp * dotExpressionF3 = buildDotExp (variableDeclarations->getReference (
      getActualPlanVariableName (parallelLoop->getUserSubroutineName ())),
      buildOpaqueVarRefExp (nblocks, subroutineScope));

  SgMultiplyOp * multiplyExpressionF3 = buildMultiplyOp (dotExpressionF3,
      variableDeclarations->getReference (numberOfIndirectOpDats));

  SgAggregateInitializer * parameterExpressionF3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          multiplyExpressionF3);

  SgStatement
      * callStatementF =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionF1, parameterExpressionF2,
              parameterExpressionF3);

  appendStatement (callStatementF, block);

  /*
   * ======================================================
   * Convert ind_offs
   * ======================================================
   */

  SgDotExp * parameterExpressionG1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          ind_offs, block));

  SgVarRefExp * parameterExpressionG2 =
      variableDeclarations->getReference (getIndirectOpDatsOffsetArrayName (
          parallelLoop->getUserSubroutineName ()));

  SgDotExp * dotExpressionG3 = buildDotExp (variableDeclarations->getReference (
      getActualPlanVariableName (parallelLoop->getUserSubroutineName ())),
      buildOpaqueVarRefExp (nblocks, subroutineScope));

  SgMultiplyOp * multiplyExpressionG3 = buildMultiplyOp (dotExpressionG3,
      variableDeclarations->getReference (numberOfIndirectOpDats));

  SgAggregateInitializer * parameterExpressionG3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          multiplyExpressionG3);

  SgStatement
      * callStatementG =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionG1, parameterExpressionG2,
              parameterExpressionG3);

  appendStatement (callStatementG, block);

  /*
   * ======================================================
   * Convert blkmap
   * ======================================================
   */

  SgDotExp * parameterExpressionH1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          blkmap, block));

  SgVarRefExp * parameterExpressionH2 = variableDeclarations->getReference (
      getColourToBlockArrayName (parallelLoop->getUserSubroutineName ()));

  SgDotExp * dotExpressionH3 = buildDotExp (variableDeclarations->getReference (
      getActualPlanVariableName (parallelLoop->getUserSubroutineName ())),
      buildOpaqueVarRefExp (nblocks, subroutineScope));

  SgAggregateInitializer * parameterExpressionH3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          dotExpressionH3);

  SgStatement
      * callStatementH =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionH1, parameterExpressionH2,
              parameterExpressionH3);

  appendStatement (callStatementH, block);

  /*
   * ======================================================
   * Convert offset
   * ======================================================
   */

  SgDotExp * parameterExpressionI1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          offset, block));

  SgVarRefExp * parameterExpressionI2 = variableDeclarations->getReference (
      getOffsetIntoBlockArrayName (parallelLoop->getUserSubroutineName ()));

  SgDotExp * dotExpressionI3 = buildDotExp (variableDeclarations->getReference (
      getActualPlanVariableName (parallelLoop->getUserSubroutineName ())),
      buildOpaqueVarRefExp (nblocks, subroutineScope));

  SgAggregateInitializer * parameterExpressionI3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          dotExpressionI3);

  SgStatement
      * callStatementI =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionI1, parameterExpressionI2,
              parameterExpressionI3);

  appendStatement (callStatementI, block);

  /*
   * ======================================================
   * Convert nelems
   * ======================================================
   */

  SgDotExp * parameterExpressionJ1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nelems, block));

  SgVarRefExp * parameterExpressionJ2 = variableDeclarations->getReference (
      getNumberOfSetElementsPerBlockArrayName (
          parallelLoop->getUserSubroutineName ()));

  SgDotExp * dotExpressionJ3 = buildDotExp (variableDeclarations->getReference (
      getActualPlanVariableName (parallelLoop->getUserSubroutineName ())),
      buildOpaqueVarRefExp (nblocks, subroutineScope));

  SgAggregateInitializer * parameterExpressionJ3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          dotExpressionJ3);

  SgStatement
      * callStatementJ =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionJ1, parameterExpressionJ2,
              parameterExpressionJ3);

  appendStatement (callStatementJ, block);

  /*
   * ======================================================
   * Convert nthrcol
   * ======================================================
   */

  SgDotExp * parameterExpressionK1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          nthrcol, block));

  SgVarRefExp * parameterExpressionK2 = variableDeclarations->getReference (
      getNumberOfThreadColoursPerBlockArrayName (
          parallelLoop->getUserSubroutineName ()));

  SgDotExp * dotExpressionK3 = buildDotExp (variableDeclarations->getReference (
      getActualPlanVariableName (parallelLoop->getUserSubroutineName ())),
      buildOpaqueVarRefExp (nblocks, subroutineScope));

  SgAggregateInitializer * parameterExpressionK3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          dotExpressionK3);

  SgStatement
      * callStatementK =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionK1, parameterExpressionK2,
              parameterExpressionK3);

  appendStatement (callStatementK, block);

  /*
   * ======================================================
   * Convert thrcol (relies on setSizeField built above)
   * ======================================================
   */

  SgDotExp * parameterExpressionL1 = buildDotExp (
      variableDeclarations->getReference (getActualPlanVariableName (
          parallelLoop->getUserSubroutineName ())), buildOpaqueVarRefExp (
          thrcol, block));

  SgVarRefExp * parameterExpressionL2 = variableDeclarations->getReference (
      getThreadColourArrayName (parallelLoop->getUserSubroutineName ()));

  SgAggregateInitializer * parameterExpressionL3 =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          setSizeField);

  SgStatement
      * callStatementL =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpressionL1, parameterExpressionL2,
              parameterExpressionL3);

  appendStatement (callStatementL, block);

  /*
   * ======================================================
   * Convert maps (ind_maps)
   * ======================================================
   */

  appendStatement (createConvertLocalToGlobalMappingStatements (), block);

  /*
   * ======================================================
   * Convert maps (loc_maps)
   * ======================================================
   */

  appendStatement (createConvertGlobalToLocalMappingStatements (), block);

  return block;
}

void
FortranOpenMPHostSubroutineIndirectLoop::createPlanFunctionExecutionStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating plan function execution statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (blockOffset), buildIntVal (0));

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * Build the inner loop body
   * ======================================================
   */

  Debug::getInstance ()->debugMessage (
      "Creating thread ID initialisation", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);
  
  SgBasicBlock * innerLoopBody = buildBasicBlock ();

  SgVarRefExp * threadIDRef = variableDeclarations->getReference (OpenMP::threadID);

  Debug::getInstance ()->debugMessage (
      "Created thread ID reference", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);
  
  SgExprStatement * initThreadID = buildAssignStatement (
    threadIDRef, OpenMP::createGetThreadIDCallStatement (subroutineScope));

  Debug::getInstance ()->debugMessage (
      "Appending statement", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);    
    
  appendStatement ( initThreadID, innerLoopBody);

  Debug::getInstance ()->debugMessage (
      "Creating kernel call", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);
    
  createKernelFunctionCallStatement (innerLoopBody);

  /*
   * ======================================================
   * Build the inner loop statement
   * ======================================================
   */

  SgAssignOp * innerLoopInitializationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (2)),
      buildIntVal (0));

  SgExpression * innerLoopUpperBoundExpression = buildSubtractOp (
      variableDeclarations->getReference (nblocks), buildIntVal (1));

  SgFortranDo * innerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          innerLoopInitializationExpression, innerLoopUpperBoundExpression,
          buildIntVal (1), innerLoopBody);

  std::vector <SgVarRefExp *> privateVariableReferences;

  privateVariableReferences.push_back (variableDeclarations->getReference (
      OpenMP::threadID));

  addTextForUnparser (innerLoopStatement,
      OpenMP::getParallelLoopDirectiveString () +
      OpenMP::getPrivateClause (privateVariableReferences) + "\n",
      AstUnparseAttribute::e_before);

  addTextForUnparser (innerLoopStatement,
      OpenMP::getEndParallelLoopDirectiveString (),
      AstUnparseAttribute::e_after);

  /*
   * ======================================================
   * Build the outer loop body
   * ======================================================
   */

  SgBasicBlock * outerLoopBody = buildBasicBlock ();

  SgAddOp * addExpression2 = buildAddOp (variableDeclarations->getReference (
      getIterationCounterVariableName (1)), buildIntVal (1));

  SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (getColourToNumberOfBlocksArrayName (
          parallelLoop->getUserSubroutineName ())), addExpression2);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (nblocks), arrayIndexExpression2);

  appendStatement (assignmentStatement2, outerLoopBody);

  appendStatement (innerLoopStatement, outerLoopBody);

  SgAddOp * addExpression3 = buildAddOp (variableDeclarations->getReference (
      blockOffset), variableDeclarations->getReference (nblocks));

  SgStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (blockOffset), addExpression3);

  appendStatement (assignmentStatement3, outerLoopBody);

  /*
   * ======================================================
   * Build the outer loop statement
   * ======================================================
   */

  SgAssignOp * outerLoopInitializationExpression = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (0));

  SgDotExp * dotExpression3 = buildDotExp (variableDeclarations->getReference (
      getActualPlanVariableName (parallelLoop->getUserSubroutineName ())),
      buildOpaqueVarRefExp (ncolors, subroutineScope));

  SgExpression * outerLoopUpperBoundExpression = buildSubtractOp (
      dotExpression3, buildIntVal (1));

  SgFortranDo * outerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          outerLoopInitializationExpression, outerLoopUpperBoundExpression,
          buildIntVal (1), outerLoopBody);

  appendStatement (outerLoopStatement, subroutineScope);
}

SgBasicBlock *
FortranOpenMPHostSubroutineIndirectLoop::createSetUpOpDatTypeArrayStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to set up types array for OP_DAT",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        getOpMapCoreName (i)), buildOpaqueVarRefExp (
        OP2::RunTimeVariableNames::dim, subroutineScope));

    SgEqualityOp * ifGuardExpression = buildEqualityOp (dotExpression,
        buildIntVal (-1));

    SgBasicBlock * thenBody = buildBasicBlock ();

    SgBasicBlock * elseBody = buildBasicBlock ();

    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (opDatTypesArray), buildIntVal (i));

    SgExprStatement * assignmentStatement1 = buildAssignStatement (
        arrayIndexExpression, buildOpaqueVarRefExp (
            OP2::Macros::fortranTypeOpGblMacro, subroutineScope));

    appendStatement (assignmentStatement1, thenBody);

    SgExprStatement * assignmentStatement2 = buildAssignStatement (
        arrayIndexExpression, buildOpaqueVarRefExp (
            OP2::Macros::fortranTypeOpDatMacro, subroutineScope));

    appendStatement (assignmentStatement2, elseBody);

    SgIfStmt * ifStatement =
        buildIfStmt (ifGuardExpression, thenBody, elseBody);

    ifStatement->setCaseInsensitive (true);
    ifStatement->set_use_then_keyword (true);
    ifStatement->set_has_end_statement (true);

    appendStatement (ifStatement, block);
  }

  return block;
}


SgBasicBlock *
FortranOpenMPHostSubroutineIndirectLoop::createPlanFunctionStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements needed for plan function", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  appendStatement (createPlanFunctionParametersPreparationStatements (), block);

//  appendStatement (createSetUpOpDatTypeArrayStatements (), block);

  appendStatement (createPlanFunctionCallStatement (), block);

  appendStatement (createPlanFunctionEpilogueStatements (), block);


  return block;
}

void
FortranOpenMPHostSubroutineIndirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;

  createEarlyExitStatementNewLibrary (subroutineScope);

  initialiseProfilingVariablesDeclaration ();

  createStartTimerHost ();
  
  appendStatement (createInitialisePartitionSizeStatements (),
      subroutineScope);

  appendStatement (createInitialiseNumberOfThreadsStatements (),
      subroutineScope);

  initialiseNumberOfOpArgs (subroutineScope);

  appendPopulationOpArgArray (subroutineScope);
      
  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  appendStatement (createPlanFunctionStatements (), subroutineScope);

  appendStatement (createTransferOpDatStatements (), subroutineScope);

  Debug::getInstance ()->debugMessage (
    "Host subroutine indirect loop, creating end timer 1",
    Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createEndTimerHost ();
  Debug::getInstance ()->debugMessage (
    "Host subroutine indirect loop, creating end timer 2",
    Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createEndTimerSynchroniseHost ();
  
    Debug::getInstance ()->debugMessage (
    "Host subroutine indirect loop, creating end timer 3",
    Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createElapsedTimeHost ();
  
    Debug::getInstance ()->debugMessage (
    "Host subroutine indirect loop, creating end timer 4",
    Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);
  
  createAccumulateTimesHost ();

  Debug::getInstance ()->debugMessage (
       "Host subroutine indirect loop, start time for kernel",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createStartTimerKernel ();

  
  createPlanFunctionExecutionStatements ();

  createEndTimerKernel ();
  createEndTimerSynchroniseKernel ();
  createElapsedTimeKernel ();
  createAccumulateTimesKernel ();

  createStartTimerHost ();  
  
  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
  
  createEndTimerHost ();
  createEndTimerSynchroniseHost ();
  createElapsedTimeHost ();
  createAccumulateTimesHost ();

}

void
FortranOpenMPHostSubroutineIndirectLoop::createPlanFunctionLocalVariableDeclarations ()
{
  using namespace PlanFunctionVariableNames;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations for plan function",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (
      opDatArray,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          opDatArray, FortranTypesBuilder::getArray_RankOne (
              FortranTypesBuilder::getFourByteInteger (), 1,
              parallelLoop->getNumberOfOpDatArgumentGroups ()), subroutineScope));

  variableDeclarations->add (
      mappingIndicesArray,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          mappingIndicesArray, FortranTypesBuilder::getArray_RankOne (
              FortranTypesBuilder::getFourByteInteger (), 1,
              parallelLoop->getNumberOfOpDatArgumentGroups ()), subroutineScope));

  variableDeclarations->add (
      mappingArray,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          mappingArray, FortranTypesBuilder::getArray_RankOne (
              FortranTypesBuilder::getFourByteInteger (), 1,
              parallelLoop->getNumberOfOpDatArgumentGroups ()), subroutineScope));

  variableDeclarations->add (
      accessDescriptorArray,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          accessDescriptorArray, FortranTypesBuilder::getArray_RankOne (
              FortranTypesBuilder::getFourByteInteger (), 1,
              parallelLoop->getNumberOfOpDatArgumentGroups ()), subroutineScope));

  variableDeclarations->add (
      indirectionDescriptorArray,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          indirectionDescriptorArray, FortranTypesBuilder::getArray_RankOne (
              FortranTypesBuilder::getFourByteInteger (), 1,
              parallelLoop->getNumberOfOpDatArgumentGroups ()), subroutineScope));

  variableDeclarations->add (
      opDatTypesArray,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          opDatTypesArray, FortranTypesBuilder::getArray_RankOne (
              FortranTypesBuilder::getFourByteInteger (), 1,
              parallelLoop->getNumberOfOpDatArgumentGroups ()), subroutineScope));

  variableDeclarations->add (numberOfIndirectOpDats,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfIndirectOpDats, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (blockOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          blockOffset, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (nblocks,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          nblocks, FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (getIterationCounterVariableName (2),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

void
FortranOpenMPHostSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createCommonLocalVariableDeclarations (subroutineScope);
      
  createOpDatLocalVariableDeclarations ();

  createOpenMPLocalVariableDeclarations ();

  createPlanFunctionLocalVariableDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionDeclarations ();
  }

  /*
   * ======================================================
   * Profiling declarations. Eventually, this should only
   * be done if a certain compiler option is turned on
   * ======================================================
   */

  createProfilingVariablesDeclaration ();  
}

FortranOpenMPHostSubroutineIndirectLoop::FortranOpenMPHostSubroutineIndirectLoop (
    SgScopeStatement * moduleScope,
    FortranOpenMPKernelSubroutine * kernelSubroutine,
    FortranParallelLoop * parallelLoop,
    FortranOpenMPModuleDeclarationsIndirectLoop * moduleDeclarations) :
  FortranOpenMPHostSubroutine (moduleScope, kernelSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "OpenMP host subroutine creation for indirect loop",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  variableDeclarations->addVisibilityToSymbolsFromOuterScope (
      moduleDeclarations->getDeclarations ());

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
