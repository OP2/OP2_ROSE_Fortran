#include <rose.h>

#include "CommandLine.h"
#include "Debug.h"
#include "OpParLoop.h"

int
main (int argc, char **argv)
{
  /*
   * Process the command-line arguments to separate them into
   * ROSE arguments and arguments understood by our tool, such
   * as debug and verbose flags
   */
  CommandLine* commandLine = new CommandLine (argc, argv);

  Debug::getInstance ()->verboseMessage ("Translation starting");

  /*
   * Pass the pre-processed command-line arguments and NOT 'argc'
   * and 'argv', otherwise ROSE will complain
   */
  SgProject *project = frontend (commandLine->getNumberOfArguments (),
                                 commandLine->getArguments ());
  ROSE_ASSERT(project != NULL);

  /*
   * Create the parallel loop object with these files
   */
  OpParLoop *opParLoop = new OpParLoop (project);

  /*
   * Traverse the ASTs of the input files in pre-order
   */
  opParLoop->traverseInputFiles (project, preorder);

  /*
   * Generate CUDA files
   */
  opParLoop->unparse ();

  Debug::getInstance ()->verboseMessage ("Translation completed");

  return 0;
}

