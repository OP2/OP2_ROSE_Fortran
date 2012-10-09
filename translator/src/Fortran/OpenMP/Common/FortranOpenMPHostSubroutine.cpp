


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


#include "FortranOpenMPHostSubroutine.h"
#include "FortranKernelSubroutine.h"
#include "FortranParallelLoop.h"
#include "FortranOpenMPModuleDeclarations.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include "RoseHelper.h"
#include "CompilerGeneratedNames.h"
#include "OP2.h"
#include "OpenMP.h"

void
FortranOpenMPHostSubroutine::createReductionEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Create the inner loop
   * ======================================================
   */

  SgBasicBlock * outerLoopBody = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i))
    {     
      
      SgAssignOp * innerLoopInitializationExpression = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              11)), buildIntVal (1));

      SgExpression * innerLoopUpperBoundExpression = 
        buildIntVal (parallelLoop->getOpDatDimension (i));
        
      SgExpression * multiplyExpression2 = buildMultiplyOp (
        buildSubtractOp (
          variableDeclarations->getReference (getIterationCounterVariableName (10)),
          buildIntVal (1)),
        buildIntVal (parallelLoop->getOpDatDimension (i)));      
        
      SgExpression * arraySubscriptExpression = buildAddOp (
        multiplyExpression2, variableDeclarations->getReference (
          getIterationCounterVariableName (11)));
        
      SgExpression * reductionArrayAccess = buildPntrArrRefExp (
        variableDeclarations->getReference (getReductionArrayHostName (i)),
        arraySubscriptExpression);

      SgExpression * globalArrayAccess;

      if ( parallelLoop->isArray (i) )
        globalArrayAccess = buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatLocalName (i)),
            variableDeclarations->getReference (getIterationCounterVariableName (11)));
      else
        globalArrayAccess = variableDeclarations->getReference (getOpDatLocalName (i));
      
      SgExpression * assignmentRightExpression;
      
      if ( parallelLoop->isIncremented (i) )      
        assignmentRightExpression = buildAddOp (
          globalArrayAccess, reductionArrayAccess);
      else if ( parallelLoop->isMaximised (i))
      {
          SgFunctionSymbol * maxFunctionSymbol =
          FortranTypesBuilder::buildNewFortranFunction ("max",
              subroutineScope);

          SgExprListExp * actualParameters = buildExprListExp (
              globalArrayAccess, reductionArrayAccess);

          assignmentRightExpression = buildFunctionCallExp (
              maxFunctionSymbol, actualParameters);
      }
      else if ( parallelLoop->isMinimised (i) )
      {
          SgFunctionSymbol * minFunctionSymbol =
          FortranTypesBuilder::buildNewFortranFunction ("min",
              subroutineScope);

          SgExprListExp * actualParameters = buildExprListExp (
              globalArrayAccess, reductionArrayAccess);

          assignmentRightExpression = buildFunctionCallExp (
              minFunctionSymbol, actualParameters);
      }

      SgStatement * assignHostArray = buildAssignStatement (
        globalArrayAccess, assignmentRightExpression);
          
      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      innerLoopBody->append_statement (assignHostArray);

      SgFortranDo * innerLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              innerLoopInitializationExpression, innerLoopUpperBoundExpression,
              buildIntVal (1), innerLoopBody);

      outerLoopBody->append_statement (innerLoopStatement);
    }
  }

  /*
   * ======================================================
   * Create the outer loop
   * ======================================================
   */

  SgAssignOp * outerLoopInitializationExpression =
      buildAssignOp (variableDeclarations->getReference (
          getIterationCounterVariableName (10)), buildIntVal (1));

  SgExpression * outerLoopUpperBoundExpression = variableDeclarations->getReference (OpenMP::numberOfThreads);      

  SgFortranDo * outerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          outerLoopInitializationExpression, outerLoopUpperBoundExpression,
          buildIntVal (1), outerLoopBody);

  appendStatement (outerLoopStatement, subroutineScope);
  
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i))
    {
      FortranStatementsAndExpressionsBuilder::appendDeallocateStatement (
        variableDeclarations->getReference (getReductionArrayHostName (i)),
        subroutineScope);
    }
  }
}

