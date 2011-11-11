#include <OpenMP.h>
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
OpenMP::getParallelForDirectiveString ()
{
  return "\n#pragma omp parallel for ";
}

SgFunctionCallExp *
OpenMP::createGetMaximumNumberOfThreadsCallStatement (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildFunctionCallExp ("omp_get_max_threads", buildVoidType (),
      buildExprListExp (), scope);
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
