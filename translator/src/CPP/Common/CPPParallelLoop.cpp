#include <CPPParallelLoop.h>
#include <rose.h>

CPPParallelLoop::CPPParallelLoop (SgFunctionCallExp * functionCallExpression,
    std::string fileName) :
  ParallelLoop (functionCallExpression, fileName)
{
}
