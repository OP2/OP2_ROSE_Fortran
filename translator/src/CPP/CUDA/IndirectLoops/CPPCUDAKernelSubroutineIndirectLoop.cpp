


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

#include "CPPCUDAKernelSubroutineIndirectLoop.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "CompilerGeneratedNames.h"
#include "PlanFunctionNames.h"
#include "OP2.h"
#include "CUDA.h"
#include "OP2Definitions.h"
#include "CPPProgramDeclarationsAndDefinitions.h"

SgStatement *
CPPCUDAKernelSubroutineIndirectLoop::createUserSubroutineCallStatement ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating call to user device subroutine", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  SgVarRefExp * itvar;
  if (parallelLoop->getNumberOfOpMatArgumentGroups () > 0)
  {
    itvar = variableDeclarations->getReference (
        getIterationCounterVariableName (2));
  }
  else
  {
    itvar = variableDeclarations->getReference (
        getIterationCounterVariableName (1));
  }
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i))
    {
      unsigned int mat_num = parallelLoop->getOpMatArgNum (i);
      SgExpression * exp;
      exp = variableDeclarations->getReference (getIterationCounterVariableName (1));

      exp = buildAddOp (variableDeclarations->getReference (getOpMatName (mat_num)),
                        exp);
      actualParameters->append_expression (exp);
      continue;
    }
    SgExpression * parameterExpression;

    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if (parallelLoop->isIndirect (i))
    {
      if (parallelLoop->isIncremented (i))
      {
        Debug::getInstance ()->debugMessage (
            "Indirect OP_DAT with increment access", Debug::OUTER_LOOP_LEVEL,
            __FILE__, __LINE__);

        parameterExpression = variableDeclarations->getReference (
            getOpDatLocalName (dat_num));
      }
      else
      {
        Debug::getInstance ()->debugMessage (
            "Indirect OP_DAT with read/read-write/write access",
            Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        if (parallelLoop->getOpIndexValue (i) < 0 && !parallelLoop->isDuplicateOpDat (i)) // OP_ALL
        {
          parameterExpression = variableDeclarations->getReference (
              getOpDatVecName (dat_num));
        }
        else
        {
                               
          SgAddOp * addExpression1 = buildAddOp (itvar,
              variableDeclarations->getReference (sharedMemoryOffset));

          SgPntrArrRefExp * arrayExpression1 =
            buildPntrArrRefExp (variableDeclarations->getReference (
                    getGlobalToLocalMappingName (dat_num)), addExpression1);

          SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (arrayExpression1,
              buildIntVal (parallelLoop->getOpDatDimension (i)));

          parameterExpression = buildAddOp (variableDeclarations->getReference (
                  getIndirectOpDatSharedMemoryName (dat_num)), multiplyExpression1);
        }
      }
    }
    else if (parallelLoop->isDirect (i))
    {
      Debug::getInstance ()->debugMessage ("Direct OP_DAT",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      SgAddOp * addExpression1 = buildAddOp (itvar,
          variableDeclarations->getReference (sharedMemoryOffset));

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (addExpression1,
          buildIntVal (parallelLoop->getOpDatDimension (i)));

      parameterExpression = buildAddOp (variableDeclarations->getReference (
          getOpDatName (dat_num)), multiplyExpression1);
    }
    else if (parallelLoop->isReductionRequired (i))
    {
      Debug::getInstance ()->debugMessage ("Reduction argument",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      parameterExpression = variableDeclarations->getReference (
          getOpDatLocalName (dat_num));
    }
    else
    {
      Debug::getInstance ()->debugMessage ("Global with read access",
          Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

      if (parallelLoop->isPointer (i) || parallelLoop->isArray (i))
      {
        parameterExpression = variableDeclarations->getReference (
            getOpDatLocalName (dat_num));
      }
      else
      {
        parameterExpression = variableDeclarations->getReference (getOpDatName (
            dat_num));
      }
    }

    ROSE_ASSERT (parameterExpression != NULL);

    actualParameters->append_expression (parameterExpression);
  }

  if (parallelLoop->getNumberOfOpMatArgumentGroups () > 0)
  {
    actualParameters->append_expression (variableDeclarations->getReference (
            getIterationCounterVariableName (3)));
    actualParameters->append_expression (variableDeclarations->getReference (
            getIterationCounterVariableName (4)));
  }

  return buildFunctionCallStmt (userSubroutine->getSubroutineName (),
      buildVoidType (), actualParameters, subroutineScope);
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createIncrementAndWriteAccessEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating increment and write access epilogue statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        if (parallelLoop->isWritten (i) || parallelLoop->isReadAndWritten (i)
            || parallelLoop->isIncremented (i))
        {
          /*
           * ======================================================
           * For loop body
           * ======================================================
           */

          SgBasicBlock * loopBody = buildBasicBlock ();

          SgDivideOp * divideExpression1 = buildDivideOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgPntrArrRefExp * arrayExpression1a = buildPntrArrRefExp (
              variableDeclarations->getReference (getIndirectOpDatMapName (dat_num)),
              divideExpression1);

          SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
              arrayExpression1a, buildIntVal (parallelLoop->getOpDatDimension (
                  i)));

          SgModOp * modulusExpression1 = buildModOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgAddOp * addExpression1 = buildAddOp (modulusExpression1,
              multiplyExpression1);

          SgPntrArrRefExp * arrayExpression1b = buildPntrArrRefExp (
              variableDeclarations->getReference (getOpDatName (dat_num)),
              addExpression1);

          SgPntrArrRefExp * arrayExpression1c = buildPntrArrRefExp (
              variableDeclarations->getReference (
                  getIndirectOpDatSharedMemoryName (dat_num)),
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)));

          if (parallelLoop->isIncremented (i))
          {
            SgPlusAssignOp * assignmentStatement1 = buildPlusAssignOp (
                arrayExpression1b, arrayExpression1c);

            appendStatement (buildExprStatement (assignmentStatement1),
                loopBody);
          }
          else
          {
            SgExprStatement * assignmentStatement1 = buildAssignStatement (
                arrayExpression1b, arrayExpression1c);

            appendStatement (assignmentStatement1, loopBody);
          }

          /*
           * ======================================================
           * For loop statement
           * ======================================================
           */

          SgExprStatement * initialisationExpression = buildAssignStatement (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), CUDA::getThreadId (
                  THREAD_X, subroutineScope));

          SgMultiplyOp * multiplyExpression =
              buildMultiplyOp (variableDeclarations->getReference (
                  getIndirectOpDatSizeName (dat_num)), buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgLessThanOp * upperBoundExpression = buildLessThanOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), multiplyExpression);

          SgPlusAssignOp * strideExpression = buildPlusAssignOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)),
              CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

          SgForStatement * forStatement = buildForStatement (
              initialisationExpression, buildExprStatement (
                  upperBoundExpression), strideExpression, loopBody);

          appendStatement (forStatement, block);
        }
      }
    }
  }

  return block;
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createStageOutFromLocalMemoryToSharedMemoryStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating stage out from local memory to shared memory statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  SgBasicBlock * ifBody = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      SgAddOp * addExpression1 = buildAddOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), variableDeclarations->getReference (sharedMemoryOffset));

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
          variableDeclarations->getReference (getGlobalToLocalMappingName (dat_num)),
          addExpression1);

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (arrayExpression1,
          buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgAddOp * addExpression2 = buildAddOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), multiplyExpression1);

      SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
          variableDeclarations->getReference (getIndirectOpDatSharedMemoryName (
              dat_num)), addExpression2);

      SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatLocalName (dat_num)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgPlusAssignOp * assignmentStatement = buildPlusAssignOp (
          arrayExpression2, arrayExpression3);

      appendStatement (buildExprStatement (assignmentStatement), innerLoopBody);

      SgExprStatement * innerLoopInitialisationExpression =
          buildAssignStatement (variableDeclarations->getReference (
              getIterationCounterVariableName (2)), buildIntVal (0));

      SgLessThanOp * innerLoopUpperBoundExpression = buildLessThanOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgPlusPlusOp * innerLoopstrideExpression = buildPlusPlusOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgForStatement * innerLoopStatement = buildForStatement (
          innerLoopInitialisationExpression, buildExprStatement (
              innerLoopUpperBoundExpression), innerLoopstrideExpression,
          innerLoopBody);

      appendStatement (innerLoopStatement, ifBody);
    }
  }

  SgEqualityOp * ifGuardExpression = buildEqualityOp (
      variableDeclarations->getReference (colour2),
      variableDeclarations->getReference (colour1));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  SgBasicBlock * loopBody = buildBasicBlock ();

  appendStatement (ifStatement, loopBody);

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      loopBody);

  SgExprStatement * initialisationExpression = buildAssignStatement (
      variableDeclarations->getReference (colour1), buildIntVal (0));

  SgLessThanOp * upperBoundExpression = buildLessThanOp (
      variableDeclarations->getReference (colour1),
      variableDeclarations->getReference (numberOfColours));

  SgPlusPlusOp * strideExpression = buildPlusPlusOp (
      variableDeclarations->getReference (colour1));

  SgForStatement * outerLoopStatement = buildForStatement (
      initialisationExpression, buildExprStatement (upperBoundExpression),
      strideExpression, loopBody);

  appendStatement (outerLoopStatement, block);

  return block;
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createInitialiseIncrementAccessStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise scratchpad memory",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      Debug::getInstance ()->debugMessage ("Creating statements for OP_DAT "
          + lexical_cast <string> (i), Debug::HIGHEST_DEBUG_LEVEL, __FILE__,
          __LINE__);

      SgBasicBlock * loopBody = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatLocalName (dat_num)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      if (isSgTypeFloat (parallelLoop->getOpDatBaseType (i)))
      {
        SgExprStatement * assignmentStatement = buildAssignStatement (
            arrayExpression, buildFloatVal (0));

        appendStatement (assignmentStatement, loopBody);
      }
      else if (isSgTypeDouble (parallelLoop->getOpDatBaseType (i)))
      {
        SgExprStatement * assignmentStatement = buildAssignStatement (
            arrayExpression, buildDoubleVal (0));

        appendStatement (assignmentStatement, loopBody);
      }
      else
      {
        SgExprStatement * assignmentStatement = buildAssignStatement (
            arrayExpression, buildIntVal (0));

        appendStatement (assignmentStatement, loopBody);
      }

      SgExprStatement * initialisationExpression = buildAssignStatement (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgLessThanOp * upperBoundExpression = buildLessThanOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgPlusPlusOp * strideExpression = buildPlusPlusOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgForStatement * forLoopStatement = buildForStatement (
          initialisationExpression, buildExprStatement (upperBoundExpression),
          strideExpression, loopBody);

      appendStatement (forLoopStatement, block);
    }
  }

  return block;
}

