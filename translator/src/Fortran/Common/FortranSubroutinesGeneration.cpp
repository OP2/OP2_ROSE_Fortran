#include <FortranSubroutinesGeneration.h>
#include <FortranParallelLoop.h>
#include <FortranHostSubroutine.h>
#include <FortranTypesBuilder.h>
#include <FortranProgramDeclarationsAndDefinitions.h>
#include <FortranReductionSubroutines.h>
#include <RoseHelper.h>
#include <boost/algorithm/string/predicate.hpp>

void
FortranSubroutinesGeneration::fixUseStatement (SgUseStatement * useStatement,
    std::string const & userSubroutineName)
{
  using boost::iequals;
  using SageInterface::getPreviousStatement;
  using SageInterface::removeStatement;

  std::string const & moduleNameToRemove = declarations->getModuleNameForFile (
      declarations->getFileNameForSubroutine (userSubroutineName));

  Debug::getInstance ()->debugMessage ("'" + userSubroutineName
      + "' is in Fortran module '" + moduleNameToRemove + "'",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgUseStatement * lastUseStatement = useStatement;

  ROSE_ASSERT (lastUseStatement != NULL);

  do
  {
    if (iequals (lastUseStatement->get_name ().getString (), moduleNameToRemove))
    {
      removeStatement (lastUseStatement);
      break;
    }

    lastUseStatement = isSgUseStatement (
        getPreviousStatement (lastUseStatement));
  }
  while (lastUseStatement != NULL);
}

void
FortranSubroutinesGeneration::patchCallsToParallelLoops (
    std::string const & moduleName)
{
  using SageBuilder::buildAssignInitializer;
  using SageBuilder::buildStringVal;
  using SageBuilder::buildFunctionRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::getScope;
  using SageInterface::getEnclosingFileNode;
  using SageInterface::insertStatementAfter;
  using SageInterface::getPreviousStatement;
  using SageInterface::findLastDeclarationStatement;
  using std::map;
  using std::string;
  using std::find;

  Debug::getInstance ()->debugMessage ("Patching calls to OP_PAR_LOOPs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (map <string, ParallelLoop *>::const_iterator it =
      declarations->firstParallelLoop (); it
      != declarations->lastParallelLoop (); ++it)
  {
    string const userSubroutineName = it->first;

    FortranParallelLoop * parallelLoop =
        static_cast <FortranParallelLoop *> (it->second);

    Debug::getInstance ()->debugMessage ("Parallel loop for "
        + userSubroutineName, Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    FortranHostSubroutine * hostSubroutine =
        hostSubroutines[userSubroutineName];

    SgFunctionCallExp * functionCallExpression =
        parallelLoop->getFunctionCall ();

    SgScopeStatement * scope = isSgExprStatement (
        functionCallExpression->get_parent ())->get_scope ();

    /*
     * ======================================================
     * We first need to add a 'use <NEWLY_CREATED_MODULE>'
     * statement to the Fortran module where the OP_PAR_LOOP
     * call takes place
     * ======================================================
     */

    /*
     * ======================================================
     * Recursively go back in the scopes until we can find a
     * declaration statement
     * ======================================================
     */
    SgStatement * lastDeclarationStatement = findLastDeclarationStatement (
        scope);

    SgScopeStatement * parent = scope;

    while (lastDeclarationStatement == NULL)
    {
      parent = (SgScopeStatement *) parent->get_parent ();
      lastDeclarationStatement = findLastDeclarationStatement (parent);
    }

    if (lastDeclarationStatement == NULL)
    {
      throw Exceptions::CodeGeneration::UnfoundStatementException (
          "Could not find declaration statements");
    }

    /*
     * ======================================================
     * Now find the last 'use' statement
     * ======================================================
     */

    SgStatement * previousStatement = lastDeclarationStatement;
    SgUseStatement * lastUseStatement;
    do
    {
      previousStatement = getPreviousStatement (previousStatement);
      lastUseStatement = isSgUseStatement (previousStatement);
    }
    while (lastUseStatement == NULL);

    if (lastUseStatement == NULL)
    {
      throw Exceptions::CodeGeneration::UnfoundStatementException (
          "Could not find last 'use' statement");
    }

    if (find (dirtyFiles.begin (), dirtyFiles.end (),
        parallelLoop->getFileName ()) == dirtyFiles.end ())
    {
      Debug::getInstance ()->debugMessage ("Adding new use statement for '"
          + moduleName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

      dirtyFiles.push_back (parallelLoop->getFileName ());

      /*
       * ======================================================
       * Add a new 'use' statement for the newly created module
       * only if we have not previously added it to this file
       * ======================================================
       */

      SgUseStatement * newUseStatement = new SgUseStatement (
          getEnclosingFileNode (lastUseStatement)->get_file_info (),
          moduleName, false);

      insertStatementAfter (lastUseStatement, newUseStatement);
    }

    fixUseStatement (lastUseStatement, userSubroutineName);

    /*
     * ======================================================
     * The character array contains exactly the number of
     * characters as the user subroutine name
     * ======================================================
     */

    SgTypeString * characterArray = FortranTypesBuilder::getString (
        userSubroutineName.size ());

    SgAssignInitializer * initializer = buildAssignInitializer (buildStringVal (
        userSubroutineName), characterArray);

    SgVariableDeclaration * userSubroutineNameStringVariable =
        buildVariableDeclaration (userSubroutineName + "_user", characterArray,
            initializer, getScope (lastDeclarationStatement));

    insertStatementAfter (lastDeclarationStatement,
        userSubroutineNameStringVariable);

    /*
     * ======================================================
     * Modify the call to OP_PAR_LOOP with a call to the newly
     * built host subroutine
     * ======================================================
     */

    SgFunctionRefExp * hostSubroutineReference = buildFunctionRefExp (
        hostSubroutine->getSubroutineHeaderStatement ());

    functionCallExpression->set_function (hostSubroutineReference);

    /*
     * ======================================================
     * Modify the first parameter from a kernel reference to
     * a kernel name
     * ======================================================
     */

    SgExpressionPtrList & arguments =
        functionCallExpression->get_args ()->get_expressions ();

    arguments.erase (arguments.begin ());

    arguments.insert (arguments.begin (), buildVarRefExp (
        userSubroutineNameStringVariable));

    /*
     * ======================================================
     * Set where the function call is invoked as a transformation
     * in the unparser
     * ======================================================
     */

    SgLocatedNode * functionCallLocation = isSgLocatedNode (
        functionCallExpression);

    ROSE_ASSERT (functionCallLocation != NULL);

    functionCallLocation->get_file_info ()->setTransformation ();
  }
}

void
FortranSubroutinesGeneration::addContains ()
{
  using SageInterface::appendStatement;

  SgContainsStatement * containsStatement = new SgContainsStatement (
      RoseHelper::getFileInfo ());

  containsStatement->set_definingDeclaration (containsStatement);

  appendStatement (containsStatement, moduleScope);
}

SgModuleStatement *
FortranSubroutinesGeneration::createFortranModule (
    std::string const & moduleName)
{
  using std::string;
  using std::vector;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating Fortran module",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgGlobal * globalScope = sourceFile->get_globalScope ();

  SgModuleStatement * moduleStatement =
      FortranTypesBuilder::buildModuleDeclaration (moduleName, globalScope);

  moduleStatement->get_declarationModifier ().get_accessModifier ().setPublic ();

  moduleStatement->get_definition ()->setCaseInsensitive (true);

  appendStatement (moduleStatement, globalScope);

  return moduleStatement;
}

void
FortranSubroutinesGeneration::generate ()
{
  using std::string;

  string const & moduleName = "GENERATED_MODULE";

  SgModuleStatement * moduleStatement = createFortranModule (moduleName);

  moduleScope = moduleStatement->get_definition ();

  addLibraries ();

  createModuleDeclarations ();

  addContains ();

  createReductionSubroutines ();

  createSubroutines ();

  patchCallsToParallelLoops (moduleName);
}

FortranSubroutinesGeneration::FortranSubroutinesGeneration (
    SgProject * project,
    FortranProgramDeclarationsAndDefinitions * declarations,
    std::string const & newFileName) :
  SubroutinesGeneration <FortranProgramDeclarationsAndDefinitions,
      FortranHostSubroutine> (project, declarations, newFileName)
{
  reductionSubroutines = new FortranReductionSubroutines ();
}
