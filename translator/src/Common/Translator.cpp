#include "CommandLine.h"
#include "TranslatorCommandLineOptions.h"
#include "Debug.h"
#include "Globals.h"
#include "Exceptions.h"
#include "FortranProgramDeclarationsAndDefinitions.h"
#include "FortranCUDASubroutinesGeneration.h"
#include "FortranOpenMPSubroutinesGeneration.h"
#include "CPPProgramDeclarationsAndDefinitions.h"
#include "CPPCUDASubroutinesGeneration.h"
#include "CPPOpenMPSubroutinesGeneration.h"
#include "CPPOpenCLSubroutinesGeneration.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <rose.h>

#include "CPPPreProcess.h"

template <class TGenerator>
  void
  unparseSourceFiles (SgProject * project, TGenerator * generator)
  {
    using std::vector;
    using std::string;

    class TreeVisitor: public AstSimpleProcessing
    {
      private:

        TGenerator * generator;

        SgProject * project;

        vector <string> outputFiles;

        string generatedFile;

      public:

        virtual void
        visit (SgNode * node)
        {
          using boost::iequals;
          using boost::filesystem::path;
          using boost::filesystem::system_complete;

          SgSourceFile * file = isSgSourceFile (node);

          if (file != NULL)
          {
            path p = system_complete (path (file->getFileName ()));

            if (generator->isDirty (p.filename ()))
            {
              Debug::getInstance ()->debugMessage ("Unparsing '"
                  + p.filename () + "'", Debug::FUNCTION_LEVEL, __FILE__,
                  __LINE__);

              outputFiles.push_back ("rose_" + p.filename ());

              file->unparse ();
            }
            else if (iequals (p.filename (), generator->getFileName ()))
            {
              Debug::getInstance ()->debugMessage ("Unparsing generated file '"
                  + p.filename () + "'", Debug::FUNCTION_LEVEL, __FILE__,
                  __LINE__);

              outputFiles.push_back (p.filename ());

              generatedFile = p.filename ();

              file->unparse ();
            }
            else
            {
              Debug::getInstance ()->debugMessage ("File '" + p.filename ()
                  + "' remains unchanged", Debug::FUNCTION_LEVEL, __FILE__,
                  __LINE__);

              outputFiles.push_back ("rose_" + p.filename ());

              file->unparse ();
            }
          }
        }

        TreeVisitor (TGenerator * generator, SgProject * project) :
          generator (generator), project (project)
        {
          using std::string;
          using std::ofstream;

          traverseInputFiles (this->project, preorder);

          string fileName;

          switch (Globals::getInstance ()->getTargetBackend ())
          {
            case TargetLanguage::CUDA:
            {
              fileName = ".translator.cuda";

              break;
            }

            case TargetLanguage::OPENMP:
            {
              fileName = ".translator.openmp";

              break;
            }

            case TargetLanguage::OPENCL:
            {
              fileName = ".translator.opencl";

              break;
            }

            default:
            {
              throw Exceptions::CommandLine::LanguageException (
                  "Unknown/unsupported backend selected");
            }
          }

          ofstream outputFile;

          outputFile.open (fileName.c_str ());

          outputFile << "generated=" + generatedFile;

          outputFile << std::endl;

          outputFile << "files=";

          for (vector <string>::iterator it = outputFiles.begin (); it
              != outputFiles.end (); ++it)
          {
            outputFile << *it << " ";
          }

          outputFile << std::endl;

          outputFile.close ();
        }
    };

    new TreeVisitor (generator, project);
  }