void
CPPCUDAKernelSubroutineIndirectLoop::createStageToVectorDatStatements (SgBasicBlock * loopBody)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;
  using namespace OP2::RunTimeVariableNames;

  unsigned int count = 1;
  for (unsigned int i=1; i <= parallelLoop->getNumberOfArgumentGroups (); i++ ) {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isIndirect (i) && parallelLoop->getOpIndexValue (i) < 0) // OP_ALL
    {
      int ntimes = declarations->getOpMapDefinition (parallelLoop->getOpMapName (i))->getDimension ();
      unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
      SgExpression * datvec = variableDeclarations->getReference (getOpDatVecName (dat_num));
      SgVarRefExp * itvar;
      if (parallelLoop->getNumberOfOpMatArgumentGroups () > 0)
      {
        itvar = variableDeclarations->getReference (
          getIterationCounterVariableName (2));
      }
      else
      {
        itvar = variableDeclarations->getReference (
          getIterationCounterVariableName (1));
      }

      SgAddOp * addexp = buildAddOp (itvar,
                                     variableDeclarations->getReference (sharedMemoryOffset));

      for (int j = 0; j < ntimes; j++)
      {
        SgPntrArrRefExp * arrexp =
          buildPntrArrRefExp (variableDeclarations->getReference (
                                getGlobalToLocalMappingName (count++)), addexp);

        SgMultiplyOp * multexp = buildMultiplyOp (arrexp,
                                                  buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgExpression * exp = buildAddOp (
          variableDeclarations->getReference (
            getIndirectOpDatSharedMemoryName (dat_num)),
          multexp);

        SgStatement * s = buildAssignStatement (buildPntrArrRefExp (datvec,
                                                                    buildIntVal (j)), exp);
        appendStatement (s, loopBody);
      }
    }
  }
}

