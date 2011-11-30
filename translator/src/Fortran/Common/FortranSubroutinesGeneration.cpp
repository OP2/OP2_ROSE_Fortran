#include <FortranSubroutinesGeneration.h>
#include <FortranParallelLoop.h>
#include <FortranHostSubroutine.h>
#include <FortranTypesBuilder.h>
#include <FortranProgramDeclarationsAndDefinitions.h>
#include <FortranReductionSubroutines.h>
#include <RoseHelper.h>
#include <boost/algorithm/string/predicate.hpp>

void
FortranSubroutinesGeneration::removeUseStatement (
    SgUseStatement * useStatement, std::string const & userSubroutineName)
{
  using namespace SageInterface;
  using boost::iequals;

  std::string const & fileName = declarations->getFileNameForSubroutine (
      userSubroutineName);

  std::string const & moduleNameToRemove = declarations->getModuleNameForFile (
      fileName);

  Debug::getInstance ()->debugMessage ("'" + userSubroutineName
      + "' is in Fortran module '" + moduleNameToRemove + "' (file '"
      + fileName + "')", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

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

SgVariableDeclaration *
FortranSubroutinesGeneration::addUserSubroutineNameDeclaration (
    SgScopeStatement * scope, std::string const & userSubroutineName)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Adding character array declaration with name '" + userSubroutineName
          + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgTypeString * characterArray = FortranTypesBuilder::getString (
      userSubroutineName.size ());

  SgAssignInitializer * initializer = buildAssignInitializer (buildStringVal (
      userSubroutineName), characterArray);

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      userSubroutineName + "_user", characterArray, initializer, scope);

  insertStatementAfterLastDeclaration (variableDeclaration, scope);

  return variableDeclaration;
}

void
FortranSubroutinesGeneration::addModuleUseStatement (SgNode * parent,
    std::string const & moduleName)
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Adding module use statement",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  if (isSgProgramHeaderStatement (parent))
  {
    Debug::getInstance ()->debugMessage ("Program header statement",
        Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

    SgProgramHeaderStatement * parentStatement = isSgProgramHeaderStatement (
        parent);

    SgScopeStatement * scope = parentStatement->get_definition ()->get_body ();

    if (find (headersWithAddedUseStatements.begin (),
        headersWithAddedUseStatements.end (),
        parentStatement->get_name ().getString ())
        == headersWithAddedUseStatements.end ())
    {
      SgUseStatement * newUseStatement = new SgUseStatement (
          getEnclosingFileNode (parentStatement)->get_file_info (), moduleName,
          false);

      prependStatement (newUseStatement, scope);

      headersWithAddedUseStatements.push_back (
          parentStatement->get_name ().getString ());
    }
  }
  else if (isSgProcedureHeaderStatement (parent))
  {
    Debug::getInstance ()->debugMessage ("Procedure header statement",
        Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

    SgProcedureHeaderStatement * parentStatement =
        isSgProcedureHeaderStatement (parent);

    SgScopeStatement * scope = parentStatement->get_definition ()->get_body ();

    if (find (headersWithAddedUseStatements.begin (),
        headersWithAddedUseStatements.end (),
        parentStatement->get_name ().getString ())
        == headersWithAddedUseStatements.end ())
    {
      SgUseStatement * newUseStatement = new SgUseStatement (
          getEnclosingFileNode (parentStatement)->get_file_info (), moduleName,
          false);

      prependStatement (newUseStatement, scope);

      headersWithAddedUseStatements.push_back (
          parentStatement->get_name ().getString ());
    }
  }
  else
  {
    throw Exceptions::CodeGeneration::UnfoundStatementException (
        "Could not find enclosing scope of OP_PAR_LOOP call");
  }
}

void
FortranSubroutinesGeneration::patchCallsToParallelLoops (
    std::string const & moduleName)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using std::map;
  using std::vector;
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

    for (vector <SgFunctionCallExp *>::const_iterator it =
        parallelLoop->getFirstFunctionCall (); it
        != parallelLoop->getLastFunctionCall (); ++it)
    {
      SgFunctionCallExp * functionCallExpression = *it;

      ROSE_ASSERT (isSgExprStatement(functionCallExpression->get_parent()));

      /*
       * ======================================================
       * Recursively go back in the scopes until we can find the
       * program header or subroutine header in which the
       * OP_PAR_LOOP call is contained
       * ======================================================
       */

      SgNode * parent = functionCallExpression->get_parent ();

      while (!isSgProcedureHeaderStatement (parent)
          && !isSgProgramHeaderStatement (parent))
      {
        parent = parent->get_parent ();
      }

      /*
       * ======================================================
       * Add module use statement to the scope just obtained
       * ======================================================
       */

      addModuleUseStatement (parent, moduleName);

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

      arguments.insert (arguments.begin (), buildStringVal (userSubroutineName));

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
}

void
FortranSubroutinesGeneration::addContains ()
{
  using namespace SageInterface;

  SgContainsStatement * containsStatement = new SgContainsStatement (
      RoseHelper::getFileInfo ());

  containsStatement->set_definingDeclaration (containsStatement);

  appendStatement (containsStatement, moduleScope);
}

SgModuleStatement *
FortranSubroutinesGeneration::createFortranModule (
    std::string const & moduleName)
{
  using namespace SageInterface;
  using std::string;
  using std::vector;

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

  determineWhichInputFilesToBeUnparsed ();
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
