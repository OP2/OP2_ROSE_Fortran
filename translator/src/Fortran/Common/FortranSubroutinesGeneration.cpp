#include <FortranSubroutinesGeneration.h>
#include <FortranParallelLoop.h>
#include <FortranHostSubroutine.h>
#include <FortranTypesBuilder.h>
#include <FortranProgramDeclarationsAndDefinitions.h>
#include <FortranReductionSubroutines.h>
#include <RoseHelper.h>
#include <boost/algorithm/string/predicate.hpp>

namespace
{
  std::string const & moduleName = "GENERATED_MODULE";
}

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

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
FortranSubroutinesGeneration::patchCallsToParallelLoops ()
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
  using std::vector;
  using std::find;

  Debug::getInstance ()->debugMessage ("Patching calls to OP_PAR_LOOPs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector <string> processedFiles;

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
      Debug::getInstance ()->errorMessage (
          "Could not find declaration statements", __FILE__, __LINE__);
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
      Debug::getInstance ()->errorMessage (
          "Could not find last 'use' statement", __FILE__, __LINE__);
    }

    if (find (processedFiles.begin (), processedFiles.end (),
        parallelLoop->getFileName ()) == processedFiles.end ())
    {
      Debug::getInstance ()->debugMessage ("Adding new use statement for '"
          + moduleName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

      processedFiles.push_back (parallelLoop->getFileName ());

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
FortranSubroutinesGeneration::createFortranModule (SgSourceFile & sourceFile)
{
  using std::string;
  using std::vector;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating Fortran module",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgGlobal * globalScope = sourceFile.get_globalScope ();

  SgModuleStatement * moduleStatement =
      FortranTypesBuilder::buildModuleDeclaration (moduleName, globalScope);

  moduleStatement->get_declarationModifier ().get_accessModifier ().setPublic ();

  moduleStatement->get_definition ()->setCaseInsensitive (true);

  appendStatement (moduleStatement, globalScope);

  return moduleStatement;
}

SgSourceFile &
FortranSubroutinesGeneration::createSourceFile ()
{
  using SageBuilder::buildFile;
  using std::string;

  /*
   * ======================================================
   * To create a new file (to which the AST is later unparsed),
   * the API expects the name of an existing file and the
   * name of the output file. There is no input file corresponding
   * to our output file, therefore we first create a dummy
   * Fortran file. This will cause the unparser to generate
   * a warning about its internal stack state, but it can
   * suitably be ignored
   * ======================================================
   */
  string const inputFileName = "BLANK.F95";

  FILE * inputFile = fopen (inputFileName.c_str (), "w+");
  if (inputFile != NULL)
  {
    Debug::getInstance ()->debugMessage ("Creating dummy source file '"
        + inputFileName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    fclose (inputFile);
  }
  else
  {
    Debug::getInstance ()->errorMessage (
        "Could not create dummy Fortran file '" + inputFileName + "'",
        __FILE__, __LINE__);
  }

  /*
   * ======================================================
   * Now generate the target backend file. The suffix of the
   * file changes according to the backend because the PGI
   * Fortran compiler requires the ".CUF" suffix to
   * correctly compile CUDA code
   * ======================================================
   */

  string outputFileName = "rose_" + fileSuffix;

  Debug::getInstance ()->debugMessage ("Generating file '" + outputFileName
      + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgSourceFile * sourceFile = isSgSourceFile (buildFile (inputFileName,
      outputFileName, NULL));

  /*
   * ======================================================
   * Later unparse according to the Fortran 95 standard
   * ======================================================
   */
  sourceFile->set_F95_only (true);

  /*
   * ======================================================
   * No implicit symbols shall be allowed in the generated
   * Fortran file
   * ======================================================
   */
  sourceFile->set_fortran_implicit_none (true);

  sourceFile->set_outputFormat (SgFile::e_free_form_output_format);

  /*
   * ======================================================
   * Store the file so it can be unparsed after AST
   * construction
   * ======================================================
   */
  generatedFiles.push_back (sourceFile);

  return *sourceFile;
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
FortranSubroutinesGeneration::generate ()
{
  SgSourceFile & sourceFile = createSourceFile ();

  SgModuleStatement * moduleStatement = createFortranModule (sourceFile);

  moduleScope = moduleStatement->get_definition ();

  addLibraries ();

  createModuleDeclarations ();

  addContains ();

  createReductionSubroutines ();

  createSubroutines ();

  patchCallsToParallelLoops ();

  unparse ();
}

FortranSubroutinesGeneration::FortranSubroutinesGeneration (
    FortranProgramDeclarationsAndDefinitions * declarations,
    std::string const & fileSuffix) :
  SubroutinesGeneration <FortranProgramDeclarationsAndDefinitions,
      FortranHostSubroutine> (declarations, fileSuffix)
{
  reductionSubroutines = new FortranReductionSubroutines ();
}