void
CPPCUDAKernelSubroutineIndirectLoop::createExecutionLoopStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;
  using std::string;
  using boost::lexical_cast;
  using boost::iequals;

  Debug::getInstance ()->debugMessage (
      "Creating main execution loop statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * loopBody = buildBasicBlock ();

  SgExprStatement * initialisationExpression = buildAssignStatement (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      CUDA::getThreadId (THREAD_X, subroutineScope));

  SgPlusAssignOp * strideExpression = buildPlusAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

  SgLessThanOp * upperBoundExpression;

  /*
   * Need to zero mat entry calls if accessing via OP_INC
   */
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i))
    {
      unsigned int mat_num = parallelLoop->getOpMatArgNum (i);
      OpArgMatDefinition * arg_mat = parallelLoop->getOpMatArg (mat_num);
      OpMatDefinition * mat = declarations->getOpMatDefinition (arg_mat->getMatName ());
      SgExprStatement * exp = buildAssignStatement (
        buildPntrArrRefExp (variableDeclarations->getReference (getOpMatName (mat_num)),
                            variableDeclarations->getReference (getIterationCounterVariableName (1))),
        buildCastExp(buildIntVal(0), mat->getBaseType ()));
      /* Only need to zero if incrementing (for OP_WRITE we can just
       * leave it) */
      if (iequals (arg_mat->getAccessType (), OP2::OP_INC))
      {
        appendStatement (exp, loopBody);
      }
    }
  }

  if (parallelLoop->hasIncrementedOpDats ())
  {
    SgExprStatement * assignmentStatement1 = buildAssignStatement (
        variableDeclarations->getReference (colour2), buildIntVal (-1));

    appendStatement (assignmentStatement1, loopBody);

    SgBasicBlock * ifBody = buildBasicBlock ();

    appendStatementList (
        createInitialiseIncrementAccessStatements ()->getStatementList (),
        ifBody);

    createStageToVectorDatStatements (ifBody);

    appendStatement (createUserSubroutineCallStatement (), ifBody);

    SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
        getIterationCounterVariableName (1)),
        variableDeclarations->getReference (sharedMemoryOffset));

    SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
        variableDeclarations->getReference (pthrcol), addExpression1);

    SgExprStatement * assignmentStatement2 = buildAssignStatement (
        variableDeclarations->getReference (colour2), arrayExpression1);

    appendStatement (assignmentStatement2, ifBody);

    SgExpression * ifGuardExpression =
        buildLessThanOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)),
            variableDeclarations->getReference (numberOfActiveThreads));

    SgIfStmt * ifStatement =
        RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
            ifGuardExpression, ifBody);

    appendStatement (ifStatement, loopBody);

    appendStatementList (
        createStageOutFromLocalMemoryToSharedMemoryStatements ()->getStatementList (),
        loopBody);

    upperBoundExpression = buildLessThanOp (variableDeclarations->getReference (
        getIterationCounterVariableName (1)),
        variableDeclarations->getReference (numberOfActiveThreadsCeiling));
  }
  else
  {
    OpIterationSpaceDefinition * itspace = parallelLoop->getOpIterationSpace ();
    if (itspace != NULL)
    {
      int div = 1;
      std::vector<int *> itdims = itspace->getIterationDimensions ();
      for (unsigned int i = 0; i < itdims.size (); i++)
      {
        div *= (itdims[i][1] - itdims[i][0]);
      }
      /* find set-element from sub-iteration space */
      /* i2 = i1 / (\prod(iteration_space_extents)) */
      appendStatement (buildAssignStatement (variableDeclarations->getReference (
                  getIterationCounterVariableName (2)),
              buildIntegerDivideOp (variableDeclarations->getReference (
                      getIterationCounterVariableName (1)),
                  buildIntVal(div))),
          loopBody);

      /* i3 is extent of first entry in iteration space
       *
       * i3 = (i1 - i2 * \prod(iteration_space_extents))/(i3_extent)
       *
       * except when i3_extent is 1, in which case it's just the lower
       * limit */
      int * extent = itspace->getIterationDimensions ()[0];
      int i3_extent = extent[1] - extent[0];
      if (i3_extent == 1)
      {
        appendStatement (buildAssignStatement (variableDeclarations->getReference (
                    getIterationCounterVariableName (3)),
                buildIntVal (extent[0])),
            loopBody);
      }
      else
      {
        appendStatement (buildAssignStatement (variableDeclarations->getReference (
                    getIterationCounterVariableName (3)),
                buildIntegerDivideOp (buildSubtractOp (variableDeclarations->getReference (
                            getIterationCounterVariableName (1)),
                        buildMultiplyOp (variableDeclarations->getReference (
                                getIterationCounterVariableName (2)),
                            buildIntVal (div))),
                    buildIntVal (i3_extent))),
            loopBody);
      }

      /* i4 is extent of second entry in iteration space
       *
       * i4 = (i1 - i2 * \prod(iteration_space_extents) - i3 * i3_extent)
       *
       * except when i4_extent is 1, in which case it's just the lower
       * limit */
      extent = itspace->getIterationDimensions ()[1];
      int i4_extent = extent[1] - extent[0];
      if (i4_extent == 1)
      {
        appendStatement (buildAssignStatement (variableDeclarations->getReference (
                    getIterationCounterVariableName (4)),
                buildIntVal (extent[0])),
            loopBody);
      }
      else
      {
        appendStatement (buildAssignStatement (variableDeclarations->getReference (
                    getIterationCounterVariableName (4)),
                buildSubtractOp (variableDeclarations->getReference (
                        getIterationCounterVariableName (1)),
                    buildAddOp (
                        buildMultiplyOp (variableDeclarations->getReference (
                                getIterationCounterVariableName (2)),
                            buildIntVal (div)),
                        buildMultiplyOp (variableDeclarations->getReference (
                                getIterationCounterVariableName (3)),
                            buildIntVal (i3_extent))))),
            loopBody);
      }
    }

    createStageToVectorDatStatements (loopBody);

    appendStatement (createUserSubroutineCallStatement (), loopBody);

    SgVarRefExp * nthread = variableDeclarations->getReference (numberOfActiveThreads);

    if (itspace != NULL)
    {
      int mult = 1;
      std::vector<int *> itdims = itspace->getIterationDimensions ();
      for (unsigned int i = 0; i < itdims.size (); i++)
      {
        mult *= (itdims[i][1] - itdims[i][0]);
      }
      upperBoundExpression = buildLessThanOp (variableDeclarations->getReference (
              getIterationCounterVariableName (1)),
          buildMultiplyOp (nthread, buildIntVal (mult)));
    }
    else
    {
      upperBoundExpression = buildLessThanOp (variableDeclarations->getReference (
              getIterationCounterVariableName (1)), nthread);
    }
  }

  SgForStatement * forLoopStatement = buildForStatement (
      initialisationExpression, buildExprStatement (upperBoundExpression),
      strideExpression, loopBody);
  appendStatement (forLoopStatement, subroutineScope);
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createInitialiseCUDASharedVariablesStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise shared memory", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        /*
         * ======================================================
         * For loop body
         * ======================================================
         */

        SgBasicBlock * loopBody = buildBasicBlock ();

        if (parallelLoop->isIncremented (i))
        {
          SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
              variableDeclarations->getReference (
                  getIndirectOpDatSharedMemoryName (dat_num)),
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)));

          if (isSgTypeFloat (parallelLoop->getOpDatBaseType (i)))
          {
            SgExprStatement * assignmentStatement = buildAssignStatement (
                arrayExpression, buildFloatVal (0.0));

            appendStatement (assignmentStatement, loopBody);
          }
          else if (isSgTypeDouble (parallelLoop->getOpDatBaseType (i)))
          {
            SgExprStatement * assignmentStatement = buildAssignStatement (
                arrayExpression, buildDoubleVal (0));

            appendStatement (assignmentStatement, loopBody);
          }
          else
          {
            SgExprStatement * assignmentStatement = buildAssignStatement (
                arrayExpression, buildIntVal (0));

            appendStatement (assignmentStatement, loopBody);
          }
        }
        else if ( !parallelLoop->isWritten (i) )
        {
          SgDivideOp * divideExpression = buildDivideOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
              variableDeclarations->getReference (getIndirectOpDatMapName (dat_num)),
              divideExpression);

          SgMultiplyOp * multiplyExpression = buildMultiplyOp (
              arrayExpression1, buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgModOp * modulusExpression = buildModOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)), buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgAddOp * addExpression = buildAddOp (modulusExpression,
              multiplyExpression);

          SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
              variableDeclarations->getReference (getOpDatName (dat_num)),
              addExpression);

          SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
              variableDeclarations->getReference (
                  getIndirectOpDatSharedMemoryName (dat_num)),
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)));

          SgExprStatement * assignmentStatement = buildAssignStatement (
              arrayExpression3, arrayExpression2);

          appendStatement (assignmentStatement, loopBody);
        }

        /*
         * ======================================================
         * For loop statement
         * ======================================================
         */

        SgExprStatement * initialisationExpression = buildAssignStatement (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), CUDA::getThreadId (
                THREAD_X, subroutineScope));

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (getIndirectOpDatSizeName (dat_num)),
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgLessThanOp * upperBoundExpression = buildLessThanOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), multiplyExpression);

        SgPlusAssignOp * strideExpression = buildPlusAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

        SgForStatement * forStatement = buildForStatement (
            initialisationExpression,
            buildExprStatement (upperBoundExpression), strideExpression,
            loopBody);

        appendStatement (forStatement, block);
      }
    }
  }

  return block;
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createSetIndirectionMapPointerStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to set indirection map pointers",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  unsigned int offset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        /*
         * ======================================================
         * New statement
         * ======================================================
         */

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
            variableDeclarations->getReference (blockID), buildIntVal (
                parallelLoop->getNumberOfDistinctIndirectOpDats ()));

        SgAddOp * addExpression1a = buildAddOp (buildIntVal (offset),
            multiplyExpression1);

        SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (pindOffs), addExpression1a);

        SgAddOp * addExpression1b =
            buildAddOp (variableDeclarations->getReference (
                getLocalToGlobalMappingName (dat_num)), arrayExpression1);

        SgExprStatement * assignmentStatement1 = buildAssignStatement (
            variableDeclarations->getReference (getIndirectOpDatMapName (dat_num)),
            addExpression1b);

        appendStatement (assignmentStatement1, block);

        ++offset;
      }
    }
  }

  return block;
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createSetOpDatSharedMemoryPointerStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT shared memory pointers",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (nbytes), buildIntVal (0));

  appendStatement (assignmentStatement1, block);

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  unsigned int indirectOpDatCounter = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        /*
         * ======================================================
         * New statement
         * ======================================================
         */

        SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
            variableDeclarations->getReference (getSharedMemoryDeclarationName (
                parallelLoop->getUserSubroutineName ())),
            variableDeclarations->getReference (nbytes));

        SgAddressOfOp * addressOfExpression2 = buildAddressOfOp (
            arrayExpression2);

        SgCastExp * castExpression2 = buildCastExp (addressOfExpression2,
            buildPointerType (parallelLoop->getOpDatBaseType (i)));

        SgExprStatement * assignmentStatement2 = buildAssignStatement (
            variableDeclarations->getReference (
                getIndirectOpDatSharedMemoryName (dat_num)), castExpression2);

        appendStatement (assignmentStatement2, block);

        /*
         * ======================================================
         * New statement
         * ======================================================
         */

        indirectOpDatCounter++;

        if (indirectOpDatCounter
            < parallelLoop->getNumberOfDistinctIndirectOpDats ())
        {
          SgMultiplyOp * multiplyExpression3a = buildMultiplyOp (buildSizeOfOp (
              parallelLoop->getOpDatBaseType (i)), buildIntVal (
              parallelLoop->getOpDatDimension (i)));

          SgMultiplyOp * multiplyExpression3b =
              buildMultiplyOp (variableDeclarations->getReference (
                  getIndirectOpDatSizeName (dat_num)), multiplyExpression3a);

          SgFunctionCallExp * functionCallExpression3 =
              OP2::Macros::createRoundUpCallStatement (subroutineScope,
                  multiplyExpression3b);

          SgPlusAssignOp * assignmentStatement3 = buildPlusAssignOp (
              variableDeclarations->getReference (nbytes),
              functionCallExpression3);

          appendStatement (buildExprStatement (assignmentStatement3), block);
        }
      }
    }
  }

  return block;
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createIncrementAccessThreadZeroStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating thread zero statements for incremented OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgSubtractOp * subtractExpression1 = buildSubtractOp (
      variableDeclarations->getReference (numberOfActiveThreads), buildIntVal (
          1));

  SgDivideOp * divideExpression1 = buildDivideOp (subtractExpression1,
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

  SgExpression * addExpression1 = buildAddOp (buildIntVal (1),
      divideExpression1);

  SgMultiplyOp * multiplyExpression1 =
      buildMultiplyOp (
          CUDA::getThreadBlockDimension (THREAD_X, subroutineScope),
          addExpression1);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildOpaqueVarRefExp (numberOfActiveThreadsCeiling, subroutineScope),
      multiplyExpression1);

  appendStatement (assignmentStatement1, block);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (pnthrcol, subroutineScope),
      variableDeclarations->getReference (blockID));

  SgExprStatement * assignmentStatement2 =
      buildAssignStatement (buildOpaqueVarRefExp (numberOfColours,
          subroutineScope), arrayExpression2);

  appendStatement (assignmentStatement2, block);

  return block;
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createSetNumberOfIndirectElementsPerBlockStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise sizes of indirect OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  unsigned int offset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (blockID), buildIntVal (
                parallelLoop->getNumberOfDistinctIndirectOpDats ()));

        SgAddOp * addExpression = buildAddOp (buildIntVal (offset),
            multiplyExpression);

        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (pindSizes), addExpression);

        SgStatement * statement = buildAssignStatement (
            variableDeclarations->getReference (getIndirectOpDatSizeName (dat_num)),
            arrayExpression);

        appendStatement (statement, block);

        ++offset;
      }
    }
  }

  return block;
}

