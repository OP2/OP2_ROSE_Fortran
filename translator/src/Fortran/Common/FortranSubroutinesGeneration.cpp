#include <boost/algorithm/string/predicate.hpp>
#include <Debug.h>
#include <Globals.h>
#include <FortranSubroutinesGeneration.h>
#include <FortranTypesBuilder.h>
#include <ROSEHelper.h>
#include <FortranOpDatDimensionsDeclaration.h>
#include <FortranCUDAKernelSubroutineDirectLoop.h>
#include <FortranCUDAKernelSubroutineIndirectLoop.h>
#include <FortranCUDAHostSubroutineDirectLoop.h>
#include <FortranCUDAHostSubroutineIndirectLoop.h>
#include <FortranCUDAInitialiseConstantsSubroutine.h>
#include <FortranCUDADataSizesDeclarationDirectLoop.h>
#include <FortranCUDADataSizesDeclarationIndirectLoop.h>
#include <FortranCUDAReductionSubroutine.h>
#include <FortranOpenMPModuleDeclarationsDirectLoop.h>
#include <FortranOpenMPModuleDeclarationsIndirectLoop.h>
#include <FortranOpenMPHostSubroutineDirectLoop.h>
#include <FortranOpenMPHostSubroutineIndirectLoop.h>
#include <FortranOpenMPKernelSubroutineDirectLoop.h>
#include <FortranOpenMPKernelSubroutineIndirectLoop.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranSubroutinesGeneration::patchCallsToParallelLoops (
    ParallelLoop & parallelLoop, std::string const & userSubroutineName,
    FortranSubroutine & hostSubroutine, SgScopeStatement * scope,
    SgFunctionCallExp * functionCallExp)
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

  Debug::getInstance ()->debugMessage ("Patching call to OP_PAR_LOOP", 2);

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
  SgStatement * lastDeclarationStatement = findLastDeclarationStatement (scope);
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
    Debug::getInstance ()->errorMessage ("Could not find last 'use' statement");
  }

  /*
   * ======================================================
   * Add a new 'use' statement for CUDA module
   * ======================================================
   */
  SgUseStatement * newUseStatement =
      new SgUseStatement (
          getEnclosingFileNode (lastUseStatement)->get_file_info (),
          parallelLoop.getModuleName (), false);

  insertStatementAfter (lastUseStatement, newUseStatement);

  /*
   * ======================================================
   * Build a string variable which contains the name of the
   * kernel. This variable is passed to the host code in
   * setting up and tearing down the relevant device code
   * ======================================================
   */

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
      hostSubroutine.getProcedureHeaderStatement ());

  functionCallExp->set_function (hostSubroutineReference);

  /*
   * ======================================================
   * Modify the first parameter from a kernel reference to
   * a kernel name
   * ======================================================
   */
  SgExpressionPtrList & arguments =
      functionCallExp->get_args ()->get_expressions ();

  arguments.erase (arguments.begin ());

  arguments.insert (arguments.begin (), buildVarRefExp (
      userSubroutineNameStringVariable));

  /*
   * ======================================================
   * Set where the function call is invoked as a transformation
   * in the unparser
   * ======================================================
   */
  SgLocatedNode * functionCallLocation = isSgLocatedNode (functionCallExp);

  functionCallLocation->get_file_info ()->setTransformation ();
}

void
FortranSubroutinesGeneration::createOpenMPSubroutines (
    ParallelLoop * parallelLoop, std::string const & userSubroutineName,
    SgModuleStatement * moduleStatement, SgNode * node,
    SgFunctionCallExp * functionCallExp)
{
  addOpenMPLibraries (moduleStatement);

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
        "Generating subroutines for direct loop", 5);

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
        "Generating subroutines for indirect loop", 5);

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

  /*
   * ======================================================
   * Get the scope of the AST node representing the entire
   * call statement
   * ======================================================
   */
  SgScopeStatement * scope =
      isSgExprStatement (node->get_parent ())->get_scope ();

  patchCallsToParallelLoops (*parallelLoop, userSubroutineName,
      *hostSubroutine, scope, functionCallExp);
}

