#include <FortranSubroutinesGeneration.h>
#include <boost/algorithm/string/predicate.hpp>
#include <FortranTypesBuilder.h>
#include <RoseHelper.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

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

  Debug::getInstance ()->debugMessage ("Patching calls to OP_PAR_LOOPs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (map <string, FortranParallelLoop *>::iterator it =
      parallelLoops.begin (); it != parallelLoops.end (); ++it)
  {
    string const userSubroutineName = it->first;

    FortranParallelLoop * parallelLoop = it->second;

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
      Debug::getInstance ()->errorMessage (
          "Could not find last 'use' statement");
    }

    /*
     * ======================================================
     * Add a new 'use' statement for the newly created module
     * ======================================================
     */

    SgUseStatement * newUseStatement = new SgUseStatement (
        getEnclosingFileNode (lastUseStatement)->get_file_info (),
        "MODULE_NAME", false);

    insertStatementAfter (lastUseStatement, newUseStatement);

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
      FortranTypesBuilder::buildModuleDeclaration ("GENERATED_MODULE",
          globalScope);

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
        "Could not create dummy Fortran file '" + inputFileName + "'");
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

void
FortranSubroutinesGeneration::visit (SgNode * node)
{
  using boost::starts_with;
  using std::pair;
  using std::string;

  switch (node->variantT ())
  {

    case V_SgFunctionCallExp:
    {
      /*
       * ======================================================
       * Function call found in AST
       * ======================================================
       */
      SgFunctionCallExp * functionCallExp = isSgFunctionCallExp (node);

      string const
          calleeName =
              functionCallExp->getAssociatedFunctionDeclaration ()->get_name ().getString ();

      Debug::getInstance ()->debugMessage ("Found function call '" + calleeName
          + "'", Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

      /*
       * ======================================================
       * The prefix of all parallel loop calls in Fortran contains
       * 'OP_PAR_LOOP'. The suffix of the call, however, changes
       * depending on the number of expected parameters. Therefore,
       * any match of this prefix indicates a call of interest
       * to the translator
       * ======================================================
       */
      if (starts_with (calleeName, OP2::OP_PAR_LOOP))
      {
        /*
         * ======================================================
         * The first argument to an 'OP_PAR_LOOP' call should be
         * a reference to the kernel function. Cast it and proceed,
         * otherwise throw an exception
         * ======================================================
         */

        SgExpressionPtrList & actualArguments =
            functionCallExp->get_args ()->get_expressions ();

        SgFunctionRefExp * functionRefExpression = isSgFunctionRefExp (
            actualArguments.front ());

        ROSE_ASSERT (functionRefExpression != NULL);

        string const
            userSubroutineName =
                functionRefExpression->getAssociatedFunctionDeclaration ()->get_name ().getString ();

        Debug::getInstance ()->debugMessage ("Found '" + calleeName
            + "' with (host) user subroutine '" + userSubroutineName + "'",
            Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

        if (parallelLoops.find (userSubroutineName) == parallelLoops.end ())
        {
          /*
           * ======================================================
           * If this kernel has not been previously encountered then
           * build the target code and modify the calls in the original
           * Fortran source code
           * ======================================================
           */

          FortranParallelLoop * parallelLoop = new FortranParallelLoop (
              functionCallExp, userSubroutineName, declarations);

          parallelLoops[userSubroutineName] = parallelLoop;

          /*
           * ======================================================
           * Create the subroutines
           * ======================================================
           */

          FortranHostSubroutine * hostSubroutine = createSubroutines (
              parallelLoop, userSubroutineName);

          hostSubroutines[userSubroutineName] = hostSubroutine;
        }
        else
        {
          Debug::getInstance ()->debugMessage ("Parallel loop for '"
              + userSubroutineName + "' already created",
              Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);
        }
      }

      break;
    }

    case V_SgSourceFile:
    {
      SgSourceFile * sourceFile = isSgSourceFile (node);

      Debug::getInstance ()->debugMessage ("Found file "
          + sourceFile->getFileName (), Debug::OUTER_LOOP_LEVEL, __FILE__,
          __LINE__);

      break;
    }

    default:
    {
      break;
    }
  }
}

FortranSubroutinesGeneration::FortranSubroutinesGeneration (
    FortranProgramDeclarationsAndDefinitions * declarations,
    std::string const & fileSuffix) :
  SubroutinesGeneration <FortranProgramDeclarationsAndDefinitions,
      FortranParallelLoop, FortranHostSubroutine> (declarations, fileSuffix)
{
  SgSourceFile & sourceFile = createSourceFile ();

  SgModuleStatement * moduleStatement = createFortranModule (sourceFile);

  moduleScope = moduleStatement->get_definition ();
}