SgIfStmt *
CPPCUDAKernelSubroutineIndirectLoop::createThreadZeroStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage ("Creating thread zero statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * ifBlock = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgAddOp * arrayIndexExpression1 = buildAddOp (CUDA::getBlockId (BLOCK_X,
      subroutineScope), variableDeclarations->getReference (blockOffset));

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (pblkMap), arrayIndexExpression1);

  SgStatement * statement1 = buildAssignStatement (
      variableDeclarations->getReference (blockID), arrayExpression1);

  appendStatement (statement1, ifBlock);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (pnelems, subroutineScope),
      variableDeclarations->getReference (blockID));

  SgStatement * statement2 = buildAssignStatement (
      variableDeclarations->getReference (numberOfActiveThreads),
      arrayExpression2);

  appendStatement (statement2, ifBlock);

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  if (parallelLoop->hasIncrementedOpDats ())
  {
    appendStatementList (
        createIncrementAccessThreadZeroStatements ()->getStatementList (),
        ifBlock);
  }

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (poffset, subroutineScope),
      variableDeclarations->getReference (blockID));

  SgStatement * statement3 = buildAssignStatement (buildOpaqueVarRefExp (
      sharedMemoryOffset, subroutineScope), arrayExpression3);

  appendStatement (statement3, ifBlock);

  appendStatementList (
      createSetNumberOfIndirectElementsPerBlockStatements ()->getStatementList (),
      ifBlock);

  appendStatementList (
      createSetIndirectionMapPointerStatements ()->getStatementList (), ifBlock);

  appendStatementList (
      createSetOpDatSharedMemoryPointerStatements ()->getStatementList (),
      ifBlock);

  /*
   * ======================================================
   * Add the if statement with a NULL else block
   * ======================================================
   */

  SgEqualityOp * ifGuardExpression = buildEqualityOp (CUDA::getThreadId (
      THREAD_X, subroutineScope), buildIntVal (0));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBlock);

  return ifStatement;
}