void
FortranSubroutinesGeneration::createCUDASubroutines (
    ParallelLoop * parallelLoop, std::string const & userSubroutineName,
    SgModuleStatement * moduleStatement, SgNode * node,
    SgFunctionCallExp * functionCallExp)
{
  addCUDALibraries (moduleStatement);

  SgScopeStatement * moduleScope = moduleStatement->get_definition ();

  /*
   * ======================================================
   * Create the type representing the dimensions of each
   * OP_DAT
   * ======================================================
   */

  FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration =
      new FortranOpDatDimensionsDeclaration (userSubroutineName, parallelLoop,
          moduleScope);

  /*
   * ======================================================
   * Create the reduction subroutines
   * ======================================================
   */
  parallelLoop->generateReductionSubroutines (moduleScope);

  FortranCUDAUserSubroutine * userDeviceSubroutine;

  FortranCUDAKernelSubroutine * kernelSubroutine;

  FortranCUDAHostSubroutine * hostSubroutine;

  if (parallelLoop->isDirectLoop ())
  {
    /*
     * ======================================================
     * Direct loop
     * ======================================================
     */
    FortranCUDADataSizesDeclarationDirectLoop * dataSizesDeclaration =
        new FortranCUDADataSizesDeclarationDirectLoop (userSubroutineName,
            parallelLoop, moduleScope);

    FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine =
        new FortranCUDAInitialiseConstantsSubroutine (userSubroutineName,
            moduleScope);

    addContains (moduleStatement);

    initialiseConstantsSubroutine->generateSubroutine ();

    userDeviceSubroutine = new FortranCUDAUserSubroutine (userSubroutineName,
        initialiseConstantsSubroutine, declarations, parallelLoop, moduleScope);

    kernelSubroutine = new FortranCUDAKernelSubroutineDirectLoop (
        userSubroutineName, userDeviceSubroutine->getSubroutineName (),
        parallelLoop, moduleScope, dataSizesDeclaration);

    hostSubroutine = new FortranCUDAHostSubroutineDirectLoop (
        userSubroutineName, userDeviceSubroutine->getSubroutineName (),
        kernelSubroutine->getSubroutineName (), parallelLoop, moduleScope,
        initialiseConstantsSubroutine, dataSizesDeclaration,
        opDatDimensionsDeclaration);
  }
  else
  {
    /*
     * ======================================================
     * Indirect loop
     * ======================================================
     */

    FortranCUDADataSizesDeclarationIndirectLoop * dataSizesDeclaration =
        new FortranCUDADataSizesDeclarationIndirectLoop (userSubroutineName,
            parallelLoop, moduleScope);

    FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine =
        new FortranCUDAInitialiseConstantsSubroutine (userSubroutineName,
            moduleScope);

    addContains (moduleStatement);

    initialiseConstantsSubroutine->generateSubroutine ();

    userDeviceSubroutine = new FortranCUDAUserSubroutine (userSubroutineName,
        initialiseConstantsSubroutine, declarations, parallelLoop, moduleScope);

    kernelSubroutine = new FortranCUDAKernelSubroutineIndirectLoop (
        userSubroutineName, userDeviceSubroutine->getSubroutineName (),
        parallelLoop, moduleScope, dataSizesDeclaration);

    hostSubroutine = new FortranCUDAHostSubroutineIndirectLoop (
        userSubroutineName, userDeviceSubroutine->getSubroutineName (),
        kernelSubroutine->getSubroutineName (), parallelLoop, moduleScope,
        initialiseConstantsSubroutine, dataSizesDeclaration,
        opDatDimensionsDeclaration);
  }

  /*
   * ======================================================
   * Get the scope of the AST node representing the entire
   * call statement
   * ======================================================
   */
  SgScopeStatement * scope =
      isSgExprStatement (node->get_parent ())->get_scope ();

  patchCallsToParallelLoops (*parallelLoop,
      userDeviceSubroutine->getSubroutineName (), *hostSubroutine, scope,
      functionCallExp);
}

