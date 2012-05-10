


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


#include "CPPHostSubroutine.h"
#include "CPPParallelLoop.h"
#include "OP2Definitions.h"
#include "CPPProgramDeclarationsAndDefinitions.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "CompilerGeneratedNames.h"
#include "PlanFunctionNames.h"
#include "OP2.h"

SgBasicBlock *
CPPHostSubroutine::createInitialisePlanFunctionArrayStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace LoopVariableNames;
  using namespace PlanFunctionVariableNames;
  using std::map;
  using std::string;
  using boost::lexical_cast;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise plan function variables",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  unsigned int count = 0;
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    SgVarRefExp * var;
    unsigned int dat_num;
    if (parallelLoop->isOpMatArg (i))
    {
      continue;
      unsigned int mat_num = parallelLoop->getOpMatArgNum (i);
      var = variableDeclarations->getReference (getOpMatName (mat_num));
    }
    else
    {
      dat_num = parallelLoop->getOpDatArgNum (i);
      var = variableDeclarations->getReference (getOpDatName (dat_num));
    }
    if (!parallelLoop->isOpMatArg (i) && parallelLoop->isIndirect (i)
        && parallelLoop->getOpIndexValue (i) < 0)
    {
      int ntimes = declarations->getOpMapDefinition (parallelLoop->getOpMapName (i))->getDimension ();
      SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (opDatArray), buildIntVal (count++));

      SgExprStatement * mod = buildAssignStatement (buildDotExp (
              variableDeclarations->getReference (getOpDatName (dat_num)),
              buildOpaqueVarRefExp ("idx", subroutineScope)),
          buildIntVal (0));

      appendStatement (mod, block);
      SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayIndexExpression, var);

      appendStatement (assignmentStatement, block);
      for (int j = 1; j < ntimes; j++)
      {
        string tmpname = getOpDatName (dat_num) + "tmp" + lexical_cast <string> (j);
        variableDeclarations->add (tmpname,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                tmpname, buildOpaqueType (OP2::OP_ARG, subroutineScope),
                subroutineScope));

        SgExprListExp * parms = buildExprListExp ();
        parms->append_expression (var);
        parms->append_expression (buildAddressOfOp (
                variableDeclarations->getReference (tmpname)));
        SgStatement * funcall = buildFunctionCallStmt (
            "op_duplicate_arg", buildVoidType (), parms, subroutineScope);

        appendStatement (funcall, block);
        mod = buildAssignStatement (buildDotExp (
                variableDeclarations->getReference (tmpname),
                buildOpaqueVarRefExp ("idx", subroutineScope)),
            buildIntVal (j));
        appendStatement (mod, block);
        SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (opDatArray), buildIntVal (count++));

        var = variableDeclarations->getReference (tmpname);
        SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayIndexExpression, var);

        appendStatement (assignmentStatement, block);
      }
    }
    else
    {
        SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
          variableDeclarations->getReference (opDatArray), buildIntVal (count++));

        SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayIndexExpression, var);

        appendStatement (assignmentStatement, block);
    }
  }

  map <string, unsigned int> indirectOpDatsToIndirection;
  unsigned int indirection = 0;

  count = 0;
  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    if (parallelLoop->isOpMatArg (i))
    {
      continue;
    }
    SgPntrArrRefExp * arrayIndexExpression = buildPntrArrRefExp (
      variableDeclarations->getReference (indirectionDescriptorArray),
      buildIntVal (count++));

    SgExprStatement * assignmentStatement;

    if (parallelLoop->isIndirect (i))
    {
      if (parallelLoop->isDuplicateOpDat (i) == false)
      {
        assignmentStatement = buildAssignStatement (arrayIndexExpression,
                                                    buildIntVal (indirection));

        indirectOpDatsToIndirection[parallelLoop->getOpDatVariableName (i)]
          = indirection;

        if (parallelLoop->getOpIndexValue (i) < 0)
        {
          int ntimes = declarations->getOpMapDefinition (parallelLoop->getOpMapName (i))->getDimension ();
          for ( int j = 1; j < ntimes; j++ )
          {
            SgExprStatement * s = buildAssignStatement (
              buildPntrArrRefExp (
                variableDeclarations->getReference (indirectionDescriptorArray),
                buildIntVal(count++)),
              buildIntVal (indirection));
            appendStatement (s, block);
          }
        }
        indirection++;
      }
      else
      {
        assignmentStatement = buildAssignStatement (arrayIndexExpression,
                                                    buildIntVal (
                                                      indirectOpDatsToIndirection[parallelLoop->getOpDatVariableName (i)]));
        if (parallelLoop->getOpIndexValue (i) < 0)
        {
          int ntimes = declarations->getOpMapDefinition (parallelLoop->getOpMapName (i))->getDimension ();
          for ( int j = 1; j < ntimes; j++ )
          {
            SgExprStatement * s = buildAssignStatement (
              buildPntrArrRefExp (
                variableDeclarations->getReference (indirectionDescriptorArray),
                buildIntVal(count++)),
              buildIntVal (indirectOpDatsToIndirection[parallelLoop->getOpDatVariableName (i)]));
            appendStatement (s, block);
          }
        }
      }
    }
    else
    {
      assignmentStatement = buildAssignStatement (arrayIndexExpression,
                                                  buildIntVal (-1));
    }
    appendStatement (assignmentStatement, block);
  }

  return block;
}