void
CPPCUDAKernelSubroutineIndirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  if (parallelLoop->isReductionRequired ())
  {
    createReductionPrologueStatements ();
  }

  appendStatement (createThreadZeroStatements (), subroutineScope);

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      subroutineScope);

  appendStatementList (
      createInitialiseCUDASharedVariablesStatements ()->getStatementList (),
      subroutineScope);

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      subroutineScope);

  createExecutionLoopStatements ();

  appendStatementList (
      createIncrementAndWriteAccessEpilogueStatements ()->getStatementList (),
      subroutineScope);

  if (parallelLoop->isReductionRequired ())
  {
    createReductionEpilogueStatements ();
  }
}

void
CPPCUDAKernelSubroutineIndirectLoop::createIncrementAccessLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for incremented OP_DATS",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVariableDeclaration * variableDeclaration1 =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfColours, buildIntType (), subroutineScope);

  variableDeclarations->add (numberOfColours, variableDeclaration1);

  variableDeclaration1->get_declarationModifier ().get_storageModifier ().setCudaShared ();

  SgVariableDeclaration * variableDeclaration2 =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfActiveThreadsCeiling, buildIntType (), subroutineScope);

  variableDeclaration2->get_declarationModifier ().get_storageModifier ().setCudaShared ();

  variableDeclarations->add (numberOfActiveThreadsCeiling, variableDeclaration2);

  variableDeclarations ->add (colour1,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (colour1,
          buildIntType (), subroutineScope));

  variableDeclarations ->add (colour2,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (colour2,
          buildIntType (), subroutineScope));

  variableDeclarations ->add (
      getIterationCounterVariableName (2),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2), buildIntType (), subroutineScope));
}

