#include <TargetBackend.h>
#include <Debug.h>

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
      Debug::getInstance ()->errorMessage ("Unknown backend selected",
          __FILE__, __LINE__);
    }
  }
}