void
CPPHostSubroutine::createFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using namespace OP2VariableNames;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating host subroutine formal parameters", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Add name of user kernel
   * ======================================================
   */

  string const & kernelVariableName = getUserSubroutineName ();

  SgVariableDeclaration
      * kernelVariableNameDeclaration =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              kernelVariableName, buildPointerType (buildConstType (
                  buildCharType ())), subroutineScope, formalParameters);

  variableDeclarations->add (kernelVariableName, kernelVariableNameDeclaration);

  /*
   * ======================================================
   * Add OP_SET
   * ======================================================
   */

  string const & opSetVariableName = getOpSetName ();

  variableDeclarations->add (
      opSetVariableName,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          opSetVariableName, buildOpaqueType (OP2::OP_SET, subroutineScope),
          subroutineScope, formalParameters));

  /*
   * ======================================================
   * Add OP_DAT, indirection, OP_MAP, access arguments for
   * each OP_DAT argument group
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfArgumentGroups (); ++i)
  {
    string variableName;
    if (parallelLoop->isOpMatArg (i))
    {
      variableName = getOpMatName (parallelLoop->getOpMatArgNum (i));
    }
    else
    {
      variableName = getOpDatName (parallelLoop->getOpDatArgNum (i));
    }

    variableDeclarations->add (
        variableName,
        RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            variableName, buildOpaqueType (OP2::OP_ARG, subroutineScope),
            subroutineScope, formalParameters));
  }
}

SgFunctionParameterList *
CPPHostSubroutine::getCopyOfFormalParameters ()
{
  using namespace SageInterface;

  return deepCopy (formalParameters);
}

CPPHostSubroutine::CPPHostSubroutine (SgScopeStatement * moduleScope,
    Subroutine <SgFunctionDeclaration> * calleeSubroutine,
    CPPParallelLoop * parallelLoop) :
  HostSubroutine <SgFunctionDeclaration> (calleeSubroutine, parallelLoop)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  subroutineHeaderStatement = buildDefiningFunctionDeclaration (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  appendStatement (subroutineHeaderStatement, moduleScope);
}

CPPHostSubroutine::CPPHostSubroutine (SgScopeStatement * moduleScope,
    Subroutine <SgFunctionDeclaration> * calleeSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPProgramDeclarationsAndDefinitions * declarations) :
  HostSubroutine <SgFunctionDeclaration> (calleeSubroutine, parallelLoop)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  subroutineHeaderStatement = buildDefiningFunctionDeclaration (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  appendStatement (subroutineHeaderStatement, moduleScope);

  this->declarations = declarations;
}