void
CPPCUDAKernelSubroutineIndirectLoop::createExecutionLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed to execute kernel",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVariableDeclaration * variableDeclaration1 =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          sharedMemoryOffset, buildIntType (), subroutineScope);

  variableDeclaration1->get_declarationModifier ().get_storageModifier ().setCudaShared ();

  variableDeclarations->add (sharedMemoryOffset, variableDeclaration1);

  SgVariableDeclaration * variableDeclaration2 =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfActiveThreads, buildIntType (), subroutineScope);

  variableDeclaration2->get_declarationModifier ().get_storageModifier ().setCudaShared ();

  variableDeclarations->add (numberOfActiveThreads, variableDeclaration2);

  SgVariableDeclaration * variableDeclaration3 =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (nbytes,
          buildIntType (), subroutineScope);

  variableDeclaration3->get_declarationModifier ().get_storageModifier ().setCudaShared ();

  variableDeclarations ->add (nbytes, variableDeclaration3);

  variableDeclarations->add (blockID,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (blockID,
          buildIntType (), subroutineScope));

  variableDeclarations ->add (
      getIterationCounterVariableName (1),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1), buildIntType (), subroutineScope));
  if (parallelLoop->getNumberOfOpMatArgumentGroups () > 0)
  {
    variableDeclarations ->add (
        getIterationCounterVariableName (2),
        RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
            getIterationCounterVariableName (2), buildIntType (), subroutineScope));
    variableDeclarations ->add (
      getIterationCounterVariableName (3),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
        getIterationCounterVariableName (3), buildIntType (), subroutineScope));
    variableDeclarations ->add (
      getIterationCounterVariableName (4),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
        getIterationCounterVariableName (4), buildIntType (), subroutineScope));
  }
}

