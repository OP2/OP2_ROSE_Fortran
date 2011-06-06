/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Contains the entry point (i.e. 'main') of the translator.
 * It does the following:
 *
 * 1) Divides the command-line arguments into two sets: those
 * recognised by our translator and those recognised by ROSE.
 *
 * 2) Obtains the declarations of OP2 variables (i.e. sets, maps,
 * data, etc.) and subroutines found in the source files to be
 * processed.
 *
 * 3) Creates the subroutines which implement an OP_PAR_LOOP.
 *
 * 4) Unparses the generated subroutines to files; there is one
 * file per unique OP_PAR_LOOP call.
 *
 * 5) Unparses the original source files to reflect the changes
 * made to OP_PAR_LOOP calls.
 */

#include <boost/algorithm/string.hpp>
#include <rose.h>
#include <CommandLine.h>
#include <CommonNamespaces.h>
#include <Debug.h>
#include <Declarations.h>
#include <Globals.h>
#include <ModifyOP2Calls.h>
#include <FortranSubroutinesGeneration.h>

int
main (int argc, char ** argv)
{
  using boost::iequals;

  /*
   * ======================================================
   * Process the command-line arguments to separate them into
   * ROSE arguments and arguments understood by our tool, such
   * as debug and verbose flags
   * ======================================================
   */
  CommandLine * commandLine = new CommandLine (argc, argv);

  if (iequals (Globals::getInstance ()->getTargetBackend (),
      TargetBackends::Unknown))
  {
    Debug::getInstance ()->errorMessage (
        "You must specify a target backend on the command-line. Supported backends are: {"
            + TargetBackends::CUDA + ", " + TargetBackends::OpenMP + "}");
  }

  Debug::getInstance ()->verboseMessage ("Translation starting");

  /*
   * ======================================================
   * Pass the pre-processed command-line arguments and NOT
   * 'argc' and 'argv', otherwise ROSE will complain
   * ======================================================
   */
  SgProject * project = frontend (commandLine->getNumberOfArguments (),
      commandLine->getArguments ());

  ROSE_ASSERT (project != NULL);

  if (project->get_Cxx_only () == true)
  {
    Debug::getInstance ()->verboseMessage ("C++ project detected");

    ModifyOP2Calls * modifyOP2Calls = new ModifyOP2Calls (project);

    modifyOP2Calls->traverseInputFiles (project, preorder);
  }
  else
  {
    Debug::getInstance ()->verboseMessage ("Fortran project detected");

    /*
     * ======================================================
     * Obtain all OP2 declarations
     * ======================================================
     */
    Debug::getInstance ()->verboseMessage (
        "Retrieving declarations in source files");

    Declarations * declarations = new Declarations (project);

    declarations->traverseInputFiles (project, preorder);

    /*
     * ======================================================
     * Create the subroutines implementing an OP_PAR_LOOP
     * ======================================================
     */
    Debug::getInstance ()->verboseMessage (
        "Creating subroutines for OP_PAR_LOOPs");

    FortranSubroutinesGeneration * newSubroutines = new FortranSubroutinesGeneration (
        project, declarations);

    newSubroutines->traverseInputFiles (project, preorder);

    /*
     * ======================================================
     * Output the generated subroutines to respective files
     * ======================================================
     */
    newSubroutines->unparse ();

    /*
     * ======================================================
     * Unparse input source files as calls to OP_PAR_LOOPs
     * will now have changed
     * ======================================================
     */
    project->unparse ();
  }

  Debug::getInstance ()->verboseMessage ("Translation completed");

  return 0;
}

