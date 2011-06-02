#include <ModifyOP2Calls.h>
#include <Debug.h>

void
ModifyOP2Calls::visit (SgNode * node)
{
  using std::string;

  switch (node->variantT ())
  {
    case V_SgFunctionCallExp:
    {
      SgFunctionCallExp * functionCallExp = isSgFunctionCallExp (node);

      string const
          calleeName =
              functionCallExp->getAssociatedFunctionSymbol ()->get_name ().getString ();

      Debug::getInstance ()->debugMessage (calleeName, 1);

      break;
    }

    default:
    {

    }
  }
}

ModifyOP2Calls::ModifyOP2Calls (SgProject * project)
{
  this->project = project;
}