void
CPPCUDAKernelSubroutineIndirectLoop::createStageInVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local thread variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if ( (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
         || (parallelLoop->isGlobal (i) && !parallelLoop->isReductionRequired (i)))
    {
      string const & variableName = getOpDatLocalName (dat_num);

      variableDeclarations ->add (variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, buildArrayType (parallelLoop->getOpDatBaseType (i),
                  buildIntVal (parallelLoop->getOpDatDimension (i))),
              subroutineScope));
    }
  }
}

void
CPPCUDAKernelSubroutineIndirectLoop::createCUDASharedVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA shared variable declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  string const & variableName = getSharedMemoryDeclarationName (
      parallelLoop->getUserSubroutineName ());

  SgVariableDeclaration * autosharedVariableDeclaration =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName, buildArrayType (buildCharType ()), subroutineScope);

  autosharedVariableDeclaration->get_declarationModifier ().get_storageModifier ().setCudaDynamicShared ();

  variableDeclarations->add (variableName, autosharedVariableDeclaration);
}

void
CPPCUDAKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createStageInVariableDeclarations ();

  createCUDASharedVariableDeclarations ();

  createExecutionLocalVariableDeclarations ();

  if (parallelLoop->isReductionRequired ())
  {
    createReductionVariableDeclarations ();
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i))
    {
      continue;
    }
    else
    {
      unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
      if (parallelLoop->isDuplicateOpDat (i) == false)
      {
        if (parallelLoop->isIndirect (i))
        {
          Debug::getInstance ()->debugMessage (
            "Creating shared indirection mapping for OP_DAT " + lexical_cast <
            string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

          string const variableName = getIndirectOpDatMapName (dat_num);

          SgVariableDeclaration * variableDeclaration =
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, buildPointerType (buildIntType ()),
              subroutineScope);

          variableDeclaration->get_declarationModifier ().get_storageModifier ().setCudaShared ();

          variableDeclarations->add (variableName, variableDeclaration);

          if (parallelLoop->getOpIndexValue (i) < 0) // OP_ALL
          {
            string varname = getOpDatVecName (dat_num);
            SgType * datatype = parallelLoop->getOpDatBaseType (i);
            unsigned int mapdim;

            mapdim = declarations->getOpMapDefinition (parallelLoop->getOpMapName (i))->getDimension ();
            SgVariableDeclaration * dec =
              RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  varname, buildArrayType (buildPointerType (datatype),
                          buildIntVal (mapdim)),
                  subroutineScope);
            variableDeclarations->add (varname, dec);
          }
        }
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating indirection size argument for OP_DAT " + lexical_cast <
                string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

        string const variableName = getIndirectOpDatSizeName (dat_num);

        SgVariableDeclaration * variableDeclaration =
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildIntType (), subroutineScope);

        variableDeclaration->get_declarationModifier ().get_storageModifier ().setCudaShared ();

        variableDeclarations->add (variableName, variableDeclaration);
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating shared memory pointer for OP_DAT "
                + lexical_cast <string> (i), Debug::INNER_LOOP_LEVEL, __FILE__,
            __LINE__);

        string const variableName = getIndirectOpDatSharedMemoryName (dat_num);

        SgVariableDeclaration * variableDeclaration =
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildPointerType (parallelLoop->getOpDatBaseType (
                    i)), subroutineScope);

        variableDeclaration->get_declarationModifier ().get_storageModifier ().setCudaShared ();

        variableDeclarations->add (variableName, variableDeclaration);

        indirectOpDatSharedMemoryDeclarations[parallelLoop->getOpDatVariableName (
            i)] = variableDeclaration;
      }
    }
    else
    {
      Debug::getInstance ()->debugMessage (
          "New shared memory declaration NOT needed for OP_DAT "
              + lexical_cast <string> (i), Debug::INNER_LOOP_LEVEL, __FILE__,
          __LINE__);

      string const variableName = getIndirectOpDatSharedMemoryName (dat_num);

      variableDeclarations ->add (
          variableName,
          indirectOpDatSharedMemoryDeclarations[parallelLoop->getOpDatVariableName (
              i)]);
    }
  }

  if (parallelLoop->hasIncrementedOpDats ())
  {
    createIncrementAccessLocalVariableDeclarations ();
  }
}

