#include <iostream>
#include <UDrawGraph.h>

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
UDrawGraph::visit (SgNode * node)
{
  std::cout << node->class_name () << std::endl;

  switch (node->variantT ())
  {
    case V_SgProcedureHeaderStatement:
    {
      break;
    }

    default:
    {
      break;
    }
  }
}

UDrawGraph::UDrawGraph ()
{

}