CPPSubroutinesGeneration *
handleCPPProject (SgProject * project)
{
  CPPSubroutinesGeneration * generator;

  switch (Globals::getInstance ()->getTargetBackend ())
  {
    case TargetLanguage::CUDA:
    {
      Debug::getInstance ()->debugMessage ("CUDA code generation selected",
          Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

      CPPProgramDeclarationsAndDefinitions * declarations =
          new CPPProgramDeclarationsAndDefinitions (project);

      generator = new CPPCUDASubroutinesGeneration (project, declarations);

      break;
    }

    case TargetLanguage::OPENMP:
    {
      Debug::getInstance ()->debugMessage ("OpenMP code generation selected",
          Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

      CPPProgramDeclarationsAndDefinitions * declarations =
          new CPPProgramDeclarationsAndDefinitions (project);

      generator = new CPPOpenMPSubroutinesGeneration (project, declarations);

      break;
    }

    case TargetLanguage::OPENCL:
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
      throw Exceptions::CommandLine::LanguageException (
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
    case TargetLanguage::CUDA:
    {
      Debug::getInstance ()->debugMessage ("CUDA code generation selected",
          Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

      FortranProgramDeclarationsAndDefinitions * declarations =
          new FortranProgramDeclarationsAndDefinitions (project);

      generator = new FortranCUDASubroutinesGeneration (project, declarations);

      break;
    }

    case TargetLanguage::OPENMP:
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
      throw Exceptions::CommandLine::LanguageException (
          "Unknown/unsupported backend selected");
    }
  }

  return generator;
}

void
checkFreeVariablesFileOption ()
{
  using std::string;

  if (Globals::getInstance ()->getFreeVariablesModuleName ().empty ())
  {
    throw Exceptions::ASTParsing::NoSourceFileException (
        "You have not supplied a file containing the declarations of free variables referenced within the user kernel functions");
  }
}

void
checkBackendOption ()
{
  using std::vector;
  using std::string;

  if (Globals::getInstance ()->getTargetBackend ()
      == TargetLanguage::UNKNOWN_BACKEND)
  {
    vector <TargetLanguage::BACKEND> values;
    values.push_back (TargetLanguage::CUDA);
    values.push_back (TargetLanguage::OPENMP);
    values.push_back (TargetLanguage::OPENCL);

    string backendsString;

    unsigned int i = 1;
    for (vector <TargetLanguage::BACKEND>::iterator it = values.begin (); it
        != values.end (); ++it, ++i)
    {
      backendsString += TargetLanguage::toString (*it);

      if (i < values.size ())
      {
        backendsString += ", ";
      }
    }

    throw Exceptions::CommandLine::LanguageException (
        "You have not selected a target backend on the command-line. Supported backends are: "
            + backendsString);
  }
}

void
addCommandLineOptions ()
{
  CommandLine::getInstance ()->addOption (new CUDAOption ("Generate CUDA code",
      TargetLanguage::toString (TargetLanguage::CUDA)));

  CommandLine::getInstance ()->addOption (
      new OpenMPOption ("Generate OpenMP code", TargetLanguage::toString (
          TargetLanguage::OPENMP)));

  CommandLine::getInstance ()->addOption (
      new OpenCLOption ("Generate OpenCL code", TargetLanguage::toString (
          TargetLanguage::OPENCL)));

  CommandLine::getInstance ()->addOption (new FreeVariablesModuleOption (
      "The module containing free variables referenced in user kernels"));

  CommandLine::getInstance ()->addOption (new OxfordOption (
      "Refactor OP2 calls to comply with Oxford API", "oxford"));
    
  CommandLine::getInstance ()->addOption (new PreprocessOption (
      "Preprocess OP2 declarations", "pre"));                                                                

  CommandLine::getInstance ()->addUDrawGraphOption ();
}

void
processUserSelections (SgProject * project)
{
  using std::string;
  using std::vector;

  if (project->get_Cxx_only ())
  {
    Debug::getInstance ()->debugMessage ("C++ project detected",
        Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

    Globals::getInstance ()->setHostLanguage (TargetLanguage::CPP);

    if (Globals::getInstance ()->preprocess ())
    {
      CPPProgramDeclarationsAndDefinitions * declarations =
        new CPPProgramDeclarationsAndDefinitions (project);
        
      new CPPPreProcess (project, declarations);
          
      project->unparse ();
    }
    else
		if (Globals::getInstance ()->renderOxfordAPICalls ())
		{
			if (Globals::getInstance ()->getTargetBackend ()
				!= TargetLanguage::UNKNOWN_BACKEND)
			{
				throw Exceptions::CommandLine::MutuallyExclusiveException (
				  "You have selected to generate code for " + toString (
				  Globals::getInstance ()->getTargetBackend ())
                  + " and replace all OP2 calls with calls complying with the Oxford API. These options are mutually exclusive");
			}

			CPPProgramDeclarationsAndDefinitions * declarations =
			new CPPProgramDeclarationsAndDefinitions (project);

			// new CPPModifyOP2CallsToComplyWithOxfordAPI (project, declarations);

			project->unparse ();
		}
		else
		{
			checkBackendOption ();

			CPPSubroutinesGeneration * generator = handleCPPProject (project);

			unparseSourceFiles (project, generator);
		}
  }
  else if (project->get_Fortran_only ())
  {
    Debug::getInstance ()->debugMessage ("Fortran project detected",
        Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

    Globals::getInstance ()->setHostLanguage (TargetLanguage::FORTRAN);

    checkBackendOption ();

    checkFreeVariablesFileOption ();

    FortranSubroutinesGeneration * generator = handleFortranProject (project);

    unparseSourceFiles (project, generator);
  }
  else
  {
    throw Exceptions::CommandLine::LanguageException (
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

  try
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

    vector <string> args;

    CommandLine::getInstance ()->getRoseArguments (args);

    SgProject * project = frontend (args);

    ROSE_ASSERT (project != NULL);

    Debug::getInstance ()->debugMessage ("Translation starting",
        Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

    processUserSelections (project);
  }
  catch (Exceptions::CUDA::GridDimensionException const & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::CUDA::GridDimensionException::returnValue;
  }
  catch (Exceptions::CUDA::SharedVariableTypeException const & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::CUDA::SharedVariableTypeException::returnValue;
  }
  catch (Exceptions::CUDA::ThreadBlockDimensionException const & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::CUDA::ThreadBlockDimensionException::returnValue;
  }
  catch (Exceptions::CommandLine::LanguageException const & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::CommandLine::LanguageException::returnValue;
  }
  catch (Exceptions::CommandLine::MutuallyExclusiveException const & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::CommandLine::MutuallyExclusiveException::returnValue;
  }
  catch (Exceptions::ParallelLoop::OpGblReadWriteException const & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::ParallelLoop::OpGblReadWriteException::returnValue;
  }
  catch (Exceptions::ParallelLoop::OpGblWriteException const & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::ParallelLoop::OpGblWriteException::returnValue;
  }
  catch (Exceptions::ParallelLoop::UnsupportedBaseTypeException const & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::ParallelLoop::UnsupportedBaseTypeException::returnValue;
  }
  catch (Exceptions::ParallelLoop::UnknownAccessException const & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::ParallelLoop::UnknownAccessException::returnValue;
  }
  catch (Exceptions::CodeGeneration::FortranVariableAttributeException const
      & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::CodeGeneration::FortranVariableAttributeException::returnValue;
  }
  catch (Exceptions::CodeGeneration::UnknownVariableException const & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::CodeGeneration::UnknownVariableException::returnValue;
  }
  catch (Exceptions::CodeGeneration::DuplicateVariableException const & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::CodeGeneration::DuplicateVariableException::returnValue;
  }
  catch (Exceptions::CodeGeneration::UnknownSubroutineException const & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::CodeGeneration::UnknownSubroutineException::returnValue;
  }
  catch (Exceptions::CodeGeneration::UnfoundStatementException const & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::CodeGeneration::UnfoundStatementException::returnValue;
  }
  catch (Exceptions::CodeGeneration::FileCreationException const & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::CodeGeneration::FileCreationException::returnValue;
  }
  catch (Exceptions::ASTParsing::NoSourceFileException const & e)
  {
    std::cout << e.what () << std::endl;

    return Exceptions::ASTParsing::NoSourceFileException::returnValue;
  }

  Debug::getInstance ()->debugMessage ("Translation completed",
      Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

  return 0;
}
