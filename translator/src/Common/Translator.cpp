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
#include <TargetBackend.h>
#include <Debug.h>
#include <Globals.h>
#include <UDrawGraph.h>
#include <FortranProgramDeclarationsAndDefinitions.h>
#include <FortranCUDASubroutinesGeneration.h>
#include <FortranOpenMPSubroutinesGeneration.h>
#include <CPPModifyOP2CallsToComplyWithOxfordAPI.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPCUDASubroutinesGeneration.h>
#include <CPPOpenCLSubroutinesGeneration.h>

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
      Globals::getInstance ()->setTargetBackend (TargetBackend::CUDA);
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
      Globals::getInstance ()->setTargetBackend (TargetBackend::OPENMP);
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
      Globals::getInstance ()->setTargetBackend (TargetBackend::OPENCL);
    }

    OpenCLOption (std::string helpMessage, std::string longOption) :
      CommandLineOption (helpMessage, "", longOption)
    {
    }
};

void
handleCPPProject (SgProject * project)
{
  CPPProgramDeclarationsAndDefinitions * declarations =
      new CPPProgramDeclarationsAndDefinitions (project);

  switch (Globals::getInstance ()->getTargetBackend ())
  {
    case TargetBackend::CUDA:
    {
      new CPPCUDASubroutinesGeneration (project, declarations);

      break;
    }

    case TargetBackend::OPENMP:
    {
      Debug::getInstance ()->errorMessage (
          "OpenMP code generation not yet supported in C++", __FILE__, __LINE__);

      break;
    }

    case TargetBackend::OPENCL:
    {
      Debug::getInstance ()->debugMessage ("OpenCL code generation selected",
          Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

      new CPPOpenCLSubroutinesGeneration (project, declarations);

      break;
    }

    default:
    {
      Debug::getInstance ()->errorMessage ("Unknown backend selected",
          __FILE__, __LINE__);

      break;
    }
  }
}

void
handleFortranProject (SgProject * project)
{
  switch (Globals::getInstance ()->getTargetBackend ())
  {
    case TargetBackend::CUDA:
    {
      Debug::getInstance ()->debugMessage ("CUDA code generation selected",
          Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

      FortranProgramDeclarationsAndDefinitions * declarations =
          new FortranProgramDeclarationsAndDefinitions (project);

      new FortranCUDASubroutinesGeneration (project, declarations);

      break;
    }

    case TargetBackend::OPENMP:
    {
      Debug::getInstance ()->debugMessage ("OpenMP code generation selected",
          Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

      FortranProgramDeclarationsAndDefinitions * declarations =
          new FortranProgramDeclarationsAndDefinitions (project);

      new FortranOpenMPSubroutinesGeneration (project, declarations);

      break;
    }

    case TargetBackend::OPENCL:
    {
      Debug::getInstance ()->errorMessage (
          "OpenCL code generation not yet supported in Fortran", __FILE__,
          __LINE__);

      FortranProgramDeclarationsAndDefinitions * declarations =
          new FortranProgramDeclarationsAndDefinitions (project);

      break;
    }

    default:
    {
      Debug::getInstance ()->errorMessage ("Unknown backend selected",
          __FILE__, __LINE__);

      break;
    }
  }
}

void
checkBackendOption ()
{
  using std::vector;
  using std::string;

  if (Globals::getInstance ()->getTargetBackend () == TargetBackend::UNKNOWN)
  {
    vector <TargetBackend::BACKEND_VALUE> values;
    values.push_back (TargetBackend::CUDA);
    values.push_back (TargetBackend::OPENMP);

    string backendsString;

    unsigned int i = 1;
    for (vector <TargetBackend::BACKEND_VALUE>::iterator it = values.begin (); it
        != values.end (); ++it, ++i)
    {
      backendsString += TargetBackend::toString (*it);

      if (i < values.size ())
      {
        backendsString += ", ";
      }
    }

    Debug::getInstance ()->errorMessage (
        "You have not selected a target backend on the command-line. Supported backends are: "
            + backendsString, __FILE__, __LINE__);
  }
}

void
addCommandLineOptions ()
{
  CommandLine::getInstance ()->addOption (new CUDAOption ("Generate CUDA code",
      TargetBackend::toString (TargetBackend::CUDA)));

  CommandLine::getInstance ()->addOption (new OpenMPOption (
      "Generate OpenMP code", TargetBackend::toString (TargetBackend::OPENMP)));

  CommandLine::getInstance ()->addOption (new OpenCLOption (
      "Generate OpenCL code", TargetBackend::toString (TargetBackend::OPENCL)));

  CommandLine::getInstance ()->addOption (new OxfordOption (
      "Refactor OP2 calls to comply with Oxford API", "oxford"));

  CommandLine::getInstance ()->addUDrawGraphOption ();
}

void
processUserSelections (SgProject * project)
{
  if (project->get_Cxx_only () == true)
  {
    Debug::getInstance ()->debugMessage ("C++ project detected",
        Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

    if (Globals::getInstance ()->renderOxfordAPICalls ())
    {
      if (Globals::getInstance ()->getTargetBackend ()
          != TargetBackend::UNKNOWN)
      {
        Debug::getInstance ()->errorMessage (
            "You have selected to generate code for " + toString (
                Globals::getInstance ()->getTargetBackend ())
                + " and replace all OP2 calls with the Oxford-compliant API. These options are mutually exclusive",
            __FILE__, __LINE__);
      }

      CPPProgramDeclarationsAndDefinitions * declarations =
          new CPPProgramDeclarationsAndDefinitions (project);

      new CPPModifyOP2CallsToComplyWithOxfordAPI (project, declarations);

      project->unparse ();
    }
    else
    {
      checkBackendOption ();

      //handleCPPProject (project);

      project->unparse ();
    }
  }
  else if (project->get_Fortran_only () == true)
  {
    Debug::getInstance ()->debugMessage ("Fortran project detected",
        Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

    checkBackendOption ();

    handleFortranProject (project);

    project->unparse ();
  }
  else
  {
    Debug::getInstance ()->errorMessage (
        "The translator does not supported the programming language of the given files",
        __FILE__, __LINE__);
  }
}

int
main (int argc, char ** argv)
{
  addCommandLineOptions ();

  CommandLine::getInstance ()->parse (argc, argv);

  /*
   * ======================================================
   * Pass the pre-processed command-line arguments and NOT
   * 'argc' and 'argv', otherwise ROSE will complain it does
   * not recognise particular options
   * ======================================================
   */

  std::vector <std::string> args;

  CommandLine::getInstance ()->getRoseArguments (args);

  SgProject * project = frontend (args);

  ROSE_ASSERT (project != NULL);

  Debug::getInstance ()->debugMessage ("Translation starting",
      Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

  processUserSelections (project);

  Debug::getInstance ()->debugMessage ("Translation completed",
      Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

  return 0;
}

