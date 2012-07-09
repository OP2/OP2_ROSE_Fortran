


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


#include "CPPModuleDeclarations.h"
#include "CPPProgramDeclarationsAndDefinitions.h"
#include "CPPParallelLoop.h"
#include "ScopedVariableDeclarations.h"
#include "Debug.h"
#include "CompilerGeneratedNames.h"
#include <rose.h>
#include "OP2.h"

void
CPPModuleDeclarations::addParallelLoopSettingsVariables (
    SgScopeStatement * moduleScope,
    CPPProgramDeclarationsAndDefinitions * declarations)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::Macros;
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Adding variables to file to control OP2 block size and partition size settings",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVariableDeclaration * variableDeclaration1;

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    CPPParallelLoop * parallelLoop =
        static_cast <CPPParallelLoop *> (it->second);

    std::string const & variableName1 = getBlockSizeVariableName (
        userSubroutineName);

    variableDeclaration1 = buildVariableDeclaration (
        variableName1, buildIntType (), buildAssignInitializer (buildIntVal (
            512), buildIntType ()), moduleScope);

    variableDeclarations ->add (variableName1, variableDeclaration1);

    appendStatement (variableDeclaration1, moduleScope);

    if (parallelLoop->isDirectLoop () == false)
    {
      std::string const & variableName2 = getPartitionSizeVariableName (
          userSubroutineName);

      SgVariableDeclaration * variableDeclaration2 = buildVariableDeclaration (
          variableName2, buildIntType (), buildAssignInitializer (buildIntVal (
              512), buildIntType ()), moduleScope);

      variableDeclarations ->add (variableName2, variableDeclaration2);

      appendStatement (variableDeclaration2, moduleScope);
    }
  } 
   
    string opWarpsize = warpSizeMacro + "_0";

    string fullLine = "\n#define " + warpSizeMacro + " " + opWarpsize + "\n";
    string temp = "\n#ifdef " + opWarpsize + "\n" + fullLine + "\n#endif\n";
/*    addTextForUnparser (variableDeclaration1, temp,
        AstUnparseAttribute::e_after);*/

    addTextForUnparser (variableDeclaration1, temp,
        AstUnparseAttribute::e_after);

/*    addTextForUnparser (variableDeclaration1, "\n#endif\n", 
        AstUnparseAttribute::e_after);*/
}

ScopedVariableDeclarations *
CPPModuleDeclarations::getDeclarations ()
{
  return variableDeclarations;
}

CPPModuleDeclarations::CPPModuleDeclarations (SgScopeStatement * moduleScope,
    CPPProgramDeclarationsAndDefinitions * declarations)
{
  variableDeclarations = new ScopedVariableDeclarations ();

  addParallelLoopSettingsVariables (moduleScope, declarations);
}
