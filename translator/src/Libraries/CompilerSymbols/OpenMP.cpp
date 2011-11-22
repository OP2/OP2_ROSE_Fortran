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
          FortranTypesBuilder::buildNewFortranSubroutine (functionName, scope);

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
