#include <TargetBackend.h>
#include <Debug.h>
#include <Exceptions.h>

std::string
TargetLanguage::toString (FRONTEND frontend)
{
  switch (frontend)
  {
    case FORTRAN:
    {
      return "Fortran";
    }

    case CPP:
    {
      return "C++";
    }

    default:
    {
      throw Exceptions::CommandLine::LanguageException (
          "Unknown frontend language selected");
    }
  }
}

std::string
TargetLanguage::toString (BACKEND backend)
{
  switch (backend)
  {
    case CUDA:
    {
      return "CUDA";
    }

    case OPENMP:
    {
      return "OpenMP";
    }

    case OPENCL:
    {
      return "OpenCL";
    }

    default:
    {
      throw Exceptions::CommandLine::LanguageException (
          "Unknown backend language selected");
    }
  }
}
