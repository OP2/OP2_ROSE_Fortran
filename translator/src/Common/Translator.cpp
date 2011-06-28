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

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <rose.h>
#include <CommandLine.h>
#include <CommandLineOption.h>
#include <CommandLineOptionWithParameters.h>
#include <CommonNamespaces.h>
#include <Debug.h>
#include <Declarations.h>
#include <Globals.h>
#include <FortranSubroutinesGeneration.h>
#include <UDrawGraph.h>

class OxfordOption: public CommandLineOption
{
  public:

    virtual void
    run ()
    {
      Globals::getInstance ()->setRenderOxfordAPICalls ();
    }

    OxfordOption (std::string helpMessage, std::string longOption) :
      CommandLineOption (helpMessage, "", longOption)
    {
    }
};

class CUDAOption: public CommandLineOption
{
  public:

    virtual void
    run ()
    {
      Globals::getInstance ()->setTargetBackend (TargetBackends::CUDA);
    }

    CUDAOption (std::string helpMessage, std::string longOption) :
      CommandLineOption (helpMessage, "", longOption)
    {
    }
};

class OpenMPOption: public CommandLineOption
{
  public:

    virtual void
    run ()
    {
      Globals::getInstance ()->setTargetBackend (TargetBackends::OpenMP);
    }

    OpenMPOption (std::string helpMessage, std::string longOption) :
      CommandLineOption (helpMessage, "", longOption)
    {
    }
};

void
handleCPPProject (SgProject * project)
{
  using boost::iequals;

  if (Globals::getInstance ()->renderOxfordAPICalls ())
  {
    if (!iequals (Globals::getInstance ()->getTargetBackend (),
        TargetBackends::Unknown))
    {
      Debug::getInstance ()->errorMessage (
          "You have selected to generate code for "
              + Globals::getInstance ()->getTargetBackend ()
              + " and replace all OP2 calls with the Oxford-compliant API. These options are mutually exclusive");
    }
  }
  else if (iequals (Globals::getInstance ()->getTargetBackend (),
      TargetBackends::Unknown))
  {
    Debug::getInstance ()->errorMessage (
        "You have not selected a target backend on the command-line. Supported backends are: {"
            + TargetBackends::CUDA + ", " + TargetBackends::OpenMP + "}");
  }
}

void
handleFortranProject (SgProject * project)
{
  using boost::iequals;

  if (iequals (Globals::getInstance ()->getTargetBackend (),
      TargetBackends::Unknown))
  {
    Debug::getInstance ()->errorMessage (
        "You have not selected a target backend on the command-line. Supported backends are: {"
            + TargetBackends::CUDA + ", " + TargetBackends::OpenMP + "}");
  }

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

  FortranSubroutinesGeneration * newSubroutines =
      new FortranSubroutinesGeneration (project, declarations);

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

void
addCommandLineOptions ()
{
  CommandLine::getInstance ()->addOption (new CUDAOption ("Generate CUDA code",
      "cuda"));

  CommandLine::getInstance ()->addOption (new OpenMPOption (
      "Generate OpenMP code", "openmp"));

  CommandLine::getInstance ()->addOption (new OxfordOption (
      "Refactor OP2 calls to comply with Oxford API", "oxford"));

  CommandLine::getInstance ()->addUDrawGraphOption ();
}

int
main (int argc, char ** argv)
{
  addCommandLineOptions ();

  CommandLine::getInstance ()->parse (argc, argv);

  Debug::getInstance ()->verboseMessage ("Translation starting");

  /*
   * ======================================================
   * Pass the pre-processed command-line arguments and NOT
   * 'argc' and 'argv', otherwise ROSE will complain
   * ======================================================
   */
  SgProject * project = frontend (
      CommandLine::getInstance ()->getNumberOfArguments (),
      CommandLine::getInstance ()->getArguments ());

  ROSE_ASSERT (project != NULL);

  if (project->get_Cxx_only () == true)
  {
    Debug::getInstance ()->verboseMessage ("C++ project detected");

    handleCPPProject (project);
  }
  else
  {
    Debug::getInstance ()->verboseMessage ("Fortran project detected");

    handleFortranProject (project);
  }

  if (Globals::getInstance()->outputUDrawGraphs())
  {
    new UDrawGraph(project);
  }

  Debug::getInstance ()->verboseMessage ("Translation completed");

  return 0;
}

