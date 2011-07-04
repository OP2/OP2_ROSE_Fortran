/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Contains the entry point (i.e. 'main') of the translator.
 * It does the following:
 *
 * 1) Adds viable options to the command line and parses them.
 * This process divides the command-line arguments into two sets:
 * those recognised by our translator and those recognised by ROSE
 *
 * 2) Obtains the definitions of OP2 variables (i.e. sets, maps,
 * data, etc.) and subroutines found in the source files to be
 * processed
 *
 * 3) Creates the subroutines which implement an OP_PAR_LOOP
 *
 * 4) Unparses the generated subroutines to files; there is one
 * file per unique OP_PAR_LOOP call
 *
 * 5) Unparses the original source files to reflect the changes
 * made to OP_PAR_LOOP calls
 */

#include <boost/algorithm/string.hpp>
#include <rose.h>
#include <CommandLine.h>
#include <CommandLineOption.h>
#include <CommandLineOptionWithParameters.h>
#include <CommonNamespaces.h>
#include <Debug.h>
#include <Globals.h>
#include <UDrawGraph.h>
#include <FortranProgramDeclarationsAndDefinitions.h>
#include <FortranCUDASubroutinesGeneration.h>
#include <FortranOpenMPSubroutinesGeneration.h>
#include <CPPModifyOP2CallsToComplyWithOxfordAPI.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPSubroutinesGeneration.h>

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
      Debug::getInstance ()->errorMessage (
          "Sorry: OpenCL backend not yet supported");
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
  FortranProgramDeclarationsAndDefinitions * declarations =
      new FortranProgramDeclarationsAndDefinitions (project);

  switch (Globals::getInstance ()->getTargetBackend ())
  {
    case TargetBackends::CUDA:
    {
      new FortranCUDASubroutinesGeneration (project, declarations);

      break;
    }

    case TargetBackends::OPENMP:
    {
      new FortranOpenMPSubroutinesGeneration (project, declarations);

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

    unsigned int i = 1;
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
  addCommandLineOptions ();

  CommandLine::getInstance ()->parse (argc, argv);

  Debug::getInstance ()->debugMessage ("Translation starting", 1, __FILE__,
      __LINE__);

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
    Debug::getInstance ()->debugMessage ("C++ project detected", 1, __FILE__,
        __LINE__);

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

      CPPProgramDeclarationsAndDefinitions * declarations =
          new CPPProgramDeclarationsAndDefinitions (project);

      CPPModifyOP2CallsToComplyWithOxfordAPI * modifyCalls =
          new CPPModifyOP2CallsToComplyWithOxfordAPI (project, declarations);

      project->unparse ();
    }
    else
    {
      checkBackendOption ();

      handleCPPProject (project);
    }
  }
  else
  {
    Debug::getInstance ()->debugMessage ("Fortran project detected", 1,
        __FILE__, __LINE__);

    checkBackendOption ();

    handleFortranProject (project);
  }

  if (Globals::getInstance ()->outputUDrawGraphs ())
  {
    new UDrawGraph (project, "ast");
  }

  Debug::getInstance ()->debugMessage ("Translation completed", 1, __FILE__,
      __LINE__);

  return 0;
}