void
FortranSubroutinesGeneration::addOpenMPLibraries (
    SgModuleStatement * moduleStatement)
{
  using boost::iequals;
  using std::string;
  using std::vector;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  Debug::getInstance ()->debugMessage (
      "Adding 'use' statements to OpenMP module", 2);

  vector <string> libs;

  libs.push_back (IndirectAndDirectLoop::Fortran::Libraries::OP2_C);

  libs.push_back (IndirectAndDirectLoop::Fortran::Libraries::OMP_LIB);

  for (vector <string>::const_iterator it = libs.begin (); it != libs.end (); ++it)
  {
    SgUseStatement* useStatement = new SgUseStatement (
        ROSEHelper::getFileInfo (), *it, false);

    useStatement->set_definingDeclaration (moduleStatement);

    appendStatement (useStatement, moduleStatement->get_definition ());

    if (iequals (*it, IndirectAndDirectLoop::Fortran::Libraries::OMP_LIB))
    {
      addTextForUnparser (useStatement, "#ifdef _OPENMP\n",
          AstUnparseAttribute::e_before);

      addTextForUnparser (useStatement, "#endif\n",
          AstUnparseAttribute::e_after);
    }
  }
}

void
FortranSubroutinesGeneration::addCUDALibraries (
    SgModuleStatement * moduleStatement)
{
  using std::string;
  using std::vector;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Adding 'use' statements to CUDA module", 2);

  vector <string> libs;

  libs.push_back (IndirectAndDirectLoop::Fortran::Libraries::ISO_C_BINDING);

  libs.push_back (IndirectAndDirectLoop::Fortran::Libraries::OP2_C);

  libs.push_back (
      IndirectAndDirectLoop::Fortran::Libraries::cudaConfigurationParams);

  libs.push_back (IndirectAndDirectLoop::Fortran::Libraries::CUDAFOR);

  for (vector <string>::const_iterator it = libs.begin (); it != libs.end (); ++it)
  {
    SgUseStatement* useStatement = new SgUseStatement (
        ROSEHelper::getFileInfo (), *it, false);

    useStatement->set_definingDeclaration (moduleStatement);

    appendStatement (useStatement, moduleStatement->get_definition ());
  }
}

void
FortranSubroutinesGeneration::addContains (SgModuleStatement * moduleStatement)
{
  using SageInterface::appendStatement;

  SgContainsStatement * containsStatement = new SgContainsStatement (
      ROSEHelper::getFileInfo ());

  containsStatement->set_definingDeclaration (containsStatement);

  appendStatement (containsStatement, moduleStatement->get_definition ());
}

SgModuleStatement *
FortranSubroutinesGeneration::createFortranModule (SgSourceFile & sourceFile,
    ParallelLoop & parallelLoop)
{
  using std::string;
  using std::vector;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating Fortran module", 2);

  SgGlobal * globalScope = sourceFile.get_globalScope ();

  SgModuleStatement * moduleStatement =
      FortranTypesBuilder::buildNewFortranModuleDeclaration (
          parallelLoop.getModuleName (), globalScope);

  moduleStatement->get_definition ()->setCaseInsensitive (true);

  appendStatement (moduleStatement, globalScope);

  return moduleStatement;
}

