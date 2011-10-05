#include <FortranOpenMPSubroutinesGeneration.h>
#include <FortranOpenMPModuleDeclarationsDirectLoop.h>
#include <FortranOpenMPModuleDeclarationsIndirectLoop.h>
#include <FortranOpenMPKernelSubroutineDirectLoop.h>
#include <FortranOpenMPKernelSubroutineIndirectLoop.h>
#include <FortranOpenMPHostSubroutineDirectLoop.h>
#include <FortranOpenMPHostSubroutineIndirectLoop.h>
#include <FortranProgramDeclarationsAndDefinitions.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <RoseHelper.h>
#include <boost/algorithm/string.hpp>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranOpenMPSubroutinesGeneration::createReductionSubroutines ()
{
}

void
FortranOpenMPSubroutinesGeneration::createSubroutines ()
{
  using std::string;
  using std::map;

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    FortranParallelLoop * parallelLoop =
        static_cast <FortranParallelLoop *> (it->second);

    FortranOpenMPKernelSubroutine * kernelSubroutine;

    if (parallelLoop->isDirectLoop ())
    {
      kernelSubroutine = new FortranOpenMPKernelSubroutineDirectLoop (
          userSubroutineName, userSubroutineName, parallelLoop, moduleScope);

      hostSubroutines[userSubroutineName]
          = new FortranOpenMPHostSubroutineDirectLoop (
              userSubroutineName,
              userSubroutineName,
              kernelSubroutine->getSubroutineName (),
              parallelLoop,
              moduleScope,
              static_cast <FortranOpenMPModuleDeclarationsDirectLoop *> (moduleDeclarations[userSubroutineName]));
    }
    else
    {
      kernelSubroutine = new FortranOpenMPKernelSubroutineIndirectLoop (
          userSubroutineName, userSubroutineName, parallelLoop, moduleScope);

      hostSubroutines[userSubroutineName]
          = new FortranOpenMPHostSubroutineIndirectLoop (
              userSubroutineName,
              userSubroutineName,
              kernelSubroutine->getSubroutineName (),
              parallelLoop,
              moduleScope,
              static_cast <FortranOpenMPModuleDeclarationsIndirectLoop *> (moduleDeclarations[userSubroutineName]));
    }
  }
}

void
FortranOpenMPSubroutinesGeneration::createModuleDeclarations ()
{
  using std::map;
  using std::string;

  /*
   * ======================================================
   * First declare the type declarations
   * ======================================================
   */

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    FortranParallelLoop * parallelLoop =
        static_cast <FortranParallelLoop *> (it->second);

    dimensionsDeclarations[userSubroutineName]
        = new FortranOpDatDimensionsDeclaration (parallelLoop, moduleScope);
  }

  /*
   * ======================================================
   * Now declare the variables
   * ======================================================
   */

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    FortranParallelLoop * parallelLoop =
        static_cast <FortranParallelLoop *> (it->second);

    if (parallelLoop->isDirectLoop ())
    {
      moduleDeclarations[userSubroutineName]
          = new FortranOpenMPModuleDeclarationsDirectLoop (parallelLoop,
              moduleScope);
    }
    else
    {
      moduleDeclarations[userSubroutineName]
          = new FortranOpenMPModuleDeclarationsIndirectLoop (parallelLoop,
              moduleScope);
    }
  }
}

void
FortranOpenMPSubroutinesGeneration::addLibraries ()
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
        RoseHelper::getFileInfo (), *it, false);

    useStatement->set_definingDeclaration (useStatement);

    appendStatement (useStatement, moduleScope);

    if (iequals (*it, Libraries::OMP_LIB))
    {
      addTextForUnparser (useStatement, "#ifdef _OPENMP\n",
          AstUnparseAttribute::e_before);

      addTextForUnparser (useStatement, "#endif\n",
          AstUnparseAttribute::e_after);
    }
  }
}
