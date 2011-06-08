#include <CPPSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

CPPSubroutine::CPPSubroutine ()
{

}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

SgFunctionDeclaration *
CPPSubroutine::getFunctionDeclaration ()
{
  return functionDeclaration;
}
