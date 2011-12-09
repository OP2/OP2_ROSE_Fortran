#include <cstdlib>
#include <boost/filesystem.hpp>
#include <Globals.h>
#include <Debug.h>

Globals * Globals::globalsInstance = NULL;

Globals::Globals ()
{
  /*
   * ======================================================
   * Assume that the frontend and backend are unknown
   * ======================================================
   */

  frontend = TargetLanguage::UNKNOWN_FRONTEND;

  backend = TargetLanguage::UNKNOWN_BACKEND;

  /*
   * ======================================================
   * Assume other options have not been set
   * ======================================================
   */
  oxfordOption = false;

  preprocessOption = false;

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
Globals::setHostLanguage (TargetLanguage::FRONTEND frontend)
{
  this->frontend = frontend;
}

TargetLanguage::FRONTEND
Globals::getHostLanguage () const
{
  return frontend;
}

void
Globals::setTargetBackend (TargetLanguage::BACKEND backend)
{
  this->backend = backend;
}

TargetLanguage::BACKEND
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
Globals::setPreprocess ()
{
    preprocessOption = true;
}

bool
Globals::preprocess () const
{
    return preprocessOption;
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
  Debug::getInstance ()->debugMessage ("Adding file '" + fileName + "'",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  inputFilenames.push_back (fileName);
}

bool
Globals::isInputFile (std::string const & fileName) const
{
  return std::find (inputFilenames.begin (), inputFilenames.end (), fileName)
      != inputFilenames.end ();
}

void
Globals::setFreeVariablesModuleName (std::string const & moduleName)
{
  freeVariablesModuleName = moduleName;
}

std::string const
Globals::getFreeVariablesModuleName () const
{
  return freeVariablesModuleName;
}