void
FortranOpenMPHostSubroutine::createReductionPrologueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating reduction prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Warning: this must happen after we have obtained the
   * number of OMP threads
   * ======================================================
   */

  SgBasicBlock * outerLoopBody = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i))
    {     

      /*
       * ======================================================
       * Allocate 1 variable per thread DIM * num_threads
       * ======================================================
       */
      SgExpression * allocateSize = buildMultiplyOp (
        variableDeclarations->getReference (OpenMP::numberOfThreads),
        buildIntVal (parallelLoop->getOpDatDimension (i)));
        
      FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
        variableDeclarations->getReference (getReductionArrayHostName (i)),
        allocateSize, subroutineScope);

      
      SgAssignOp * innerLoopInitializationExpression = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              11)), buildIntVal (1));

      SgExpression * innerLoopUpperBoundExpression = 
        buildIntVal (parallelLoop->getOpDatDimension (i));

      SgExpression * multiplyExpression2 = buildMultiplyOp (
        buildSubtractOp (
          variableDeclarations->getReference (getIterationCounterVariableName (10)),
          buildIntVal (1)),
        buildIntVal (parallelLoop->getOpDatDimension (i)));
      
      SgExpression * arraySubscriptExpression = buildAddOp (
        multiplyExpression2, variableDeclarations->getReference (
          getIterationCounterVariableName (11)));
        
      SgExpression * reductionArrayAccess = buildPntrArrRefExp (
        variableDeclarations->getReference (getReductionArrayHostName (i)),
        arraySubscriptExpression);
      
      SgExpression * assignmentLeftExpression;
      
      if ( parallelLoop->isIncremented (i) )
        assignmentLeftExpression = buildIntVal (0);
      else              
        assignmentLeftExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatLocalName (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (11)));
        
      SgStatement * assignReductionArray = buildAssignStatement (
        reductionArrayAccess, assignmentLeftExpression);
          
      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      innerLoopBody->append_statement (assignReductionArray);

      SgFortranDo * innerLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              innerLoopInitializationExpression, innerLoopUpperBoundExpression,
              buildIntVal (1), innerLoopBody);

      outerLoopBody->append_statement (innerLoopStatement);      
    }
  }

  /*
   * ======================================================
   * Create the outer loop
   * ======================================================
   */

  SgAssignOp * outerLoopInitializationExpression =
      buildAssignOp (variableDeclarations->getReference (
          getIterationCounterVariableName (10)), buildIntVal (1));

  SgExpression * outerLoopUpperBoundExpression = variableDeclarations->getReference (OpenMP::numberOfThreads);      

  SgFortranDo * outerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          outerLoopInitializationExpression, outerLoopUpperBoundExpression,
          buildIntVal (1), outerLoopBody);

  appendStatement (outerLoopStatement, subroutineScope);  
}

void
FortranOpenMPHostSubroutine::createReductionDeclarations ()
{
  using namespace SageBuilder;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating reduction local variable declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (getIterationCounterVariableName (10),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (10),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (getIterationCounterVariableName (11),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (11),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i))
    {
      string const & reductionArrayName = getReductionArrayHostName (i);

      SgVariableDeclaration * reductionArray =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              reductionArrayName, FortranTypesBuilder::getArray_RankOne (
                  parallelLoop->getOpDatBaseType (i)), subroutineScope, 1,
              ALLOCATABLE);

      variableDeclarations->add (reductionArrayName, reductionArray);
    }
  }
}

SgBasicBlock *
FortranOpenMPHostSubroutine::createInitialiseNumberOfThreadsStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise the number of threads",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OpenMP::numberOfThreads),
      OpenMP::createGetMaximumNumberOfThreadsCallStatement (subroutineScope));

  appendStatement (assignmentStatement1, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OpenMP::numberOfThreads),
      buildIntVal (1));

  appendStatement (assignmentStatement2, subroutineScope);

  addTextForUnparser (assignmentStatement1, OpenMP::getIfDirectiveString (),
      AstUnparseAttribute::e_before);

  addTextForUnparser (assignmentStatement2, OpenMP::getElseDirectiveString (),
      AstUnparseAttribute::e_before);

  addTextForUnparser (assignmentStatement2, OpenMP::getEndIfDirectiveString (),
      AstUnparseAttribute::e_after);

  return block;
}

