#include <cstdlib>
#include <typeinfo>
#include <iostream>

#include "Debug.h"

Debug * Debug::debugInstance = NULL;

Debug::Debug ()
{
  /*
   * Assume that there is no verbose and no debug information
   * to be output
   */
  verbose = false;
  debugLevel = 0;
}

Debug *
Debug::getInstance ()
{
  if (debugInstance == NULL)
  {
    debugInstance = new Debug ();
  }
  return debugInstance;
}

void
Debug::setVerbose (bool verbose)
{
  Debug::debugInstance->verbose = verbose;
}

bool
Debug::isVerbose () const
{
  return debugInstance->verbose;
}

void
Debug::setDebugLevel (int debugLevel)
{
  debugInstance->debugLevel = debugLevel;

  // By default turn verbose messages provided the debug level is above 0
  if (debugLevel > LOWEST_DEBUG_LEVEL)
  {
    debugInstance->verbose = true;
  }
}

int
Debug::getDebugLevel () const
{
  return debugInstance->debugLevel;
}

void
Debug::verboseMessage (std::string const & message) const
{
  using std::cout;
  using std::endl;

  if (debugInstance->verbose)
  {
    cout << message + "." << endl;
  }
}

void
Debug::debugMessage (std::string const & message, int const debugLevel) const
{
  using std::cout;
  using std::endl;

  if (debugLevel <= debugInstance->debugLevel && debugInstance->debugLevel
      > LOWEST_DEBUG_LEVEL)
  {
    cout << message + "." << endl;
  }
}

void
Debug::errorMessage (std::string const & message) const
{
  using std::cout;
  using std::endl;

  cout << message + "." << endl;
  exit (1);
}
