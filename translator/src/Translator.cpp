#include <rose.h>

#include "CommandLine.h"
#include "Debug.h"
#include "CreateKernels.h"
#include "OP2DeclaredVariables.h"

int
main (int argc, char ** argv)
{
  /*
   * Process the command-line arguments to separate them into
   * ROSE arguments and arguments understood by our tool, such
   * as debug and verbose flags
   */
  CommandLine * commandLine = new CommandLine (argc, argv);

  Debug::getInstance ()->verboseMessage ("Translation starting");

  /*
   * Pass the pre-processed command-line arguments and NOT 'argc'
   * and 'argv', otherwise ROSE will complain
   */
  SgProject * project = frontend (commandLine->getNumberOfArguments (),
      commandLine->getArguments ());
  ROSE_ASSERT ( project != NULL );

  /*
   * We first need to get all OP2 declarations that will be used later to build the CUDA modules
   */
  Debug::getInstance ()->debugMessage ("Retrieving OP2 declarations", 2);
  OP2DeclaredVariables * op2DeclaredVariables = new OP2DeclaredVariables (
      project);
  op2DeclaredVariables->traverseInputFiles (project, preorder);

  /*
   * Create the parallel loop object with these files
   */
  Debug::getInstance ()->debugMessage ("Creating parallel loops", 2);
  CreateKernels * opParLoop = new CreateKernels (project, op2DeclaredVariables);

  /*
   * Traverse the ASTs of the input files in pre-order
   */
  opParLoop->traverseInputFiles (project, preorder);

  /*
   * Generate CUDA files
   */
  opParLoop->unparse ();

  /*
   * Unparse input source files
   */
  project->unparse ();

  Debug::getInstance ()->verboseMessage ("Translation completed");

  return 0;
}

