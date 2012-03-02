


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

#include <boost/algorithm/string.hpp>
#include "CPPOpenMPKernelSubroutineIndirectLoop.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "CompilerGeneratedNames.h"
#include "PlanFunctionNames.h"
#include "OP2.h"
#include "OpenMP.h"

SgStatement *
CPPOpenMPKernelSubroutineIndirectLoop::createUserSubroutineCallStatement ()
{
  using boost::iequals;
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage ("Creating call to user kernel",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isReductionRequired (i))
    {
      actualParameters->append_expression (variableDeclarations->getReference (
          getOpDatName (i)));
    }
    else if (parallelLoop->isGlobal (i))
    {
      actualParameters->append_expression (variableDeclarations->getReference (
          getOpDatName (i)));
    }
    else if (parallelLoop->isIndirect (i))
    {
      if (parallelLoop->isIncremented (i))
      {
        actualParameters->append_expression (
            variableDeclarations->getReference (getOpDatLocalName (i)));
      }
      else
      {
        SgAddOp * addExpression1 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            variableDeclarations->getReference (OpenMP::threadBlockOffset));

        SgPntrArrRefExp * arrayExpression1 =
            buildPntrArrRefExp (variableDeclarations->getReference (
                getGlobalToLocalMappingName (i)), addExpression1);

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (arrayExpression1,
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgAddOp
            * addExpression2 =
                buildAddOp (
                    buildVarRefExp (
                        sharedIndirectionDeclarations[parallelLoop->getOpDatVariableName (
                            i)]), multiplyExpression);

        actualParameters->append_expression (addExpression2);
      }
    }
    else
    {
      SgAddOp * addExpression1 = buildAddOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), variableDeclarations->getReference (
              OpenMP::threadBlockOffset));

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (addExpression1,
          buildIntVal (parallelLoop->getOpDatDimension (i)));

/*		
      SgAddOp * addExpression2 = buildAddOp (
          variableDeclarations->getReference (getOpDatName (i)),
          multiplyExpression);
*/
		// REPLACED BY
		
		SgAddOp * addExpression2 = NULL;
		
		if (parallelLoop->isDuplicateOpDat (i))
		{
			int oj;
			for (oj = 1; oj < i; oj++)
			{
				if (iequals (parallelLoop->getOpDatVariableName (oj), parallelLoop->getOpDatVariableName (i)) )
					break;
			}
			
			addExpression2 = buildAddOp (
										 variableDeclarations->getReference (getOpDatName (oj)),
										 multiplyExpression);
		}
		else
		{
			addExpression2 = buildAddOp (
										 variableDeclarations->getReference (getOpDatName (i)),
										 multiplyExpression);
		}
		
		// END REPLACE

      actualParameters->append_expression (addExpression2);
    }
  }

  SgFunctionCallExp * userSubroutineCall = buildFunctionCallExp (
      userSubroutine->getSubroutineName (), buildVoidType (), actualParameters,
      subroutineScope);

  return buildExprStatement (userSubroutineCall);
}

