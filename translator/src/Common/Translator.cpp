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
#include <CommandLineOption.h>
#include <CommandLineOptionWithParameters.h>
#include <CommonNamespaces.h>
#include <Debug.h>
#include <Declarations.h>
#include <Globals.h>
#include <UDrawGraph.h>
#include <FortranSubroutinesGeneration.h>
#include <FortranCUDASubroutinesGeneration.h>
#include <FortranOpenMPSubroutinesGeneration.h>

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
      Globals::getInstance ()->setTargetBackend (TargetBackends::OPENMP);
    }

    OpenMPOption (std::string helpMessage, std::string longOption) :
      CommandLineOption (helpMessage, "", longOption)
    {
    }
};

class OpenCLOption: public CommandLineOption
{
  public:

    virtual void
    run ()
    {
      Debug::getInstance ()->errorMessage ("Sorry: OpenCL backend not yet supported");
    }

    OpenCLOption (std::string helpMessage, std::string longOption) :
      CommandLineOption (helpMessage, "", longOption)
    {
    }
};

void
handleCPPProject (SgProject * project)
{
}

void
handleFortranProject (SgProject * project)
{
  /*
   * ======================================================
   * Obtain all OP2 declarations
   * ======================================================
   */
  Debug::getInstance ()->verboseMessage (
      "Retrieving declarations in source files");

  FortranDeclarations * declarations = new FortranDeclarations (project);

  /*
   * ======================================================
   * Create and output the subroutines implementing an
   * OP_PAR_LOOP
   * ======================================================
   */
  Debug::getInstance ()->verboseMessage (
      "Creating subroutines for OP_PAR_LOOPs");

  switch (Globals::getInstance ()->getTargetBackend ())
  {
    case TargetBackends::CUDA:
    {
      FortranCUDASubroutinesGeneration * newSubroutines =
          new FortranCUDASubroutinesGeneration (declarations);

      newSubroutines->traverseInputFiles (project, preorder);

      newSubroutines->unparse ();

      break;
    }

    case TargetBackends::OPENMP:
    {
      FortranOpenMPSubroutinesGeneration * newSubroutines =
          new FortranOpenMPSubroutinesGeneration (declarations);

      newSubroutines->traverseInputFiles (project, preorder);

      newSubroutines->unparse ();

      break;
    }

    default:
    {
      break;
    }
  }

  /*
   * ======================================================
   * Unparse input source files as calls to OP_PAR_LOOPs
   * will now have changed
   * ======================================================
   */
  project->unparse ();
}

void
checkBackendOption ()
{
  using std::vector;
  using std::string;

  if (Globals::getInstance ()->getTargetBackend () == TargetBackends::UNKNOWN)
  {
    vector <TargetBackends::BACKEND_VALUE> values;
    values.push_back (TargetBackends::CUDA);
    values.push_back (TargetBackends::OPENMP);

    string backendsString;

    int i = 0;
    for (vector <TargetBackends::BACKEND_VALUE>::iterator it = values.begin (); it
        != values.end (); ++it, ++i)
    {
      backendsString += TargetBackends::toString (*it);

      if (i < values.size ())
      {
        backendsString += ", ";
      }
    }

    Debug::getInstance ()->errorMessage (
        "You have not selected a target backend on the command-line. Supported backends are: "
            + backendsString);
  }
}

void
addCommandLineOptions ()
{
  CommandLine::getInstance ()->addOption (new CUDAOption ("Generate CUDA code",
      TargetBackends::toString (TargetBackends::CUDA)));

  CommandLine::getInstance ()->addOption (
      new OpenMPOption ("Generate OpenMP code", TargetBackends::toString (
          TargetBackends::OPENMP)));

  CommandLine::getInstance ()->addOption (
      new OpenCLOption ("Generate OpenCL code", TargetBackends::toString (
          TargetBackends::OPENCL)));

  CommandLine::getInstance ()->addOption (new OxfordOption (
      "Refactor OP2 calls to comply with Oxford API", "oxford"));

  CommandLine::getInstance ()->addUDrawGraphOption ();
}

int
main (int argc, char ** argv)
{
  using std::string;

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

    if (Globals::getInstance ()->renderOxfordAPICalls ())
    {
      if (Globals::getInstance ()->getTargetBackend ()
          != TargetBackends::UNKNOWN)
      {
        Debug::getInstance ()->errorMessage (
            "You have selected to generate code for " + toString (
                Globals::getInstance ()->getTargetBackend ())
                + " and replace all OP2 calls with the Oxford-compliant API. These options are mutually exclusive");
      }
    }
    else
    {
      checkBackendOption ();
    }

    handleCPPProject (project);
  }
  else
  {
    Debug::getInstance ()->verboseMessage ("Fortran project detected");

    checkBackendOption ();

    handleFortranProject (project);
  }

  if (Globals::getInstance ()->outputUDrawGraphs ())
  {
    new UDrawGraph (project, "ast");
  }

  Debug::getInstance ()->verboseMessage ("Translation completed");

  return 0;
}

