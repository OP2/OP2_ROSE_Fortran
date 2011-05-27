#include <cstdlib>
#include <Globals.h>
#include <CommonNamespaces.h>

Globals * Globals::globalsInstance = NULL;

Globals::Globals ()
{
  /*
   * ======================================================
   * Assume that the target backend is unknown
   * ======================================================
   */
  backend = TargetBackends::Unknown;
}

Globals *
Globals::getInstance ()
{
  if (globalsInstance == NULL)
  {
    globalsInstance = new Globals ();
  }
  return globalsInstance;
}

void
Globals::setTargetBackend (std::string const & backend)
{
  globalsInstance->backend = backend;
}

std::string
Globals::getTargetBackend () const
{
  return globalsInstance->backend;
}