SgBasicBlock *
FortranOpenMPHostSubroutine::createInitialisePartitionSizeStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise the partition size",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OP2VariableNames::partitionSize),
      buildOpaqueVarRefExp (OP2VariableNames::OP_PART_SIZE_1, subroutineScope));

  appendStatement (assignmentStatement1, subroutineScope);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (OP2VariableNames::partitionSize),
      buildIntVal (0));

  appendStatement (assignmentStatement2, subroutineScope);

  addTextForUnparser (assignmentStatement1, OpenMP::getIfPartitionSizeDirectiveString (),
      AstUnparseAttribute::e_before);

  addTextForUnparser (assignmentStatement2, OpenMP::getElseDirectiveString (),
      AstUnparseAttribute::e_before);

  addTextForUnparser (assignmentStatement2, OpenMP::getEndIfDirectiveString (),
      AstUnparseAttribute::e_after);

  return block;
}


SgBasicBlock *
FortranOpenMPHostSubroutine::createTransferOpDatStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::RunTimeVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to transfer OP_DAT", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  SgDotExp * dotExpression1 = buildDotExp (variableDeclarations->getReference (
      getOpSetName ()), buildOpaqueVarRefExp (Fortran::setPtr, block));

  SgPointerAssignOp * assignExpression1 = new SgPointerAssignOp (
      RoseHelper::getFileInfo (), variableDeclarations->getReference (
          getOpSetCoreName ()), dotExpression1, buildVoidType ());

  assignExpression1->set_endOfConstruct (RoseHelper::getFileInfo ());

  appendStatement (buildExprStatement (assignExpression1), block);


//   for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
//   {
//     if (parallelLoop->isDuplicateOpDat (i) == false)
//     {
//       if (parallelLoop->isReductionRequired (i) == false)
//       {
//         SgDotExp * dotExpression = buildDotExp (
//             variableDeclarations->getReference (getOpDatCoreName (i)),
//             buildOpaqueVarRefExp (set, block));
// 
//         SgStatement
//             * callStatement =
//                 FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
//                     subroutineScope, dotExpression,
//                     variableDeclarations->getReference (getOpSetCoreName (i)));
// 
//         appendStatement (callStatement, block);
//       }
//     }
//   }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgDotExp * dotExpression1 = buildDotExp (
          variableDeclarations->getReference (getOpArgName (i)),
          buildOpaqueVarRefExp (dim, block));

      if (parallelLoop->isReductionRequired (i))
      {
        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (getOpDatCardinalityName (i)),
            dotExpression1);

        appendStatement (assignmentStatement, block);
      }
      else
      {
        SgExprStatement * assignmentStatement;
        
        if ( parallelLoop->isGlobal (i) == false )
        {
          SgFunctionSymbol * functionSymbol =
          FortranTypesBuilder::buildNewFortranFunction (OP2VariableNames::getSetSizeFunctionName,
          subroutineScope);

          SgExprListExp * argumentsSetSize = buildExprListExp (variableDeclarations->getReference (
            getOpArgName (i)));
              
          SgFunctionCallExp * getSetSize = buildFunctionCallExp (      
            functionSymbol, argumentsSetSize);

          SgMultiplyOp * multiplyExpression = buildMultiplyOp (dotExpression1,
              getSetSize );

          assignmentStatement = buildAssignStatement (
              variableDeclarations->getReference (getOpDatCardinalityName (i)),
              multiplyExpression);
        }
        else
        {
          assignmentStatement = buildAssignStatement (
              variableDeclarations->getReference (getOpDatCardinalityName (i)),
              dotExpression1);          
        }
          
        appendStatement (assignmentStatement, block);
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (getOpArgName (i)),
          buildOpaqueVarRefExp (data, block));

      SgStatement * callStatement;
      
      if (parallelLoop->isGlobal (i) == true && parallelLoop->isArray (i) == false)
      {
        callStatement =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
            subroutineScope, dotExpression,
            variableDeclarations->getReference (getOpDatLocalName (i)));
      }
      else
      {        
        SgAggregateInitializer * shapeExpression =
            FortranStatementsAndExpressionsBuilder::buildShapeExpression (
                variableDeclarations->getReference (getOpDatCardinalityName (i)));

        callStatement =
          FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
            subroutineScope, dotExpression,
            variableDeclarations->getReference (getOpDatLocalName (i)),
            shapeExpression);        
      }
      
      appendStatement (callStatement, block);
    }
  }

  return block;
}

