#include <CommandLine.h>
#include <CommandLineOption.h>
#include <CommandLineOptionWithParameters.h>
#include <TargetBackend.h>
#include <Debug.h>
#include <Globals.h>
#include <Exceptions.h>
#include <UDrawGraph.h>
#include <FortranProgramDeclarationsAndDefinitions.h>
#include <FortranCUDASubroutinesGeneration.h>
#include <FortranOpenMPSubroutinesGeneration.h>
#include <CPPModifyOP2CallsToComplyWithOxfordAPI.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPCUDASubroutinesGeneration.h>
#include <CPPOpenCLSubroutinesGeneration.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <rose.h>

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

CPPSubroutinesGeneration *
handleCPPProject (SgProject * project)
{
  CPPSubroutinesGeneration * generator;

  switch (Globals::getInstance ()->getTargetBackend ())
  {
    case TargetBackend::CUDA:
    {
      Debug::getInstance ()->debugMessage ("CUDA code generation selected",
          Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

      CPPProgramDeclarationsAndDefinitions * declarations =
          new CPPProgramDeclarationsAndDefinitions (project);

      generator = new CPPCUDASubroutinesGeneration (project, declarations);

      break;
    }

    case TargetBackend::OPENCL:
    {
      Debug::getInstance ()->debugMessage ("OpenCL code generation selected",
          Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

      CPPProgramDeclarationsAndDefinitions * declarations =
          new CPPProgramDeclarationsAndDefinitions (project);

      generator = new CPPOpenCLSubroutinesGeneration (project, declarations);

      break;
    }

    default:
    {
      throw Exceptions::CommandLine::BackendException (
          "Unknown/unsupported backend selected");
    }
  }

  return generator;
}

FortranSubroutinesGeneration *
handleFortranProject (SgProject * project)
{
  FortranSubroutinesGeneration * generator;

  switch (Globals::getInstance ()->getTargetBackend ())
  {
    case TargetBackend::CUDA:
    {
      Debug::getInstance ()->debugMessage ("CUDA code generation selected",
          Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

      FortranProgramDeclarationsAndDefinitions * declarations =
          new FortranProgramDeclarationsAndDefinitions (project);

      generator = new FortranCUDASubroutinesGeneration (project, declarations);

      break;
    }

    case TargetBackend::OPENMP:
    {
      Debug::getInstance ()->debugMessage ("OpenMP code generation selected",
          Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

      FortranProgramDeclarationsAndDefinitions * declarations =
          new FortranProgramDeclarationsAndDefinitions (project);

      generator
          = new FortranOpenMPSubroutinesGeneration (project, declarations);

      break;
    }

    default:
    {
      throw Exceptions::CommandLine::BackendException (
          "Unknown/unsupported backend selected");
    }
  }

  return generator;
}

void
unparseOriginalSourceFiles (SgProject * project,
    FortranSubroutinesGeneration * generator)
{
  class TreeVisitor: public AstSimpleProcessing
  {
    private:

      FortranSubroutinesGeneration * generator;

    public:

      TreeVisitor (FortranSubroutinesGeneration * generator) :
        generator (generator)
      {
      }

      virtual void
      visit (SgNode * node)
      {
        using boost::filesystem::path;
        using boost::filesystem::system_complete;

        SgSourceFile * file = isSgSourceFile (node);
        if (file != NULL)
        {
          path p = system_complete (
              path (isSgSourceFile (node)->getFileName ()));

          if (generator->isDirty (p.filename ()))
          {
            Debug::getInstance ()->debugMessage ("Unparsing '" + p.filename ()
                + "'", Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

            file->unparse ();
          }
          else
          {
            Debug::getInstance ()->debugMessage ("File '" + p.filename ()
                + "' remains unchanged", Debug::FUNCTION_LEVEL, __FILE__,
                __LINE__);
          }
        }
      }
  };

  TreeVisitor * visitor = new TreeVisitor (generator);

  visitor->traverse (project, preorder);
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
    values.push_back (TargetBackend::OPENCL);

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

    Exceptions::CommandLine::BackendException (
        "You have not selected a target backend on the command-line. Supported backends are: "
            + backendsString);
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
  using std::string;
  using std::vector;

  if (project->get_Cxx_only () == true)
  {
    Debug::getInstance ()->debugMessage ("C++ project detected",
        Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

    if (Globals::getInstance ()->renderOxfordAPICalls ())
    {
      if (Globals::getInstance ()->getTargetBackend ()
          != TargetBackend::UNKNOWN)
      {
        throw Exceptions::CommandLine::MutuallyExclusiveException (
            "You have selected to generate code for " + toString (
                Globals::getInstance ()->getTargetBackend ())
                + " and replace all OP2 calls with calls complying with the Oxford API. These options are mutually exclusive");
      }

      CPPProgramDeclarationsAndDefinitions * declarations =
          new CPPProgramDeclarationsAndDefinitions (project);

      new CPPModifyOP2CallsToComplyWithOxfordAPI (project, declarations);

      project->unparse ();
    }
    else
    {
      checkBackendOption ();

      CPPSubroutinesGeneration * generator = handleCPPProject (project);
    }
  }
  else if (project->get_Fortran_only () == true)
  {
    Debug::getInstance ()->debugMessage ("Fortran project detected",
        Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

    checkBackendOption ();

    FortranSubroutinesGeneration * generator = handleFortranProject (project);

    unparseOriginalSourceFiles (project, generator);
  }
  else
  {
    throw Exceptions::CommandLine::FrontendException (
        "The translator does not supported the programming language of the given files");
  }
}

int
main (int argc, char ** argv)
{
  using std::string;
  using std::vector;
  using std::cout;
  using std::endl;

  addCommandLineOptions ();

  CommandLine::getInstance ()->parse (argc, argv);

  /*
   * ======================================================
   * Pass the pre-processed command-line arguments and NOT
   * 'argc' and 'argv', otherwise ROSE will complain it does
   * not recognise particular options
   * ======================================================
   */

  vector <string> args;

  CommandLine::getInstance ()->getRoseArguments (args);

  SgProject * project = frontend (args);

  ROSE_ASSERT (project != NULL);

  Debug::getInstance ()->debugMessage ("Translation starting",
      Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

  try
  {
    processUserSelections (project);
  }
  catch (Exceptions::CUDA::GridDimensionException const & e)
  {
    std::cout << e.what () << std::endl;
  }
  catch (Exceptions::CUDA::SharedVariableTypeException const & e)
  {
    std::cout << e.what () << std::endl;
  }
  catch (Exceptions::CUDA::ThreadBlockDimensionException const & e)
  {
    std::cout << e.what () << std::endl;
  }
  catch (Exceptions::CommandLine::BackendException const & e)
  {
    std::cout << e.what () << std::endl;
  }
  catch (Exceptions::CommandLine::FrontendException const & e)
  {
    std::cout << e.what () << std::endl;
  }
  catch (Exceptions::CommandLine::MutuallyExclusiveException const & e)
  {
    std::cout << e.what () << std::endl;
  }
  catch (Exceptions::ParallelLoop::OpGblReadWriteException const & e)
  {
    std::cout << e.what () << std::endl;
  }
  catch (Exceptions::ParallelLoop::OpGblWriteException const & e)
  {
    std::cout << e.what () << std::endl;
  }
  catch (Exceptions::ParallelLoop::UnsupportedBaseTypeException const & e)
  {
    std::cout << e.what () << std::endl;
  }
  catch (Exceptions::ParallelLoop::UnknownAccessException const & e)
  {
    std::cout << e.what () << std::endl;
  }
  catch (Exceptions::CodeGeneration::FortranVariableAttributeException const
      & e)
  {
    std::cout << e.what () << std::endl;
  }
  catch (Exceptions::CodeGeneration::UnknownVariableException const & e)
  {
    std::cout << e.what () << std::endl;
  }
  catch (Exceptions::CodeGeneration::UnknownSubroutineException const & e)
  {
    std::cout << e.what () << std::endl;
  }
  catch (Exceptions::CodeGeneration::UnfoundStatementException const & e)
  {
    std::cout << e.what () << std::endl;
  }
  catch (Exceptions::CodeGeneration::FileCreationException const & e)
  {
    std::cout << e.what () << std::endl;
  }

  Debug::getInstance ()->debugMessage ("Translation completed",
      Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

  return 0;
}

