


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

  if (parallelLoop->isDirectLoop () == false)
  {
    for (unsigned int i = 1; i
        <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
    {
      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (getOpMapName (i)),
          buildOpaqueVarRefExp (Fortran::mapPtr, block));

      SgPointerAssignOp * assignExpression = new SgPointerAssignOp (
          RoseHelper::getFileInfo (), variableDeclarations->getReference (
              getOpMapCoreName (i)), dotExpression, buildVoidType ());

      assignExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

      appendStatement (buildExprStatement (assignExpression), block);
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        getOpDatName (i)), buildOpaqueVarRefExp (Fortran::dataPtr, block));

    SgPointerAssignOp * assignExpression = new SgPointerAssignOp (
        RoseHelper::getFileInfo (), variableDeclarations->getReference (
            getOpDatCoreName (i)), dotExpression, buildVoidType ());

    assignExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

    appendStatement (buildExprStatement (assignExpression), block);
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i) == false)
      {
        SgDotExp * dotExpression = buildDotExp (
            variableDeclarations->getReference (getOpDatCoreName (i)),
            buildOpaqueVarRefExp (set, block));

        SgStatement
            * callStatement =
                FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                    subroutineScope, dotExpression,
                    variableDeclarations->getReference (getOpSetCoreName (i)));

        appendStatement (callStatement, block);
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgDotExp * dotExpression1 = buildDotExp (
          variableDeclarations->getReference (getOpDatCoreName (i)),
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
          SgDotExp * dotExpression2 = buildDotExp (
              variableDeclarations->getReference (getOpSetCoreName (i)),
              buildOpaqueVarRefExp (size, block));

          SgMultiplyOp * multiplyExpression = buildMultiplyOp (dotExpression1,
              dotExpression2);

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
          variableDeclarations->getReference (getOpDatCoreName (i)),
          buildOpaqueVarRefExp (dat, block));

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

FortranOpenMPHostSubroutine::FortranOpenMPHostSubroutine (
    SgScopeStatement * moduleScope, FortranKernelSubroutine * kernelSubroutine,
    FortranParallelLoop * parallelLoop) :
  FortranHostSubroutine (moduleScope, kernelSubroutine, parallelLoop)
{
}