void
FortranOpenMPHostSubroutine::createOpDatLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating variables needed to initialise OP_DAT data",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (getOpSetCoreName (),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getOpSetCoreName (), buildPointerType (
              FortranTypesBuilder::buildClassDeclaration (OP2::OP_SET_CORE,
                  subroutineScope)->get_type ()), subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    string const & variableName1 = getOpDatCoreName (i);

    variableDeclarations->add (variableName1,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName1, buildPointerType (
                FortranTypesBuilder::buildClassDeclaration (OP2::OP_DAT_CORE,
                    subroutineScope)->get_type ()), subroutineScope));

    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName2 = getOpDatLocalName (i);

      /*
       * ======================================================
       * Distinguish between global scalars and global arrays
       * and op_dats
       * ======================================================
       */

      if ( parallelLoop->isGlobal (i) && !parallelLoop->isArray (i) )
        variableDeclarations->add (variableName2,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName2, buildPointerType (parallelLoop->getOpDatBaseType (i)), subroutineScope));
      else
        variableDeclarations->add (variableName2,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName2, buildPointerType (
                    FortranTypesBuilder::getArray_RankOne (
                        parallelLoop->getOpDatBaseType (i))), subroutineScope));

      string const & variableName3 = getOpDatCardinalityName (i);

      variableDeclarations->add (variableName3,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName3, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));

      if (parallelLoop->isReductionRequired (i) == false)
      {
        string const & variableName4 = getOpSetCoreName (i);

        variableDeclarations->add (variableName4,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName4, buildPointerType (
                    FortranTypesBuilder::buildClassDeclaration (
                        OP2::OP_SET_CORE, subroutineScope)->get_type ()),
                subroutineScope));
      }
    }

    if (parallelLoop->isDirectLoop () == false)
    {
      string const & variableName5 = getOpMapCoreName (i);

      variableDeclarations->add (variableName5,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName5, buildPointerType (
                  FortranTypesBuilder::buildClassDeclaration (OP2::OP_MAP_CORE,
                      subroutineScope)->get_type ()), subroutineScope));
    }
  }
}

void
FortranOpenMPHostSubroutine::createOpenMPLocalVariableDeclarations ()
{
  using namespace LoopVariableNames;


  variableDeclarations->add (OpenMP::threadID,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::threadID,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));  
  
  variableDeclarations->add (getIterationCounterVariableName (1),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (OpenMP::numberOfThreads,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::numberOfThreads, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  if (parallelLoop->isDirectLoop ())
  {
    variableDeclarations->add (OpenMP::sliceStart,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            OpenMP::sliceStart, FortranTypesBuilder::getFourByteInteger (),
            subroutineScope));

    variableDeclarations->add (OpenMP::sliceEnd,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            OpenMP::sliceEnd, FortranTypesBuilder::getFourByteInteger (),
            subroutineScope));
  }
  
  variableDeclarations->add (OP2VariableNames::partitionSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2VariableNames::partitionSize, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));
}


void
FortranOpenMPHostSubroutine::createProfilingVariablesDeclaration ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations for performance profiling",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (startTimeHost,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          startTimeHost,
          FortranTypesBuilder::getDoublePrecisionFloat (), subroutineScope));
      
