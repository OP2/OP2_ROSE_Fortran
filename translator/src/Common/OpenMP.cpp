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

SgFunctionCallExp *
OpenMP::createGetMaximumNumberOfThreadsCallStatement (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildFunctionCallExp ("omp_get_max_threads", buildVoidType (),
      buildExprListExp (), scope);
}
