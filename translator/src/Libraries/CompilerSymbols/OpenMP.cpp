


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


#include "OpenMP.h"
#include "Globals.h"
#include "Exceptions.h"
#include "FortranTypesBuilder.h"
#include <rose.h>

std::string const
OpenMP::getIfDirectiveString ()
{
  return "\n#ifdef _OPENMP \n";
}

std::string const
OpenMP::getElseDirectiveString ()
{
  return "\n#else \n";
}

std::string const
OpenMP::getEndIfDirectiveString ()
{
  return "\n#endif \n";
}

std::string const
OpenMP::getParallelLoopDirectiveString ()
{
  switch (Globals::getInstance ()->getHostLanguage ())
  {
    case TargetLanguage::FORTRAN:
    {
      return "\n!$OMP PARALLEL DO ";
    }
    case TargetLanguage::CPP:
    {
      return "\n#pragma omp parallel for ";
    }
    default:
    {
      throw Exceptions::CommandLine::LanguageException ("Unknown host language");
    }
  }
}

std::string const
OpenMP::getEndParallelLoopDirectiveString ()
{
  switch (Globals::getInstance ()->getHostLanguage ())
  {
    case TargetLanguage::FORTRAN:
    {
      return "!$OMP END PARALLEL DO\n";
    }
    default:
    {
      throw Exceptions::CommandLine::LanguageException (
          "Unknown/unsupported host language");
    }
  }
}

SgFunctionCallExp *
OpenMP::createGetMaximumNumberOfThreadsCallStatement (SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using std::string;

  string const functionName = "omp_get_max_threads";

  switch (Globals::getInstance ()->getHostLanguage ())
  {
    case TargetLanguage::FORTRAN:
    {
      SgFunctionSymbol * functionSymbol =
          FortranTypesBuilder::buildNewFortranFunction (functionName, scope);

      return buildFunctionCallExp (functionSymbol, buildExprListExp ());
    }
    case TargetLanguage::CPP:
    {
      return buildFunctionCallExp (functionName, buildVoidType (),
          buildExprListExp (), scope);
    }
    default:
    {
      throw Exceptions::CommandLine::LanguageException ("Unknown host language");
    }
  }
}

std::string const
OpenMP::getPrivateClause (std::vector <SgVarRefExp *> privateVariableReferences)
{
  using namespace SageBuilder;
  using std::vector;
  using std::string;

  string privateClause = "private (";

  int i = 1;
  const int size = privateVariableReferences.size ();
  for (vector <SgVarRefExp *>::iterator it = privateVariableReferences.begin (); it
      != privateVariableReferences.end (); ++it)
  {
    privateClause.append ((*it)->unparseToString ());
    if (i < size)
    {
      privateClause.append (",");
    }
    i++;
  }

  privateClause.append (")\n");

  return privateClause;
}