SgBasicBlock *
CPPOpenMPKernelSubroutineIndirectLoop::createStageOutIncrementedOpDatStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to stage out incremented OP_DATs after kernel execution",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      SgAddOp * addExpression = buildAddOp (variableDeclarations->getReference (
          getIterationCounterVariableName (1)),
          variableDeclarations->getReference (OpenMP::threadBlockOffset));

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (getGlobalToLocalMappingName (i)),
          addExpression);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          variableDeclarations->getReference (getIncrementAccessMapName (i)),
          arrayExpression);

      appendStatement (assignmentStatement, block);
    }
  }

  SgBasicBlock * ifBody = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      SgBasicBlock * innerLoopBody = buildBasicBlock ();

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (
          variableDeclarations->getReference (getIncrementAccessMapName (i)),
          buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgAddOp * addExpression1 = buildAddOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), multiplyExpression);

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
          buildVarRefExp (
              sharedIndirectionDeclarations[parallelLoop->getOpDatVariableName (
                  i)]), addExpression1);

      SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatLocalName (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgPlusAssignOp * assignmentStatement = buildPlusAssignOp (
          arrayExpression1, arrayExpression3);

      appendStatement (buildExprStatement (assignmentStatement), innerLoopBody);

      SgExprStatement * innerLoopinitialisation = buildAssignStatement (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgLessThanOp * innerLoopUppperBound = buildLessThanOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgPlusPlusOp * innerLoopStrideExpression = buildPlusPlusOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgForStatement * innerLoopStatement = buildForStatement (
          innerLoopinitialisation, buildExprStatement (innerLoopUppperBound),
          innerLoopStrideExpression, innerLoopBody);

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

  SgExprStatement * loopinitialisation = buildAssignStatement (
      variableDeclarations->getReference (colour1), buildIntVal (0));

  SgLessThanOp * loopUppperBound = buildLessThanOp (
      variableDeclarations->getReference (colour1),
      variableDeclarations->getReference (numberOfColours));

  SgPlusPlusOp * loopStrideExpression = buildPlusPlusOp (
      variableDeclarations->getReference (colour1));

  SgForStatement * loopStatement = buildForStatement (loopinitialisation,
      buildExprStatement (loopUppperBound), loopStrideExpression, loopBody);

  appendStatement (loopStatement, block);

  return block;
}

SgBasicBlock *
CPPOpenMPKernelSubroutineIndirectLoop::createIncrementedOpDatPrologueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise local variables of incremented OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isIndirect (i) && parallelLoop->isIncremented (i))
    {
      SgBasicBlock * loopBody = buildBasicBlock ();

      SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (getOpDatLocalName (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression, buildIntVal (0));

      appendStatement (assignmentStatement, loopBody);

      SgExprStatement * loopinitialisation = buildAssignStatement (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (0));

      SgLessThanOp * loopUppperBound = buildLessThanOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)), buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgPlusPlusOp * loopStrideExpression = buildPlusPlusOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              2)));

      SgForStatement * loopStatement = buildForStatement (loopinitialisation,
          buildExprStatement (loopUppperBound), loopStrideExpression, loopBody);

      appendStatement (loopStatement, block);
    }
  }

  return block;
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createExecutionLoopStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating main execution loop statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * loopBody = buildBasicBlock ();

  if (parallelLoop->hasIncrementedOpDats ())
  {
    SgExprStatement * assignmentStatement1 = buildAssignStatement (
        variableDeclarations->getReference (colour2), buildIntVal (-1));

    appendStatement (assignmentStatement1, loopBody);

    SgBasicBlock * ifBody = buildBasicBlock ();

    appendStatementList (
        createIncrementedOpDatPrologueStatements ()->getStatementList (),
        ifBody);

    appendStatement (createUserSubroutineCallStatement (), ifBody);

    SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
        getIterationCounterVariableName (1)),
        variableDeclarations->getReference (OpenMP::threadBlockOffset));

    SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
        variableDeclarations->getReference (getThreadColourArrayName ()),
        addExpression1);

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
        createStageOutIncrementedOpDatStatements ()->getStatementList (),
        loopBody);

    SgExprStatement * loopinitialisation =
        buildAssignStatement (variableDeclarations->getReference (
            getIterationCounterVariableName (1)), buildIntVal (0));

    SgLessThanOp * loopUppperBound =
        buildLessThanOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)),
            variableDeclarations->getReference (numberOfActiveThreadsCeiling));

    SgPlusPlusOp * loopStrideExpression =
        buildPlusPlusOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)));

    SgForStatement * loopStatement = buildForStatement (loopinitialisation,
        buildExprStatement (loopUppperBound), loopStrideExpression, loopBody);

    appendStatement (loopStatement, subroutineScope);
  }
  else
  {
    appendStatement (createUserSubroutineCallStatement (), loopBody);

    SgExprStatement * loopinitialisation =
        buildAssignStatement (variableDeclarations->getReference (
            getIterationCounterVariableName (1)), buildIntVal (0));

    SgLessThanOp * loopUppperBound =
        buildLessThanOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)),
            variableDeclarations->getReference (numberOfActiveThreads));

    SgPlusPlusOp * loopStrideExpression =
        buildPlusPlusOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)));

    SgForStatement * loopStatement = buildForStatement (loopinitialisation,
        buildExprStatement (loopUppperBound), loopStrideExpression, loopBody);

    appendStatement (loopStatement, subroutineScope);
  }

}