//   variableDeclarations->add (startTimeHost,
//       FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
//           startTimeHost,
//           cudaEventType, subroutineScope));

  variableDeclarations->add (endTimeHost,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          endTimeHost,
          FortranTypesBuilder::getDoublePrecisionFloat (), subroutineScope));

//   variableDeclarations->add (endTimeHost,
//       FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
//           endTimeHost,
//           cudaEventType, subroutineScope));

  variableDeclarations->add (startTimeKernel,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          startTimeKernel,
          FortranTypesBuilder::getDoublePrecisionFloat (), subroutineScope));


//   variableDeclarations->add (startTimeKernel,
//       FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
//           startTimeKernel,
//           cudaEventType, subroutineScope));

  variableDeclarations->add (endTimeKernel,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          endTimeKernel,
          FortranTypesBuilder::getDoublePrecisionFloat (), subroutineScope));

//   variableDeclarations->add (endTimeKernel,
//       FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
//           endTimeKernel,
//           cudaEventType, subroutineScope));



//   variableDeclarations->add (istat,
//       FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
//           istat,
//           FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (accumulatorHostTime,
    FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
       accumulatorHostTime, FortranTypesBuilder::getDoublePrecisionFloat (), subroutineScope));

  variableDeclarations->add (accumulatorKernelTime,
    FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
       accumulatorKernelTime, FortranTypesBuilder::getDoublePrecisionFloat (), subroutineScope));
       
       
  variableDeclarations->add (
      timeArrayStart,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          timeArrayStart, FortranTypesBuilder::getArray_RankOne (
              FortranTypesBuilder::getFourByteInteger (), 1,
              8), subroutineScope));
              
  variableDeclarations->add (
      timeArrayEnd,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          timeArrayEnd, FortranTypesBuilder::getArray_RankOne (
              FortranTypesBuilder::getFourByteInteger (), 1,
              8), subroutineScope));
}

void
FortranOpenMPHostSubroutine::initialiseProfilingVariablesDeclaration ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;

  string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);
  string const & numberCalledVariableName = numberCalled +
    parallelLoop->getUserSubroutineName () + postfixName;

  SgAssignOp * incrementNumberTimesCalled = buildAssignOp (
   variableDeclarations->getReference (numberCalledVariableName),
   buildAddOp (variableDeclarations->getReference (numberCalledVariableName),
      buildIntVal (1)));

  appendStatement (buildExprStatement(incrementNumberTimesCalled), subroutineScope);

  
//   SgFunctionSymbol * functionSymbol =
//     FortranTypesBuilder::buildNewFortranFunction (OP2::FortranSpecific::CUDANames::cudaEventCreateFunction,
//       subroutineScope);
// 
//   SgExprListExp * actualParameters1 = buildExprListExp ();
//   SgExprListExp * actualParameters2 = buildExprListExp ();
//   SgExprListExp * actualParameters3 = buildExprListExp ();
//   SgExprListExp * actualParameters4 = buildExprListExp ();
// 
//   actualParameters1->append_expression (
//     variableDeclarations->getReference (startTimeHost));
// 
//   actualParameters2->append_expression (
//     variableDeclarations->getReference (endTimeHost));
//   
//   actualParameters3->append_expression (
//     variableDeclarations->getReference (startTimeKernel));
// 
//   actualParameters4->append_expression (
//     variableDeclarations->getReference (endTimeKernel));
// 
//   SgFunctionCallExp * functionCallExp1 = buildFunctionCallExp (
//     functionSymbol, actualParameters1);
// 
//   SgFunctionCallExp * functionCallExp2 = buildFunctionCallExp (
//     functionSymbol, actualParameters2);
// 
//   SgFunctionCallExp * functionCallExp3 = buildFunctionCallExp (
//     functionSymbol, actualParameters3);
// 
//   SgFunctionCallExp * functionCallExp4 = buildFunctionCallExp (
//     functionSymbol, actualParameters4);
// 
//   SgAssignOp * assignFunctionCall1 = buildAssignOp (
//     variableDeclarations->getReference (istat),
//     functionCallExp1);
// 
//   SgAssignOp * assignFunctionCall2 = buildAssignOp (
//     variableDeclarations->getReference (istat),
//     functionCallExp2);
// 
//   SgAssignOp * assignFunctionCall3 = buildAssignOp (
//     variableDeclarations->getReference (istat),
//     functionCallExp3);
// 
//   SgAssignOp * assignFunctionCall4 = buildAssignOp (
//     variableDeclarations->getReference (istat),
//     functionCallExp4);
// 
//   appendStatement (buildExprStatement(assignFunctionCall1), subroutineScope);
//   appendStatement (buildExprStatement(assignFunctionCall2), subroutineScope);
//   appendStatement (buildExprStatement(assignFunctionCall3), subroutineScope);
//   appendStatement (buildExprStatement(assignFunctionCall4), subroutineScope);
// 
//   string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);
//   string const & numberCalledVariableName = numberCalled +
//     parallelLoop->getUserSubroutineName () + postfixName;
// 
//   SgAssignOp * incrementNumberTimesCalled = buildAssignOp (
//    moduleDeclarations->getDeclarations ()->getReference (numberCalledVariableName),
//    buildAddOp (moduleDeclarations->getDeclarations ()->getReference (numberCalledVariableName),
//       buildIntVal (1)));
// 
//   appendStatement (buildExprStatement(incrementNumberTimesCalled), subroutineScope);
}