SgSourceFile &
FortranSubroutinesGeneration::createSourceFile (ParallelLoop & parallelLoop)
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
  string const inputFileName = "BLANK_" + parallelLoop.getModuleName ()
      + ".F95";

  FILE * inputFile = fopen (inputFileName.c_str (), "w+");
  if (inputFile != NULL)
  {
    Debug::getInstance ()->debugMessage ("Creating dummy source file '"
        + inputFileName + "'", 2);

    fclose (inputFile);
  }
  else
  {
    Debug::getInstance ()->errorMessage (
        "Could not create dummy Fortran file '" + inputFileName + "'");
  }

  /*
   * ======================================================
   * Now generate the target backend file
   * ======================================================
   */
  string const outputFileName = parallelLoop.getModuleName () + ".F95";

  Debug::getInstance ()->debugMessage ("Generating file '" + outputFileName
      + "'", 2);

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
FortranSubroutinesGeneration::visit (SgNode * node)
{
  using boost::iequals;
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

        SgFunctionRefExp * functionRefExp = isSgFunctionRefExp (
            actualArguments.front ());
        try
        {
          if (functionRefExp != NULL)
          {
            string const
                userSubroutineName =
                    functionRefExp->getAssociatedFunctionDeclaration ()->get_name ().getString ();

            Debug::getInstance ()->debugMessage ("Found '" + calleeName
                + "' with (host) user subroutine '" + userSubroutineName + "'",
                1);

            if (parallelLoops.find (userSubroutineName) == parallelLoops.end ())
            {
              /*
               * ======================================================
               * If this kernel has not been previously encountered then
               * build the target code and modify the calls in the original
               * Fortran source code
               * ======================================================
               */
              ParallelLoop * parallelLoop = new ParallelLoop (
                  userSubroutineName, actualArguments, declarations);

              parallelLoops[userSubroutineName] = parallelLoop;

              /*
               * ======================================================
               * Generate an additional source file for this OP_PAR_LOOP
               * ======================================================
               */
              SgSourceFile & sourceFile = createSourceFile (*parallelLoop);

              /*
               * ======================================================
               * Create the Fortran module
               * ======================================================
               */
              SgModuleStatement * moduleStatement = createFortranModule (
                  sourceFile, *parallelLoop);

              if (iequals (Globals::getInstance ()->getTargetBackend (),
                  TargetBackends::CUDA))
              {
                createCUDASubroutines (parallelLoop, userSubroutineName,
                    moduleStatement, node, functionCallExp);
              }
              else if (iequals (Globals::getInstance ()->getTargetBackend (),
                  TargetBackends::OpenMP))
              {
                createOpenMPSubroutines (parallelLoop, userSubroutineName,
                    moduleStatement, node, functionCallExp);
              }
            }
          }
          else
          {
            throw actualArguments.front ();
          }
        }
        catch (SgNode * exceptionNode)
        {
          Debug::getInstance ()->errorMessage (
              "First argument to 'OP_PAR_LOOP' is not a function. The argument has type '"
                  + exceptionNode->class_name () + "'");
        }
      }

      break;
    }

    case V_SgSourceFile:
    {
      SgSourceFile * sourceFile = isSgSourceFile (node);

      Debug::getInstance ()->debugMessage ("Found file "
          + sourceFile->getFileName (), 8);
      break;
    }

    default:
    {
      break;
    }
  }
}

void
FortranSubroutinesGeneration::unparse ()
{
  using std::vector;

  Debug::getInstance ()->verboseMessage ("Generating files");

  for (vector <SgSourceFile *>::const_iterator it = generatedFiles.begin (); it
      != generatedFiles.end (); ++it)
  {
    Debug::getInstance ()->debugMessage ("Unparsing to '"
        + (*it)->getFileName () + "'", 1);

    /*
     * ======================================================
     * Unparse the created files after checking consistency
     * of ASTs
     * ======================================================
     */
    SgProject * project = (*it)->get_project ();

    AstTests::runAllTests (project);

    project->unparse ();
  }
}

FortranSubroutinesGeneration::FortranSubroutinesGeneration (
    SgProject * project, Declarations * declarations) :
  project (project), declarations (declarations)
{
}
