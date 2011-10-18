#include <CUDA.h>
#include <FortranTypesBuilder.h>
#include <Debug.h>
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
  using boost::lexical_cast;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
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
  using boost::lexical_cast;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
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
  using boost::lexical_cast;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
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
  using boost::lexical_cast;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
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

SgFunctionCallExp *
CUDA::createDeviceThreadSynchronisationCallStatement (SgScopeStatement * scope)
{
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using std::string;

  string const functionName = "syncthreads";

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine (functionName, scope);

  SgExprListExp * actualParameters = buildExprListExp ();

  SgFunctionCallExp * subroutineCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  return subroutineCall;
}

SgFunctionCallExp *
CUDA::createHostThreadSynchronisationCallStatement (SgScopeStatement * scope)
{
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using std::string;

  string const functionName = "cudaThreadSynchronize";

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction (functionName, scope);

  SgExprListExp * actualParameters = buildExprListExp ();

  SgFunctionCallExp * subroutineCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  return subroutineCall;
}
