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
  ROSE_ASSERT (project != NULL);

  /*
   * We first need to get all OP2 declarations
   */
  Debug::getInstance ()->debugMessage ("Retrieving OP2 declarations", 2);
  Declarations * op2DeclaredVariables = new Declarations (
      project);
  op2DeclaredVariables->traverseInputFiles (project, preorder);

  /*
   * Create the parallel loop object with these files
   */
  Debug::getInstance ()->debugMessage ("Creating kernels for OP_PAR_LOOPs", 2);
  CreateKernels * createKernels = new CreateKernels (project,
      op2DeclaredVariables);

  /*
   * Traverse the ASTs of the input files in pre-order
   */
  createKernels->traverseInputFiles (project, preorder);

  /*
   * Generate CUDA files
   */
  createKernels->unparse ();

  /*
   * Unparse input source files as function calls to OP_PAR_LOOPs
   * will now have changed
   */
  project->unparse ();

  Debug::getInstance ()->verboseMessage ("Translation completed");

  return 0;
}

