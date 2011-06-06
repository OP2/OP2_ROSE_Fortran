#include <boost/lexical_cast.hpp>
#include <FortranKernelSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

std::string
FortranKernelSubroutine::get_OP_DAT_FormalParameterName (
    unsigned int OP_DAT_ArgumentGroup)
{
  using boost::lexical_cast;
  using std::string;

  return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
      + lexical_cast <string> (OP_DAT_ArgumentGroup);
}

FortranKernelSubroutine::FortranKernelSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    ParallelLoop * parallelLoop) :
  Subroutine (subroutineName + SubroutineNameSuffixes::kernelSuffix)
{
  this->userSubroutineName = userSubroutineName;

  this->parallelLoop = parallelLoop;
}