void
FortranOpenMPHostSubroutine::createStartTimerHost ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;

  /*  SgMultiplyOp * millisecondFactor = buildMultiplyOp (
    buildFloatVal (0.001), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayStart), buildIntVal(8)));
  */

  SgMultiplyOp * millisecondFactor = buildMultiplyOp (
    buildFloatVal (1), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayStart), buildIntVal(8)));

  /*  SgExpression * secondsFactor = buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayStart), buildIntVal(7));
  */
  SgExpression * secondsFactor = buildMultiplyOp (
      buildFloatVal (1000), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayStart), buildIntVal(7)));

  /*  SgMultiplyOp * minutesFactor = buildMultiplyOp (
    buildIntVal (60), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayStart), buildIntVal(6)));
  */

  SgMultiplyOp * minutesFactor = buildMultiplyOp (
    buildIntVal (60000), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayStart), buildIntVal(6)));

  /*  SgMultiplyOp * hoursFactor = buildMultiplyOp (
    buildIntVal (3600), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayStart), buildIntVal(5)));
  */

  SgMultiplyOp * hoursFactor = buildMultiplyOp (
    buildIntVal (3600000), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayStart), buildIntVal(5)));
  
  SgAssignOp * assignStartTimeHost = buildAssignOp (
    variableDeclarations->getReference (startTimeHost),
    buildAddOp (buildAddOp (buildAddOp (millisecondFactor, secondsFactor), minutesFactor), hoursFactor));

  appendStatement (buildExprStatement (assignStartTimeHost), subroutineScope);
  
  addTextForUnparser (assignStartTimeHost,
     "call date_and_time(values="+timeArrayStart+")\n",
    AstUnparseAttribute::e_before);
}


void
FortranOpenMPHostSubroutine::createEndTimerHost ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;

  /*  SgMultiplyOp * millisecondFactor = buildMultiplyOp (
    buildFloatVal (0.001), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayEnd), buildIntVal(8)));
  */

  SgMultiplyOp * millisecondFactor = buildMultiplyOp (
    buildFloatVal (1), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayEnd), buildIntVal(8)));

  /*  SgExpression * secondsFactor = buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayEnd), buildIntVal(7));
  */

  SgExpression * secondsFactor = buildMultiplyOp (
      buildIntVal (1000), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayEnd), buildIntVal(7)));

  /*  SgMultiplyOp * minutesFactor = buildMultiplyOp (
    buildIntVal (60), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayEnd), buildIntVal(6)));
  */

  SgMultiplyOp * minutesFactor = buildMultiplyOp (
    buildIntVal (60000), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayEnd), buildIntVal(6)));

  // SgMultiplyOp * hoursFactor = buildMultiplyOp (
  //   buildIntVal (3600), buildPntrArrRefExp (
  //     variableDeclarations->getReference (timeArrayEnd), buildIntVal(5)));

  SgMultiplyOp * hoursFactor = buildMultiplyOp (
    buildIntVal (3600000), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayEnd), buildIntVal(5)));
      
  SgAssignOp * assignStopTimeHost = buildAssignOp (
    variableDeclarations->getReference (endTimeHost),
    buildAddOp (buildAddOp (buildAddOp (millisecondFactor, secondsFactor), minutesFactor), hoursFactor));

  appendStatement (buildExprStatement (assignStopTimeHost), subroutineScope);
  
  addTextForUnparser (assignStopTimeHost,
     "call date_and_time(values="+timeArrayEnd+")\n",
    AstUnparseAttribute::e_before);
}

