#include <Globals.h>

Globals * Globals::globalsInstance = NULL;

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

Globals::Globals ()
{
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
