#include <CPPSubroutinesGeneration.h>
#include <FortranTypesBuilder.h>
#include <Cxx_Grammar.h>
/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
CPPSubroutinesGeneration::patchCallsToParallelLoops (
    /*std::string const & moduleName*/)
{
  using std::map;
  using std::string;
  using std::vector;

  Debug::getInstance()->debugMessage("Patching calls to OP_PAR_LOOPs", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  vector <string> processedFiles;

  for (map <string, CPPParallelLoop *>::const_iterator it = declarations->firstParallelLoop();
      it != declarations->lastParallelLoop();
      ++it)
    {
      string const userSubroutineName = it->first;

      CPPParallelLoop * parallelLoop = it->second;

      CPPHostSubroutine * hostSubroutine =
          hostSubroutines[userSubroutineName];

      SgFunctionCallExp * functionCallExpression =
          parallelLoop->getFunctionCall();

      SgScopeStatement * scope =
          isSgExprStatement( functionCallExpression->get_parent() )->get_scope();

      SgFunctionRefExp * hostSubroutineReference = buildFunctionRefExp(
          hostSubroutine->getSubroutineHeaderStatement() );

      functionCallExpression->set_function( /* XXX */ );

      /*
       * ==================================================
       * Remove the first parameter (kernel reference)
       * ==================================================
       */

      SgExpressionPtrList & arguments =
          functionCallExpression->get_args ()->get_expressions ();

      arguments.erase (arguments.begin ());





    }
}

SgSourceFile &
CPPSubroutinesGeneration::createSourceFile ()
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
  string const inputFileName = "BLANK.cpp";

  FILE * inputFile = fopen (inputFileName.c_str (), "w+");
  if (inputFile != NULL)
  {
    Debug::getInstance ()->debugMessage ("Creating dummy source file '" + inputFileName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

    fclose (inputFile);
  }
  else
  {
    Debug::getInstance ()->errorMessage ("Could not create dummy C++ file '" + inputFileName + "'");
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

  Debug::getInstance ()->debugMessage ("Generating file '" + outputFileName + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgSourceFile * sourceFile = isSgSourceFile (buildFile (inputFileName,
      outputFileName, NULL));

  /*
   * ======================================================
   * Later unparse C++ code
   * ======================================================
   */
  sourceFile->set_Cxx_only (true);

  /*
   * ======================================================
   * Store the file so it can be unparsed after AST
   * construction
   * ======================================================
   */
  generatedFiles.push_back (sourceFile);

  return *sourceFile;
}


CPPSubroutinesGeneration::CPPSubroutinesGeneration (
    CPPProgramDeclarationsAndDefinitions * declarations,
    std::string const & fileSuffix) :
  SubroutinesGeneration <CPPProgramDeclarationsAndDefinitions, CPPHostSubroutine> (declarations, fileSuffix)
{
  SgSourceFile & sourceFile = createSourceFile ();

  moduleScope = sourceFile.get_globalScope ();
}

void CPPSubroutinesGeneration::generate()
{
  SgSourceFile & sourceFile = createSourceFile();

  initialiseConstantsSubroutine = new FortranInitialiseConstantsSubroutine (
      moduleScope, declarations);

  initialiseConstantsSubroutine->declareConstants ();

  createSubroutines ();

  patchCallsToParallelLoops ();

  unparse ();
}


