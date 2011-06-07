#include <CPPSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */
CPPSubroutine::CPPSubroutine (std::string const & subroutineName) :
  Subroutine (subroutineName)
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
