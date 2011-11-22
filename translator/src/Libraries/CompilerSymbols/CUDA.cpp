#include <CUDA.h>
#include <FortranTypesBuilder.h>
#include <Debug.h>
#include <Globals.h>
#include <Exceptions.h>
#include <rose.h>
#include <boost/lexical_cast.hpp>

namespace
{
  std::string const xField = "x";
  std::string const yField = "y";
  std::string const zField = "z";
}

SgDotExp *
CUDA::getThreadId (THREAD_BLOCK_DIMENSION dimension, SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using boost::lexical_cast;
  using std::string;

  string const threadidx = "threadIdx";

  switch (dimension)
  {
    case THREAD_X:
    {
      return buildDotExp (buildOpaqueVarRefExp (threadidx, scope),
          buildOpaqueVarRefExp (xField, scope));
    }

    case THREAD_Y:
    {
      return buildDotExp (buildOpaqueVarRefExp (threadidx, scope),
          buildOpaqueVarRefExp (yField, scope));
    }

    case THREAD_Z:
    {
      return buildDotExp (buildOpaqueVarRefExp (threadidx, scope),
          buildOpaqueVarRefExp (zField, scope));
    }

    default:
    {
      throw Exceptions::CUDA::ThreadBlockDimensionException (
          "Unknown thread block dimension '"
              + lexical_cast <string> (dimension) + "'");
    }
  }
}

SgDotExp *
CUDA::getBlockId (GRID_DIMENSION dimension, SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using boost::lexical_cast;
  using std::string;

  string const blockidx = "blockIdx";

  switch (dimension)
  {
    case BLOCK_X:
    {
      return buildDotExp (buildOpaqueVarRefExp (blockidx, scope),
          buildOpaqueVarRefExp (xField, scope));
    }

    case BLOCK_Y:
    {
      return buildDotExp (buildOpaqueVarRefExp (blockidx, scope),
          buildOpaqueVarRefExp (yField, scope));
    }

    default:
    {

      throw Exceptions::CUDA::GridDimensionException (
          "Unknown grid dimension '" + lexical_cast <string> (dimension) + "'");
    }
  }
}

SgDotExp *
CUDA::getThreadBlockDimension (THREAD_BLOCK_DIMENSION dimension,
    SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using boost::lexical_cast;
  using std::string;

  string const blockdim = "blockDim";

  switch (dimension)
  {
    case THREAD_X:
    {
      return buildDotExp (buildOpaqueVarRefExp (blockdim, scope),
          buildOpaqueVarRefExp (xField, scope));
    }

    case THREAD_Y:
    {
      return buildDotExp (buildOpaqueVarRefExp (blockdim, scope),
          buildOpaqueVarRefExp (yField, scope));
    }

    case THREAD_Z:
    {
      return buildDotExp (buildOpaqueVarRefExp (blockdim, scope),
          buildOpaqueVarRefExp (zField, scope));
    }

    default:
    {
      throw Exceptions::CUDA::ThreadBlockDimensionException (
          "Unknown thread block dimension '"
              + lexical_cast <string> (dimension) + "'");
    }
  }
}

SgDotExp *
CUDA::getGridDimension (GRID_DIMENSION dimension, SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using boost::lexical_cast;
  using std::string;

  string const griddim = "gridDim";

  switch (dimension)
  {
    case BLOCK_X:
    {
      return buildDotExp (buildOpaqueVarRefExp (griddim, scope),
          buildOpaqueVarRefExp (xField, scope));
    }

    case BLOCK_Y:
    {
      return buildDotExp (buildOpaqueVarRefExp (griddim, scope),
          buildOpaqueVarRefExp (yField, scope));
    }

    default:
    {
      throw Exceptions::CUDA::GridDimensionException (
          "Unknown thread block dimension '"
              + lexical_cast <string> (dimension) + "'");
    }
  }
}

SgVarRefExp *
CUDA::getWarpSizeReference (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueVarRefExp ("warpSize", scope);
}

SgFunctionCallExp *
CUDA::createDeviceThreadSynchronisationCallStatement (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  switch (Globals::getInstance ()->getHostLanguage ())
  {
    case TargetLanguage::FORTRAN:
    {
      SgFunctionSymbol * functionSymbol =
          FortranTypesBuilder::buildNewFortranSubroutine ("syncthreads", scope);

      return buildFunctionCallExp (functionSymbol, buildExprListExp ());
    }
    case TargetLanguage::CPP:
    {
      return buildFunctionCallExp ("__syncthreads", buildVoidType (),
          buildExprListExp (), scope);
    }
    default:
    {
      throw Exceptions::CommandLine::LanguageException ("Unknown host language");
    }
  }
}

SgFunctionCallExp *
CUDA::createHostThreadSynchronisationCallStatement (SgScopeStatement * scope)
{
  using namespace SageBuilder;

  switch (Globals::getInstance ()->getHostLanguage ())
  {
    case TargetLanguage::FORTRAN:
    {
      SgFunctionSymbol * functionSymbol =
          FortranTypesBuilder::buildNewFortranFunction(
              "cudaThreadSynchronize", scope);

      return buildFunctionCallExp (functionSymbol, buildExprListExp ());
    }
    case TargetLanguage::CPP:
    {
      return buildFunctionCallExp ("cudaThreadSynchronize", buildVoidType (),
          buildExprListExp (), scope);
    }
    default:
    {
      throw Exceptions::CommandLine::LanguageException ("Unknown host language");
    }
  }
}

SgFunctionCallExp *
CUDA::OP2RuntimeSupport::getReallocateReductionArraysCallStatement (
    SgScopeStatement * scope, SgVarRefExp * reductionBytesReference)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp (reductionBytesReference);

  return buildFunctionCallExp ("reallocReductArrays", buildVoidType (),
      actualParameters, scope);
}

SgFunctionCallExp *
CUDA::OP2RuntimeSupport::getMoveReductionArraysFromHostToDeviceCallStatement (
    SgScopeStatement * scope, SgVarRefExp * reductionBytesReference)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp (reductionBytesReference);

  return buildFunctionCallExp ("mvReductArraysToDevice", buildVoidType (),
      actualParameters, scope);
}

SgFunctionCallExp *
CUDA::OP2RuntimeSupport::getMoveReductionArraysFromDeviceToHostCallStatement (
    SgScopeStatement * scope, SgVarRefExp * reductionBytesReference)
{
  using namespace SageBuilder;

  SgExprListExp * actualParameters = buildExprListExp (reductionBytesReference);

  return buildFunctionCallExp ("mvReductArraysToHost", buildVoidType (),
      actualParameters, scope);
}

SgFunctionCallExp *
CUDA::OP2RuntimeSupport::getCUDASafeHostThreadSynchronisationCallStatement (
    SgScopeStatement * scope)
{
  using namespace SageBuilder;

  SgFunctionCallExp * threadSynchCall =
      CUDA::createHostThreadSynchronisationCallStatement (scope);

  SgExprListExp * actualParameters = buildExprListExp (threadSynchCall);

  return buildFunctionCallExp ("cutilSafeCall", buildVoidType (),
      actualParameters, scope);
}

SgVarRefExp *
CUDA::OP2RuntimeSupport::getPointerToMemoryAllocatedForHostReductionArray (
    SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueVarRefExp ("OP_reduct_h", scope);
}

SgVarRefExp *
CUDA::OP2RuntimeSupport::getPointerToMemoryAllocatedForDeviceReductionArray (
    SgScopeStatement * scope)
{
  using namespace SageBuilder;

  return buildOpaqueVarRefExp ("OP_reduct_d", scope);
}