void
CPPCUDAKernelSubroutineIndirectLoop::createPlanFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating plan formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (
      pindSizes,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pindSizes, buildPointerType (buildIntType ()), subroutineScope,
          formalParameters));

  variableDeclarations->add (
      pindOffs,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pindOffs, buildPointerType (buildIntType ()), subroutineScope,
          formalParameters));

  variableDeclarations->add (
      pblkMap,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pblkMap, buildPointerType (buildIntType ()), subroutineScope,
          formalParameters));

  variableDeclarations->add (
      poffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          poffset, buildPointerType (buildIntType ()), subroutineScope,
          formalParameters));

  variableDeclarations->add (
      pnelems,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pnelems, buildPointerType (buildIntType ()), subroutineScope,
          formalParameters));

  variableDeclarations->add (
      pnthrcol,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pnthrcol, buildPointerType (buildIntType ()), subroutineScope,
          formalParameters));

  variableDeclarations->add (
      pthrcol,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          pthrcol, buildPointerType (buildIntType ()), subroutineScope,
          formalParameters));

  variableDeclarations->add (
      blockOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          blockOffset, buildIntType (), subroutineScope, formalParameters));
}

void
CPPCUDAKernelSubroutineIndirectLoop::createOpDatFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace PlanFunctionVariableNames;
  using namespace OP2::RunTimeVariableNames;
  using std::string;
  using boost::lexical_cast;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i))
    {
      unsigned int mat_num = parallelLoop->getOpMatArgNum (i);
      string name = getOpMatName (mat_num);
      OpArgMatDefinition * arg_mat = parallelLoop->getOpMatArg (mat_num);
      OpMatDefinition * mat = declarations->getOpMatDefinition (arg_mat->getMatName ());

      variableDeclarations->add (
        name, RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          name, buildPointerType (mat->getBaseType ()), subroutineScope,
          formalParameters));

    }
    else
    {
      unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
      if (parallelLoop->isDuplicateOpDat (i) == false)
      {
        if (parallelLoop->isReductionRequired (i))
        {
          string const & variableName = getReductionArrayDeviceName (dat_num);

          variableDeclarations->add (
            variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName, buildPointerType (parallelLoop->getOpDatBaseType (
                                                i)), subroutineScope, formalParameters));
        }
        else if (parallelLoop->isIndirect (i) || parallelLoop->isDirect (i))
        {
          string const variableName = getOpDatName (dat_num);

          variableDeclarations->add (
            variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName, buildPointerType (parallelLoop->getOpDatBaseType (
                                                i)), subroutineScope, formalParameters));
        }
        else if (parallelLoop->isRead (i))
        {
          string const & variableName = getOpDatName (dat_num);

          if (parallelLoop->isPointer (i))
          {
            variableDeclarations->add (
              variableName,
              RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, buildPointerType (
                  parallelLoop->getOpDatBaseType (i)), subroutineScope,
                formalParameters));
          }
          else
          {
            variableDeclarations->add (
              variableName,
              RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, parallelLoop->getOpDatBaseType (i),
                subroutineScope, formalParameters));

          }
        }
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i))
    {
      continue;
    }
    else
    {
      unsigned int dat_num = parallelLoop->getOpDatArgNum (i);
      if (parallelLoop->isDuplicateOpDat (i) == false)
      {
        if (parallelLoop->isIndirect (i))
        {
          string const & variableName = getLocalToGlobalMappingName (dat_num);

          variableDeclarations->add (
            variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName, buildPointerType (buildIntType ()),
              subroutineScope, formalParameters));
        }
      }
    }
  }

  int count = 1;
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isIndirect (i))
    {
      int ntimes;
      if (parallelLoop->getOpIndexValue (i) < 0)
      {
        ntimes = declarations->getOpMapDefinition (parallelLoop->getOpMapName (i))->getDimension ();
      }
      else
      {
        ntimes = 1;
      }
      for ( int j = 0; j < ntimes; j++ )
      {
        string const & variableName = getGlobalToLocalMappingName (count++);

        variableDeclarations->add (
          variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            variableName, buildPointerType (buildShortType ()),
            subroutineScope, formalParameters));
      }
    }
  }
}

void
CPPCUDAKernelSubroutineIndirectLoop::createFormalParameterDeclarations ()
{
  Debug::getInstance ()->debugMessage (
      "Creating formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  createOpDatFormalParameterDeclarations ();

  createPlanFormalParameterDeclarations ();
}

CPPCUDAKernelSubroutineIndirectLoop::CPPCUDAKernelSubroutineIndirectLoop (
    SgScopeStatement * moduleScope, CPPCUDAUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPReductionSubroutines * reductionSubroutines,
    CPPProgramDeclarationsAndDefinitions * declarations) :
  CPPCUDAKernelSubroutine (moduleScope, userSubroutine, parallelLoop,
      reductionSubroutines)
{
  Debug::getInstance ()->debugMessage (
      "Creating kernel subroutine of indirect loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  this->declarations = declarations;

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