void
CPPOpenMPKernelSubroutineIndirectLoop::createIncrementedOpDatEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to increment indirect OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIncremented (i))
      {
        /*
         * ======================================================
         * Inner loop
         * ======================================================
         */

        SgBasicBlock * innerLoopBody = buildBasicBlock ();

        SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (getIndirectOpDatMapName (i)),
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)));

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (arrayExpression1,
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression2 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)), multiplyExpression1);

        SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
            variableDeclarations->getReference (getOpDatName (i)),
            addExpression2);

        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression3 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)), multiplyExpression2);

        SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (
            variableDeclarations->getReference (
                getIndirectOpDatSharedMemoryName (i)), addExpression3);

        SgPlusAssignOp * assignmentStatement = buildPlusAssignOp (
            arrayExpression2, arrayExpression4);

        appendStatement (buildExprStatement (assignmentStatement),
            innerLoopBody);

        SgExprStatement * innerLoopinitialisation = buildAssignStatement (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)), buildIntVal (0));

        SgLessThanOp * innerLoopUppperBound = buildLessThanOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)), buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgPlusPlusOp * innerLoopStrideExpression = buildPlusPlusOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)));

        SgForStatement * innerLoopStatement = buildForStatement (
            innerLoopinitialisation, buildExprStatement (innerLoopUppperBound),
            innerLoopStrideExpression, innerLoopBody);

        /*
         * ======================================================
         * Outer loop
         * ======================================================
         */

        SgBasicBlock * outerLoopBody = buildBasicBlock ();

        appendStatement (innerLoopStatement, outerLoopBody);

        SgExprStatement * outerLoopinitialisation = buildAssignStatement (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (0));

        SgLessThanOp * outerLoopUppperBound = buildLessThanOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            variableDeclarations->getReference (getIndirectOpDatSizeName (i)));

        SgPlusPlusOp * outerLoopStrideExpression = buildPlusPlusOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)));

        SgForStatement * outerLoopStatement = buildForStatement (
            outerLoopinitialisation, buildExprStatement (outerLoopUppperBound),
            outerLoopStrideExpression, outerLoopBody);

        appendStatement (outerLoopStatement, subroutineScope);
      }
    }
  }
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createStageInStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to stage in indirect OP_DATs into shared memory",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        /*
         * ======================================================
         * Inner loop
         * ======================================================
         */

        SgBasicBlock * innerLoopBody = buildBasicBlock ();

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression1 = buildAddOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)), multiplyExpression1);

        SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (
                getIndirectOpDatSharedMemoryName (i)), addExpression1);

        if (parallelLoop->isIncremented (i))
        {
          SgExprStatement * assignmentStatement = buildAssignStatement (
              arrayExpression1, buildIntVal (0));

          appendStatement (assignmentStatement, innerLoopBody);
        }
        else
        {
          SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
              variableDeclarations->getReference (getIndirectOpDatMapName (i)),
              variableDeclarations->getReference (
                  getIterationCounterVariableName (1)));

          SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
              arrayExpression2, buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgAddOp * addExpression4 = buildAddOp (
              variableDeclarations->getReference (
                  getIterationCounterVariableName (2)), multiplyExpression2);

          SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
              variableDeclarations->getReference (getOpDatName (i)),
              addExpression4);

          SgExprStatement * assignmentStatement = buildAssignStatement (
              arrayExpression1, arrayExpression3);

          appendStatement (assignmentStatement, innerLoopBody);
        }

        SgExprStatement * innerLoopinitialisation = buildAssignStatement (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)), buildIntVal (0));

        SgLessThanOp * innerLoopUppperBound = buildLessThanOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)), buildIntVal (
                parallelLoop->getOpDatDimension (i)));

        SgPlusPlusOp * innerLoopStrideExpression = buildPlusPlusOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (2)));

        SgForStatement * innerLoopStatement = buildForStatement (
            innerLoopinitialisation, buildExprStatement (innerLoopUppperBound),
            innerLoopStrideExpression, innerLoopBody);

        /*
         * ======================================================
         * Outer loop
         * ======================================================
         */

        SgBasicBlock * outerLoopBody = buildBasicBlock ();

        appendStatement (innerLoopStatement, outerLoopBody);

        SgExprStatement * outerLoopinitialisation = buildAssignStatement (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (0));

        SgLessThanOp * outerLoopUppperBound = buildLessThanOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)),
            variableDeclarations->getReference (getIndirectOpDatSizeName (i)));

        SgPlusPlusOp * outerLoopStrideExpression = buildPlusPlusOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)));

        SgForStatement * outerLoopStatement = buildForStatement (
            outerLoopinitialisation, buildExprStatement (outerLoopUppperBound),
            outerLoopStrideExpression, outerLoopBody);

        appendStatement (outerLoopStatement, subroutineScope);
      }
    }
  }
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createInitialiseSharedVariableStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise shared variables of indirect OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  bool firstIndirectOp = true;
  unsigned int previousOpDat;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        previousOpDat = i;

        if (firstIndirectOp)
        {
          firstIndirectOp = false;

          SgExprStatement * assignmentStatement = buildAssignStatement (
              variableDeclarations->getReference (nbytes), buildIntVal (0));

          appendStatement (assignmentStatement, subroutineScope);
        }
        else
        {
          SgSizeOfOp * sizeOfExpression = buildSizeOfOp (
              parallelLoop->getOpDatBaseType (previousOpDat));

          SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
              sizeOfExpression, buildIntVal (parallelLoop->getOpDatDimension (
                  previousOpDat)));

          SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
              variableDeclarations->getReference (getIndirectOpDatSizeName (
                  previousOpDat)), multiplyExpression1);

          SgFunctionCallExp * functionCallExpression =
              OP2::Macros::createRoundUpCallStatement (subroutineScope,
                  multiplyExpression2);

          SgPlusAssignOp * assignmentStatement = buildPlusAssignOp (
              variableDeclarations->getReference (nbytes),
              functionCallExpression);

          appendStatement (buildExprStatement (assignmentStatement),
              subroutineScope);
        }

        string const & sharedVariableName = getSharedMemoryDeclarationName (
            parallelLoop->getOpDatBaseType (i),
            parallelLoop->getSizeOfOpDat (i));

        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (sharedVariableName),
            variableDeclarations->getReference (nbytes));

        SgAddressOfOp * addressOfExpresion = buildAddressOfOp (arrayExpression);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (
                getIndirectOpDatSharedMemoryName (i)), addressOfExpresion);

        appendStatement (assignmentStatement, subroutineScope);
      }
    }
  }
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createInitialiseIndirectOpDatMapsStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise indirect OP_DAT maps",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  unsigned int offset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (OpenMP::threadBlockID),
            buildIntVal (parallelLoop->getNumberOfDistinctIndirectOpDats ()));

        SgAddOp * addExpression1 = buildAddOp (buildIntVal (offset),
            multiplyExpression);

        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (
                getIndirectOpDatsOffsetArrayName ()), addExpression1);

        SgAddOp * addExpression2 =
            buildAddOp (variableDeclarations->getReference (
                getLocalToGlobalMappingName (i)), arrayExpression);

        SgExprStatement * assignStatement = buildAssignStatement (
            variableDeclarations->getReference (getIndirectOpDatMapName (i)),
            addExpression2);

        appendStatement (assignStatement, subroutineScope);

        offset++;
      }
    }
  }
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createInitialiseIndirectOpDatSizesStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise sizes of indirect OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  unsigned int offset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (OpenMP::threadBlockID),
            buildIntVal (parallelLoop->getNumberOfDistinctIndirectOpDats ()));

        SgAddOp * addExpression = buildAddOp (buildIntVal (offset),
            multiplyExpression);

        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (
                getIndirectOpDatsNumberOfElementsArrayName ()), addExpression);

        SgExprStatement * assignStatement = buildAssignStatement (
            variableDeclarations->getReference (getIndirectOpDatSizeName (i)),
            arrayExpression);

        appendStatement (assignStatement, subroutineScope);

        offset++;
      }
    }
  }
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createInitialiseIncrementAccessVariablesStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise thread-specific variables needed for incremented OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Statement to initialise ceiling of number of active
   * threads
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (numberOfActiveThreadsCeiling),
      variableDeclarations->getReference (numberOfActiveThreads));

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * Statement to initialise number of colours
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (
          getNumberOfThreadColoursPerBlockArrayName ()),
      variableDeclarations->getReference (OpenMP::threadBlockID));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (numberOfColours), arrayExpression2);

  appendStatement (assignmentStatement2, subroutineScope);
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createInitialiseThreadVariablesStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace PlanFunctionVariableNames;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialse thread-specific variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * Statement to initialise thread-block ID
   * ======================================================
   */

  SgAddOp * addExpression1 = buildAddOp (variableDeclarations->getReference (
      blockID), variableDeclarations->getReference (blockOffset));

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (getColourToBlockArrayName ()),
      addExpression1);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OpenMP::threadBlockID),
      arrayExpression1);

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * Statement to initialise number of active threads
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      variableDeclarations->getReference (
          getNumberOfSetElementsPerBlockArrayName ()),
      variableDeclarations->getReference (OpenMP::threadBlockID));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (numberOfActiveThreads),
      arrayExpression2);

  appendStatement (assignmentStatement2, subroutineScope);

  /*
   * ======================================================
   * Statement to initialise offset into shared memory
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      variableDeclarations->getReference (getOffsetIntoBlockArrayName ()),
      variableDeclarations->getReference (OpenMP::threadBlockID));

  SgExprStatement * assignmentStatement3 = buildAssignStatement (
      variableDeclarations->getReference (OpenMP::threadBlockOffset),
      arrayExpression3);

  appendStatement (assignmentStatement3, subroutineScope);
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createStatements ()
{
  Debug::getInstance ()->debugMessage ("Creating statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createInitialiseThreadVariablesStatements ();

  if (parallelLoop->hasIncrementedOpDats ())
  {
    createInitialiseIncrementAccessVariablesStatements ();
  }

  createInitialiseIndirectOpDatSizesStatements ();

  createInitialiseIndirectOpDatMapsStatements ();

  createInitialiseSharedVariableStatements ();

  createStageInStatements ();

  createExecutionLoopStatements ();

  if (parallelLoop->hasIncrementedOpDats ())
  {
    createIncrementedOpDatEpilogueStatements ();
  }
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createIncrementAccessLocalVariableDeclarations ()
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

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isIncremented (i))
    {
      Debug::getInstance ()->debugMessage (
          "Creating local variables for incremented OP_DAT " + lexical_cast <
              string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

      string const variableName1 = getOpDatLocalName (i);

      variableDeclarations->add (variableName1,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName1, buildArrayType (
                  parallelLoop->getOpDatBaseType (i), buildIntVal (
                      parallelLoop->getOpDatDimension (i))), subroutineScope));

      string const variableName2 = getIncrementAccessMapName (i);

      variableDeclarations->add (variableName2,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName2, buildIntType (), subroutineScope));
    }
  }

  variableDeclarations->add (numberOfColours,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfColours, buildIntType (), subroutineScope));

  variableDeclarations->add (numberOfActiveThreadsCeiling,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfActiveThreadsCeiling, buildIntType (), subroutineScope));

  variableDeclarations ->add (colour1,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (colour1,
          buildIntType (), subroutineScope));

  variableDeclarations ->add (colour2,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (colour2,
          buildIntType (), subroutineScope));
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createIndirectOpDatSizeLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using boost::lexical_cast;
  using std::string;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating indirection size argument for OP_DAT " + lexical_cast <
                string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

        string const variableName = getIndirectOpDatSizeName (i);

        variableDeclarations->add (variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildIntType (), subroutineScope));
      }
    }
  }
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createSharedVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;
  using std::find;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating shared variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector <string> autosharedNames;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const & variableName1 = getIndirectOpDatMapName (i);

        variableDeclarations->add (variableName1,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName1, buildPointerType (buildIntType ()),
                subroutineScope));

        string const & variableName2 = getIndirectOpDatSharedMemoryName (i);

        SgVariableDeclaration * variableDeclaration =
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName2, buildPointerType (
                    parallelLoop->getOpDatBaseType (i)), subroutineScope);

        variableDeclarations->add (variableName2, variableDeclaration);

        sharedIndirectionDeclarations[parallelLoop->getOpDatVariableName (i)]
            = variableDeclaration;

        string const & sharedVariableName = getSharedMemoryDeclarationName (
            parallelLoop->getOpDatBaseType (i),
            parallelLoop->getSizeOfOpDat (i));

        if (find (autosharedNames.begin (), autosharedNames.end (),
            sharedVariableName) == autosharedNames.end ())
        {
          Debug::getInstance ()->debugMessage (
              "Creating declaration with name '" + sharedVariableName
                  + "' for OP_DAT '" + parallelLoop->getOpDatVariableName (i)
                  + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          variableDeclarations->add (sharedVariableName,
              RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  sharedVariableName, buildArrayType (
                      parallelLoop->getOpDatBaseType (i), buildIntVal (64000)),
                  subroutineScope));

          autosharedNames.push_back (sharedVariableName);
        }
      }
    }
  }
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createExecutionLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed to execute kernel",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (OpenMP::threadBlockOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::threadBlockOffset, buildIntType (), subroutineScope));

  variableDeclarations->add (OpenMP::threadBlockID,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OpenMP::threadBlockID, buildIntType (), subroutineScope));

  variableDeclarations->add (numberOfActiveThreads,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          numberOfActiveThreads, buildIntType (), subroutineScope));

  variableDeclarations ->add (nbytes,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (nbytes,
          buildIntType (), subroutineScope));

  variableDeclarations ->add (
      getIterationCounterVariableName (1),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1), buildIntType (), subroutineScope));

  variableDeclarations ->add (
      getIterationCounterVariableName (2),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2), buildIntType (), subroutineScope));
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createExecutionLocalVariableDeclarations ();

  createSharedVariableDeclarations ();

  createIndirectOpDatSizeLocalVariableDeclarations ();

  if (parallelLoop->hasIncrementedOpDats ())
  {
    createIncrementAccessLocalVariableDeclarations ();
  }
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createPlanFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace PlanFunctionVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating plan formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (
      getIndirectOpDatsNumberOfElementsArrayName (),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          getIndirectOpDatsNumberOfElementsArrayName (), buildPointerType (
              buildIntType ()), subroutineScope, formalParameters));

  variableDeclarations->add (
      getIndirectOpDatsOffsetArrayName (),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          getIndirectOpDatsOffsetArrayName (), buildPointerType (
              buildIntType ()), subroutineScope, formalParameters));

  variableDeclarations->add (
      getColourToBlockArrayName (),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          getColourToBlockArrayName (), buildPointerType (buildIntType ()),
          subroutineScope, formalParameters));

  variableDeclarations->add (
      getOffsetIntoBlockArrayName (),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          getOffsetIntoBlockArrayName (), buildPointerType (buildIntType ()),
          subroutineScope, formalParameters));

  variableDeclarations->add (
      getNumberOfSetElementsPerBlockArrayName (),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          getNumberOfSetElementsPerBlockArrayName (), buildPointerType (
              buildIntType ()), subroutineScope, formalParameters));

  variableDeclarations->add (
      getThreadColourArrayName (),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          getThreadColourArrayName (), buildPointerType (buildIntType ()),
          subroutineScope, formalParameters));

  variableDeclarations->add (
      getNumberOfThreadColoursPerBlockArrayName (),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          getNumberOfThreadColoursPerBlockArrayName (), buildPointerType (
              buildIntType ()), subroutineScope, formalParameters));

  variableDeclarations->add (
      blockOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          blockOffset, buildIntType (), subroutineScope, formalParameters));

  variableDeclarations->add (
      blockID,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          blockID, buildIntType (), subroutineScope, formalParameters));
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createOpDatFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace PlanFunctionVariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const variableName = getOpDatName (i);
		
      variableDeclarations->add (
          variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName, buildPointerType (
                  parallelLoop->getOpDatBaseType (i)), subroutineScope,
              formalParameters));
    }
	else
	{
		
		/*
		 * Duplicated direct op_arg_dat
		 */
