#include <cstdlib>
#include <boost/filesystem.hpp>
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
  backend = TargetBackends::UNKNOWN;

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
Globals::setTargetBackend (TargetBackends::BACKEND_VALUE backend)
{
  this->backend = backend;
}

TargetBackends::BACKEND_VALUE
Globals::getTargetBackend () const
{
  return backend;
}

void
Globals::setRenderOxfordAPICalls ()
{
  oxfordOption = true;
}

bool
Globals::renderOxfordAPICalls () const
{
  return oxfordOption;
}

void
Globals::setOutputUDrawGraphs ()
{
  uDrawOption = true;
}

bool
Globals::outputUDrawGraphs () const
{
  return uDrawOption;
}

void
Globals::addInputFile (std::string const & fileName)
{
  inputFiles.push_back (fileName);
}

bool
Globals::isInputFile (std::string const & fileName) const
{
  return std::find (inputFiles.begin (), inputFiles.end (), fileName)
      != inputFiles.end ();
}
