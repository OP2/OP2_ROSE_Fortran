#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
CPPProgramDeclarationsAndDefinitions::visit (SgNode * node)
{
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

CPPProgramDeclarationsAndDefinitions::CPPProgramDeclarationsAndDefinitions (SgProject * project)
{
  traverseInputFiles (project, preorder);
}
