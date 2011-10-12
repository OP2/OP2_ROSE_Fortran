#include <TargetBackend.h>
#include <Debug.h>
#include <Exceptions.h>

std::string
TargetBackend::toString (BACKEND_VALUE backend)
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
      throw Exceptions::CommandLine::BackendException (
          "Unknown backend selected");
    }
  }
}
