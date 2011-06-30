#include <CPPDeclarations.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
CPPDeclarations::visit (SgNode * node)
{
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

CPPDeclarations::CPPDeclarations (SgProject * project)
{
  traverseInputFiles (project, preorder);
}
