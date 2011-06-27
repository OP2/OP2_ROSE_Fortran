#include <iostream>
#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <Debug.h>

Debug * Debug::debugInstance = NULL;

Debug::Debug ()
{
  /*
   * ======================================================
   * Assume that there is no verbose and no debug information
   * to be output
   * ======================================================
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
Debug::setVerbose ()
{
  Debug::debugInstance->verbose = true;
}

bool
Debug::isVerbose () const
{
  return debugInstance->verbose;
}

void
Debug::setDebugLevel (std::string debugLevelString)
{
  using boost::bad_lexical_cast;
  using boost::lexical_cast;
  using std::cout;

  try
  {
    /*
     * ======================================================
     * Check that the argument is an integer, otherwise throw
     * an exception
     * ======================================================
     */
    int level = lexical_cast <int> (debugLevelString);

    /*
     * ======================================================
     * Only the ordained debug levels are permissible
     * ======================================================
     */
    if (level < LOWEST_DEBUG_LEVEL || level > HIGHEST_DEBUG_LEVEL)
    {
      throw level;
    }
    else
    {
      debugInstance->debugLevel = level;

      /*
       * ======================================================
       * By default turn verbose messages provided the debug
       * level is above 0
       * ======================================================
       */
      if (debugInstance->debugLevel > LOWEST_DEBUG_LEVEL)
      {
        debugInstance->verbose = true;
      }
    }
  }
  catch (bad_lexical_cast const &)
  {
    cout << "Error: '" << debugLevelString << "' is not a valid debug level\n";
    exit (1);
  }
  catch (int debug)
  {
    cout << "Error: debug level " << debugLevelString;

    if (debug < LOWEST_DEBUG_LEVEL)
    {
      cout << " is too low.";
    }
    else
    {
      cout << " is too high";
    }

    cout << "Permissible range of debug levels = [" << LOWEST_DEBUG_LEVEL
        << ".." << HIGHEST_DEBUG_LEVEL << "].\n";

    exit (1);
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
