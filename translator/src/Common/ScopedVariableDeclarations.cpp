


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


#include <ScopedVariableDeclarations.h>
#include <Exceptions.h>
#include <rose.h>

SgVarRefExp *
ScopedVariableDeclarations::getReference (std::string const & variableName)
{
  using namespace SageBuilder;

  if (theDeclarations.count (variableName) == 0)
  {
    throw Exceptions::CodeGeneration::UnknownVariableException (
        "Unable to find '" + variableName + "' in variable declarations");
  }

  return buildVarRefExp (theDeclarations[variableName]);
}

void
ScopedVariableDeclarations::add (std::string const & variableName,
    SgVariableDeclaration * declaration)
{
  if (theDeclarations.count (variableName) != 0)
  {
    throw Exceptions::CodeGeneration::DuplicateVariableException ("'"
        + variableName + "' has already been declared");
  }

  theDeclarations[variableName] = declaration;
}

void
ScopedVariableDeclarations::addVisibilityToSymbolsFromOuterScope (
    ScopedVariableDeclarations * outerScopeDeclarations)
{
  using std::map;
  using std::string;

  for (std::map <string, SgVariableDeclaration *>::iterator it =
      outerScopeDeclarations->theDeclarations.begin (); it
      != outerScopeDeclarations->theDeclarations.end (); ++it)
  {
    add (it->first, it->second);
  }
}

ScopedVariableDeclarations::ScopedVariableDeclarations ()
{
}
