#include <FortranOpenMPSubroutinesGeneration.h>
#include <FortranOpenMPModuleDeclarationsDirectLoop.h>
#include <FortranOpenMPModuleDeclarationsIndirectLoop.h>
#include <FortranOpenMPKernelSubroutineDirectLoop.h>
#include <FortranOpenMPKernelSubroutineIndirectLoop.h>
#include <FortranOpenMPHostSubroutineDirectLoop.h>
#include <FortranOpenMPHostSubroutineIndirectLoop.h>
#include <ROSEHelper.h>

FortranHostSubroutine *
FortranOpenMPSubroutinesGeneration::createSubroutines (
    FortranParallelLoop * parallelLoop, std::string const & userSubroutineName,
    SgModuleStatement * moduleStatement)
{
  SgScopeStatement * moduleScope = moduleStatement->get_definition ();

  FortranOpenMPKernelSubroutine * kernelSubroutine;

  FortranOpenMPHostSubroutine * hostSubroutine;

  if (parallelLoop->isDirectLoop ())
  {
    /*
     * ======================================================
     * Direct loop
     * ======================================================
     */

    Debug::getInstance ()->debugMessage (
        "Generating subroutines for direct loop", Debug::FUNCTION_LEVEL,
        __FILE__, __LINE__);

    FortranOpenMPModuleDeclarationsDirectLoop * moduleDeclarations =
        new FortranOpenMPModuleDeclarationsDirectLoop (userSubroutineName,
            parallelLoop, moduleScope);

    addContains (moduleStatement);

    kernelSubroutine = new FortranOpenMPKernelSubroutineDirectLoop (
        userSubroutineName, userSubroutineName, parallelLoop, moduleScope);

    hostSubroutine = new FortranOpenMPHostSubroutineDirectLoop (
        userSubroutineName, userSubroutineName,
        kernelSubroutine->getSubroutineName (), parallelLoop, moduleScope,
        moduleDeclarations);
  }
  else
  {
    /*
     * ======================================================
     * Indirect loop
     * ======================================================
     */

    Debug::getInstance ()->debugMessage (
        "Generating subroutines for indirect loop", Debug::FUNCTION_LEVEL,
        __FILE__, __LINE__);

    FortranOpenMPModuleDeclarationsIndirectLoop * moduleDeclarations =
        new FortranOpenMPModuleDeclarationsIndirectLoop (userSubroutineName,
            parallelLoop, moduleScope);

    addContains (moduleStatement);

    kernelSubroutine = new FortranOpenMPKernelSubroutineIndirectLoop (
        userSubroutineName, userSubroutineName, parallelLoop, moduleScope);

    hostSubroutine = new FortranOpenMPHostSubroutineIndirectLoop (
        userSubroutineName, userSubroutineName,
        kernelSubroutine->getSubroutineName (), parallelLoop, moduleScope,
        moduleDeclarations);
  }

  return hostSubroutine;
}

void
FortranOpenMPSubroutinesGeneration::addLibraries (
    SgModuleStatement * moduleStatement)
{
  using boost::iequals;
  using std::string;
  using std::vector;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  Debug::getInstance ()->debugMessage (
      "Adding 'use' statements to OpenMP module", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  vector <string> libs;
  libs.push_back (Libraries::OP2_C);
  libs.push_back (Libraries::OMP_LIB);

  for (vector <string>::const_iterator it = libs.begin (); it != libs.end (); ++it)
  {
    SgUseStatement* useStatement = new SgUseStatement (
        ROSEHelper::getFileInfo (), *it, false);

    useStatement->set_definingDeclaration (moduleStatement);

    appendStatement (useStatement, moduleStatement->get_definition ());

    if (iequals (*it, Libraries::OMP_LIB))
    {
      addTextForUnparser (useStatement, "#ifdef _OPENMP\n",
          AstUnparseAttribute::e_before);

      addTextForUnparser (useStatement, "#endif\n",
          AstUnparseAttribute::e_after);
    }
  }
}
