#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string.h>

#include "CommandLine.h"
#include "Debug.h"

CommandLine::CommandLine (int argc, char **argv)
{
  using namespace std;

  const std::string debugOption = "-d";
  const std::string verboseOption = "-v";
  bool debugMode = false;

  for (int i = 0; i < argc; ++i)
  {
    if (debugMode)
    {
      /*
       * The string in argv[i-1] was '-d'. Therefore, this
       * argument should represent the debug level, an integer
       */

      debugMode = false;

      try
      {
        /*
         * Check that the character array is an integer,
         * otherwise throw an exception
         */
        int debug = boost::lexical_cast<int> (argv[i]);

        /*
         * Only the ordained debug levels are permissible
         */
        if (debug < Debug::LOWEST_DEBUG_LEVEL || debug
            > Debug::HIGHEST_DEBUG_LEVEL)
        {
          throw debug;
        }
        else
        {
          Debug::getInstance ()->setDebugLevel (debug);
        }
      }
      catch (boost::bad_lexical_cast const&)
      {
        cout << "Error: '" << argv[i] << "' is not a valid debug level" << endl;
        exit (1);
      }
      catch (int debug)
      {
        cout << "Error: debug level " << argv[i];

        if (debug < Debug::LOWEST_DEBUG_LEVEL)
        {
          cout << " is too low.";
        }
        else
        {
          cout << " is too high";
        }

        cout << "Permissible range of debug levels = ["
            << Debug::LOWEST_DEBUG_LEVEL << ".." << Debug::HIGHEST_DEBUG_LEVEL
            << "]." << endl;

        exit (1);
      }
    }
    else
    {
      if (debugOption.compare (argv[i]) == 0)
      {
        /*
         * Debug flag recognised
         */
        debugMode = true;
      }
      else if (verboseOption.compare (argv[i]) == 0)
      {
        /*
         * Verbose flag recognised so set it globally
         */
        Debug::getInstance ()->setVerbose (true);
      }
      else
      {
        /*
         * Do not recognise it so assume it is a ROSE flag
         */
        ROSE_arguments.push_back (argv[i]);
      }
    }
  }
}

int
CommandLine::getNumberOfArguments ()
{
  return ROSE_arguments.size ();
}

char **
CommandLine::getArguments ()
{
  using namespace std;

  /* Allocate enough space for all the command-line
   * arguments recognised as ROSE ones
   */
  char** argv = new char*[ROSE_arguments.size ()];

  int i = 0;
  for (vector<string>::iterator it = ROSE_arguments.begin (); it
      != ROSE_arguments.end (); ++it)
  {
    /*
     * Allocate space for the character array which represents
     * a command-line argument
     */
    argv[i] = new char[it->size ()];

    /*
     * Have to do a 'strcpy' because 'c_str' returns a pointer
     * to a constant character array and we need a modifiable
     * character array
     */
    strcpy (argv[i], (*it).c_str ());

    /*
     * Advance the index into argv
     */
    i++;
  }

  return argv;
}