void
FortranOpenMPHostSubroutine::createStartTimerKernel ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;

  // SgMultiplyOp * millisecondFactor = buildMultiplyOp (
  //   buildFloatVal (0.001), buildPntrArrRefExp (
  //     variableDeclarations->getReference (timeArrayStart), buildIntVal(8)));

  SgMultiplyOp * millisecondFactor = buildMultiplyOp (
    buildFloatVal (1), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayStart), buildIntVal(8)));

  // SgExpression * secondsFactor = buildPntrArrRefExp (
  //     variableDeclarations->getReference (timeArrayStart), buildIntVal(7));

  SgExpression * secondsFactor = buildMultiplyOp (
     buildIntVal(1000), buildPntrArrRefExp (
     variableDeclarations->getReference (timeArrayStart), buildIntVal(7)));

  // SgMultiplyOp * minutesFactor = buildMultiplyOp (
  //   buildIntVal (60), buildPntrArrRefExp (
  //     variableDeclarations->getReference (timeArrayStart), buildIntVal(6)));

  SgMultiplyOp * minutesFactor = buildMultiplyOp (
    buildIntVal (60000), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayStart), buildIntVal(6)));

  // SgMultiplyOp * hoursFactor = buildMultiplyOp (
  //   buildIntVal (3600), buildPntrArrRefExp (
  //     variableDeclarations->getReference (timeArrayStart), buildIntVal(5)));

  SgMultiplyOp * hoursFactor = buildMultiplyOp (
    buildIntVal (3600000), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayStart), buildIntVal(5)));
      
  SgAssignOp * assignStartTimeKernel = buildAssignOp (
    variableDeclarations->getReference (startTimeKernel),
    buildAddOp (buildAddOp (buildAddOp (millisecondFactor, secondsFactor), minutesFactor), hoursFactor));

  appendStatement (buildExprStatement (assignStartTimeKernel), subroutineScope);
  
  addTextForUnparser (assignStartTimeKernel,
     "call date_and_time(values="+timeArrayStart+")\n",
    AstUnparseAttribute::e_before);
}

void
FortranOpenMPHostSubroutine::createEndTimerKernel ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;

  // SgMultiplyOp * millisecondFactor = buildMultiplyOp (
  //   buildFloatVal (0.001), buildPntrArrRefExp (
  //     variableDeclarations->getReference (timeArrayEnd), buildIntVal(8)));

  SgMultiplyOp * millisecondFactor = buildMultiplyOp (
    buildFloatVal (1), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayEnd), buildIntVal(8)));

  // SgExpression * secondsFactor = buildPntrArrRefExp (
  //     variableDeclarations->getReference (timeArrayEnd), buildIntVal(7));

   SgExpression * secondsFactor = buildMultiplyOp (buildIntVal(1000), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayEnd), buildIntVal(7)));

  // SgMultiplyOp * minutesFactor = buildMultiplyOp (
  //   buildIntVal (60), buildPntrArrRefExp (
  //     variableDeclarations->getReference (timeArrayEnd), buildIntVal(6)));

  SgMultiplyOp * minutesFactor = buildMultiplyOp (
    buildIntVal (60000), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayEnd), buildIntVal(6)));

  // SgMultiplyOp * hoursFactor = buildMultiplyOp (
  //   buildIntVal (3600), buildPntrArrRefExp (
  //     variableDeclarations->getReference (timeArrayEnd), buildIntVal(5)));

  SgMultiplyOp * hoursFactor = buildMultiplyOp (
    buildIntVal (3600000), buildPntrArrRefExp (
      variableDeclarations->getReference (timeArrayEnd), buildIntVal(5)));

      
  SgAssignOp * assignStopTimeKernel = buildAssignOp (
    variableDeclarations->getReference (endTimeKernel),
    buildAddOp (buildAddOp (buildAddOp (millisecondFactor, secondsFactor), minutesFactor), hoursFactor));

  appendStatement (buildExprStatement (assignStopTimeKernel), subroutineScope);
  
  addTextForUnparser (assignStopTimeKernel,
     "call date_and_time(values="+timeArrayEnd+")\n",
    AstUnparseAttribute::e_before);
}

