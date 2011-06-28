#include <cstdlib>
#include <Globals.h>
#include <CommonNamespaces.h>

Globals * Globals::globalsInstance = NULL;

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

Globals::Globals ()
{
  /*
   * ======================================================
   * Assume that the target backend is unknown
   * ======================================================
   */
  backend = TargetBackends::Unknown;

  /*
   * ======================================================
   * Assume other options have not been set
   * ======================================================
   */
  oxfordOption = false;

  uDrawOption = false;
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

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

void
Globals::setRenderOxfordAPICalls ()
{
  globalsInstance->oxfordOption = true;
}

bool
Globals::renderOxfordAPICalls () const
{
  return globalsInstance->oxfordOption;
}

void
Globals::setOutputUDrawGraphs ()
{
  globalsInstance->uDrawOption = true;
}

bool
Globals::outputUDrawGraphs () const
{
  return globalsInstance->uDrawOption;
}
