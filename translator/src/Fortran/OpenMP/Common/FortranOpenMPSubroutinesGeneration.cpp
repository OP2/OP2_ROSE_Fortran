#include <FortranOpenMPSubroutinesGeneration.h>
#include <FortranOpenMPModuleDeclarationsDirectLoop.h>
#include <FortranOpenMPModuleDeclarationsIndirectLoop.h>
#include <FortranOpenMPKernelSubroutineDirectLoop.h>
#include <FortranOpenMPKernelSubroutineIndirectLoop.h>
#include <FortranOpenMPHostSubroutineDirectLoop.h>
#include <FortranOpenMPHostSubroutineIndirectLoop.h>
#include <RoseHelper.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranOpenMPSubroutinesGeneration::createSubroutines ()
{
}

void
FortranOpenMPSubroutinesGeneration::createModuleDeclarations ()
{
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
