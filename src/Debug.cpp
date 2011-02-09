#include <iostream>
#include "Debug.h"

Debug* Debug::debugInstance = 0;

Debug::Debug ()
{
  /*
   * Assume that there is no verbose and no debug information
   * to be output
   */
  verbose = false;
  debugLevel = 0;
}

Debug*
Debug::getInstance ()
{
  if (debugInstance == 0)
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
Debug::isVerbose ()
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
Debug::getDebugLevel ()
{
  return debugInstance->debugLevel;
}

void
Debug::verboseMessage (std::string message)
{
  using namespace std;

  if (debugInstance->verbose)
  {
    cout << message + "." << endl;
  }
}

void
Debug::debugMessage (std::string message, int debugLevel)
{
  using namespace std;

  if (debugLevel <= debugInstance->debugLevel && debugInstance->debugLevel
      > LOWEST_DEBUG_LEVEL)
  {
    cout << message + "." << endl;
  }
}
