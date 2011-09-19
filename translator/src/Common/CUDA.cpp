#include <CUDA.h>
#include <Debug.h>
#include <FortranTypesBuilder.h>

namespace
{
  std::string const xField = "x";
  std::string const yField = "y";
  std::string const zField = "z";
}

SgDotExp *
CUDA::getThreadId (THREAD_BLOCK_DIMENSION dimension, SgScopeStatement * scope)
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using std::string;

  string const threadidx = "threadidx";

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
      Debug::getInstance ()->errorMessage ("Unknown dimension", __FILE__,
          __LINE__);
    }
  }
}

SgDotExp *
CUDA::getBlockId (GRID_DIMENSION dimension, SgScopeStatement * scope)
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using std::string;

  string const blockidx = "blockidx";

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
      Debug::getInstance ()->errorMessage ("Unknown dimension", __FILE__,
          __LINE__);
    }
  }
}

SgDotExp *
CUDA::getThreadBlockDimension (THREAD_BLOCK_DIMENSION dimension,
    SgScopeStatement * scope)
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using std::string;

  string const blockdim = "blockdim";

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
      Debug::getInstance ()->errorMessage ("Unknown dimension", __FILE__,
          __LINE__);
    }
  }
}

SgDotExp *
CUDA::getGridDimension (GRID_DIMENSION dimension, SgScopeStatement * scope)
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using std::string;

  string const griddim = "griddim";

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
      Debug::getInstance ()->errorMessage ("Unknown dimension", __FILE__,
          __LINE__);
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
