#include <iostream>
#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <Debug.h>

Debug * Debug::debugInstance = NULL;

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

std::string
Debug::getFileName (std::string const & filePath)
{
  return filePath.substr (filePath.find_last_of ('/') + 1);
}

Debug::Debug ()
{
  debugLevel = 0;
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

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

void
Debug::debugMessage (std::string const & message, int const debugLevel,
    std::string const & filePath, int const lineNumber) const
{
  using boost::lexical_cast;
  using std::cout;
  using std::endl;
  using std::string;

  string const debugPrefix = "[" + debugInstance->getFileName (filePath) + ":"
      + lexical_cast <string> (lineNumber) + "] ";

  if (debugLevel <= debugInstance->debugLevel && debugInstance->debugLevel
      > LOWEST_DEBUG_LEVEL)
  {
    cout << debugPrefix + message + "." << endl;
  }
}

void
Debug::errorMessage (std::string const & message) const
{
  using std::cerr;
  using std::endl;

  cerr << message + "." << endl;
  exit (1);
}