/*
		if (parallelLoop->isDirect (i))
		{
			string const variableName = getOpDatName (i);
			
			variableDeclarations->add (
									   variableName,
									   RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (variableName,
																														buildPointerType (
																																		  parallelLoop->getOpDatBaseType (i)),
																														subroutineScope,
																														formalParameters)
									   );
																														
									   
		}
*/
	}
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const & variableName = getLocalToGlobalMappingName (i);

        variableDeclarations->add (
            variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, buildPointerType (buildIntType ()),
                subroutineScope, formalParameters));
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i)) continue;
    if (parallelLoop->isIndirect (i))
    {
      string const & variableName = getGlobalToLocalMappingName (i);

      variableDeclarations->add (
          variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName, buildPointerType (buildShortType ()),
              subroutineScope, formalParameters));
    }
  }
}

void
CPPOpenMPKernelSubroutineIndirectLoop::createFormalParameterDeclarations ()
{
  Debug::getInstance ()->debugMessage (
      "Creating formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  createOpDatFormalParameterDeclarations ();

  createPlanFormalParameterDeclarations ();
}

CPPOpenMPKernelSubroutineIndirectLoop::CPPOpenMPKernelSubroutineIndirectLoop (
    SgScopeStatement * moduleScope, CPPUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop) :
  CPPOpenMPKernelSubroutine (moduleScope, userSubroutine, parallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "Creating kernel subroutine of indirect loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}