void
FortranOpenMPHostSubroutine::createEndTimerSynchroniseKernel ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;

}

void
FortranOpenMPHostSubroutine::createEndTimerSynchroniseHost ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;

}

void
FortranOpenMPHostSubroutine::createElapsedTimeHost ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;

  SgExpression * elapsedHost = buildSubtractOp (    
    variableDeclarations->getReference (endTimeHost),
    variableDeclarations->getReference (startTimeHost));
  
  SgAssignOp * assignElapsed = buildAssignOp (
    variableDeclarations->getReference (accumulatorHostTime),
    elapsedHost);

  appendStatement (buildExprStatement (assignElapsed), subroutineScope);
}

void
FortranOpenMPHostSubroutine::createElapsedTimeKernel ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;

  SgExpression * elapsedKernel = buildSubtractOp (
    variableDeclarations->getReference (endTimeKernel),
    variableDeclarations->getReference (startTimeKernel));
  
  SgAssignOp * assignElapsed = buildAssignOp (
    variableDeclarations->getReference (accumulatorKernelTime),
    elapsedKernel);
    
  appendStatement (buildExprStatement (assignElapsed), subroutineScope);
}

void
FortranOpenMPHostSubroutine::createAccumulateTimesHost ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;

  std::cout << "here1" << std::endl;
  
  string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);
  
  std::cout << "here2" << std::endl;
  
  string const & variableNameHost = loopTimeHost +
    parallelLoop->getUserSubroutineName () + postfixName;
    
  std::cout << "here3, " << variableNameHost << std::endl;          
  

  
  SgVarRefExp * varNameHost = variableDeclarations->getReference (variableNameHost);
  
  std::cout << "here3.1, " << variableNameHost << std::endl;  
  
  SgAssignOp * accumulateHostTimes = buildAssignOp (
    varNameHost, buildAddOp (varNameHost,
      variableDeclarations->getReference (accumulatorHostTime)));

  std::cout << "here4" << std::endl;
      
  appendStatement (buildExprStatement(accumulateHostTimes), subroutineScope);
}

void
FortranOpenMPHostSubroutine::createAccumulateTimesKernel ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using std::string;

  string const postfixName = getPostfixNameAsConcatOfOpArgsNames (parallelLoop);
  string const & variableNameKernel = loopTimeKernel +
    parallelLoop->getUserSubroutineName () + postfixName;

  SgAssignOp * accumulateKernelTimes = buildAssignOp (
    variableDeclarations->getReference (variableNameKernel),
    buildAddOp (variableDeclarations->getReference (variableNameKernel),
      variableDeclarations->getReference (accumulatorKernelTime)));

  appendStatement (buildExprStatement(accumulateKernelTimes), subroutineScope);
}


FortranOpenMPHostSubroutine::FortranOpenMPHostSubroutine (
    SgScopeStatement * moduleScope, FortranKernelSubroutine * kernelSubroutine,
    FortranParallelLoop * parallelLoop) :
  FortranHostSubroutine (moduleScope, kernelSubroutine, parallelLoop)
{
}
