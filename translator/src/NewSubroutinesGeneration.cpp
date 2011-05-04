#include <boost/algorithm/string/predicate.hpp>
#include <Debug.h>
#include <NewSubroutinesGeneration.h>
#include <FortranTypesBuilder.h>
#include <ROSEHelper.h>
#include <UserDeviceSubroutine.h>
#include <KernelSubroutineOfDirectLoop.h>
#include <KernelSubroutineOfIndirectLoop.h>
#include <HostSubroutineOfDirectLoop.h>
#include <HostSubroutineOfIndirectLoop.h>
#include <InitialiseConstantsSubroutine.h>
#include <DeviceDataSizesDeclaration.h>
#include <DeviceDataSizesDeclarationDirectLoops.h>
#include <ReductionSubroutine.h>


/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
NewSubroutinesGeneration::patchOP_PAR_LOOPCalls (ParallelLoop & parallelLoop,
    UserDeviceSubroutine & userDeviceSubroutine,
    HostSubroutine & hostSubroutine, SgScopeStatement * scope,
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
   * Add a new 'use' statement
   * ======================================================
   */
  SgUseStatement* newUseStatement =
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
   * characters as the kernel name
   * ======================================================
   */
  SgTypeString * characterArray = FortranTypesBuilder::getString (
      userDeviceSubroutine.getSubroutineName ().size ());

  SgAssignInitializer * initializer = buildAssignInitializer (buildStringVal (
      userDeviceSubroutine.getSubroutineName ()), characterArray);

  SgVariableDeclaration * kernelStringVariable = buildVariableDeclaration (
      userDeviceSubroutine.getSubroutineName () + "_name", characterArray,
      initializer, getScope (lastDeclarationStatement));

  insertStatementAfter (lastDeclarationStatement, kernelStringVariable);

  /*
   * ======================================================
   * Modify the call to OP_PAR_LOOP with a call to the newly
   * built host subroutine
   * ======================================================
   */
  SgFunctionRefExp * hostSubroutineReference = buildFunctionRefExp (
      hostSubroutine.getSubroutineHeaderStatement ());

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

  arguments.insert (arguments.begin (), buildVarRefExp (kernelStringVariable));

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
NewSubroutinesGeneration::addLibraries (SgModuleStatement * moduleStatement)
{
  using std::string;
  using std::vector;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Adding 'use' statements to CUDA module", 2);

  vector <string> libs;
  libs.push_back ("ISO_C_BINDING");
  libs.push_back ("OP2_C");
  libs.push_back ("cudafor");

  for (vector <string>::const_iterator it = libs.begin (); it != libs.end (); ++it)
  {
    SgUseStatement* useStatement = new SgUseStatement (
        ROSEHelper::getFileInfo (), *it, false);

    useStatement->set_definingDeclaration (moduleStatement);

    appendStatement (useStatement, moduleStatement->get_definition ());
  }
}

void
NewSubroutinesGeneration::addContains (SgModuleStatement * moduleStatement)
{
  using SageInterface::appendStatement;

  SgContainsStatement * containsStatement = new SgContainsStatement (
      ROSEHelper::getFileInfo ());

  containsStatement->set_definingDeclaration (containsStatement);

  appendStatement (containsStatement, moduleStatement->get_definition ());
}

SgModuleStatement *
NewSubroutinesGeneration::createCUDAModule (SgSourceFile & sourceFile,
    ParallelLoop & parallelLoop)
{
  using std::string;
  using std::vector;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating CUDA module", 2);

  SgGlobal * globalScope = sourceFile.get_globalScope ();

  SgModuleStatement * moduleStatement =
      FortranTypesBuilder::buildNewFortranModuleDeclaration (
          parallelLoop.getModuleName (), globalScope);

  moduleStatement->get_definition ()->setCaseInsensitive (true);

  appendStatement ( moduleStatement, globalScope );	

  return moduleStatement;
}

SgSourceFile &
NewSubroutinesGeneration::createSourceFile (ParallelLoop & parallelLoop)
{
  using SageBuilder::buildFile;
  using std::string;

  /*
   * ======================================================
   * To create a new file (to which the AST is later unparsed),
   * the API expects the name of an existing file and the
   * name of the output file. There is no input file corresponding
   * to our output file, therefore we first create a dummy
   * Fortran file
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
   * Now generate the CUDA file
   * ======================================================
   */
  string const outputFileName = parallelLoop.getModuleName () + ".CUF";

  Debug::getInstance ()->debugMessage ("Generating CUDA file '"
      + outputFileName + "'", 2);

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
NewSubroutinesGeneration::visit (SgNode * node)
{
  using boost::starts_with;
  using std::make_pair;
  using std::pair;
  using std::string;

	// to be removed
	using SageBuilder::buildIntVal;
	using SageBuilder::buildArrayType;
	
	
  switch (node->variantT ())
  {
    case V_SgFunctionCallExp:
    {
      /*
       * ======================================================
       * Function call found in AST
       * ======================================================
       */
      SgFunctionCallExp* functionCallExp = isSgFunctionCallExp (node);

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
      if (starts_with (calleeName, OP2::OP_PAR_LOOP_PREFIX))
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
               * If this kernel has not been previously encountered then build
               * the CUDA-Fortran file and modify the calls in the original
               * Fortran source code
               * ======================================================
               */
							

              ParallelLoop * parallelLoop = new ParallelLoop (
                  userSubroutineName, actualArguments, declarations);

              parallelLoops.insert (
                  make_pair (userSubroutineName, parallelLoop));

              /*
               * ======================================================
               * Generate an additional source file for this OP_PAR_LOOP
               * ======================================================
               */
              SgSourceFile & sourceFile = createSourceFile (*parallelLoop);

              /*
               * ======================================================
               * Create the CUDA module
               * ======================================================
               */
              SgModuleStatement * moduleStatement = createCUDAModule (
                  sourceFile, *parallelLoop);

              addLibraries (moduleStatement);

              SgScopeStatement * moduleScope =
                  moduleStatement->get_definition ();

              if (parallelLoop->isDirectLoop ())
              {
								
								Debug::getInstance ()->debugMessage ( "Direct Loop", 2 );
								
								/*
                 * ======================================================
                 * Direct loop
                 * ======================================================
                 */
                DeviceDataSizesDeclarationDirectLoops * deviceDataSizesDeclarationDirectLoops =
								  new DeviceDataSizesDeclarationDirectLoops ( *parallelLoop,
									  userSubroutineName, moduleScope );
								
								deviceDataSizesDeclarationDirectLoops->initialise ( *parallelLoop, moduleScope );
								
                /*
                 * ======================================================
                 * Indirect loops use global constants. Declare them and
                 * generate the subroutine which initialises them
                 * ======================================================
                 */
								
                InitialiseConstantsSubroutine * initialiseConstantsSubroutine =
								new InitialiseConstantsSubroutine (userSubroutineName);
								
                initialiseConstantsSubroutine->declareConstants (moduleScope);
								
								

                addContains (moduleStatement);

                /*
                 * ======================================================
                 * Generate and modify user kernel so that it can run on
                 * the device
                 * ======================================================
                 */


                UserDeviceSubroutine * userDeviceSubroutine =
                    new UserDeviceSubroutine (userSubroutineName, moduleScope,
                        *declarations);

                KernelSubroutine * kernelSubroutine =
                    new KernelSubroutineOfDirectLoop (userSubroutineName,
                        *userDeviceSubroutine, *deviceDataSizesDeclarationDirectLoops,
												*parallelLoop, moduleScope);

                HostSubroutine * hostSubroutine =
                    new HostSubroutineOfDirectLoop (userSubroutineName,
                        *userDeviceSubroutine, *kernelSubroutine,
												*deviceDataSizesDeclarationDirectLoops,
                        *parallelLoop, moduleScope);

                /*
                 * ======================================================
                 * Get the scope of the AST node representing the entire
                 * call statement
                 * ======================================================
                 */
                SgScopeStatement * scope = isSgExprStatement (
                    node->get_parent ())->get_scope ();

                ROSE_ASSERT (scope != NULL);

                patchOP_PAR_LOOPCalls (*parallelLoop, *userDeviceSubroutine,
                    *hostSubroutine, scope, functionCallExp);
              }
              else
              {
                /*
                 * ======================================================
                 * Indirect loop
                 * ======================================================
                 */

								Debug::getInstance ()->debugMessage ( "Indirect Loop", 2 );

                DeviceDataSizesDeclaration * deviceDataSizesDeclaration =
                    new DeviceDataSizesDeclaration (*parallelLoop,
                        userSubroutineName, moduleScope);

								deviceDataSizesDeclaration->initialise ( *parallelLoop, moduleScope );
                /*
                 * ======================================================
                 * Indirect loops use global constants. Declare them and
                 * generate the subroutine which initialises them
                 * ======================================================
                 */

                InitialiseConstantsSubroutine * initialiseConstantsSubroutine =
                    new InitialiseConstantsSubroutine (userSubroutineName);

                initialiseConstantsSubroutine->declareConstants (moduleScope);

                addContains (moduleStatement);

                initialiseConstantsSubroutine->generateSubroutine (moduleScope);

                /*
                 * ======================================================
                 * Generate and modify user kernel so that it can run on
                 * the device
                 * ======================================================
                 */

                UserDeviceSubroutine * userDeviceSubroutine =
                    new UserDeviceSubroutine (userSubroutineName, moduleScope,
                        *declarations);

                KernelSubroutine * kernelSubroutine =
                    new KernelSubroutineOfIndirectLoop (userSubroutineName,
                        *userDeviceSubroutine, *deviceDataSizesDeclaration,
                        *parallelLoop, moduleScope);

                HostSubroutine
                    * hostSubroutine =
                        new HostSubroutineOfIndirectLoop (userSubroutineName,
                            *userDeviceSubroutine, *kernelSubroutine,
                            *initialiseConstantsSubroutine,
                            *deviceDataSizesDeclaration, *parallelLoop,
                            moduleScope);

                /*
                 * ======================================================
                 * Get the scope of the AST node representing the entire
                 * call statement
                 * ======================================================
                 */
                SgScopeStatement * scope = isSgExprStatement (
                    node->get_parent ())->get_scope ();

                ROSE_ASSERT (scope != NULL);

                patchOP_PAR_LOOPCalls (*parallelLoop, *userDeviceSubroutine,
                    *hostSubroutine, scope, functionCallExp);
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
NewSubroutinesGeneration::unparse ()
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
