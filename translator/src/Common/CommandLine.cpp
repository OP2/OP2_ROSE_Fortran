#include <iostream>
#include <CommandLine.h>
#include <Debug.h>
#include <Globals.h>
#include <CommonNamespaces.h>

CommandLine * CommandLine::globalsInstance = NULL;

/*
 * ======================================================
 * Help option
 * ======================================================
 */

void
CommandLine::HelpOption::run ()
{
  CommandLine::getInstance ()->outputOptions ();
}

CommandLine::HelpOption::HelpOption (std::string helpMessage,
    std::string shortOption, std::string longOption) :
  CommandLineOption (helpMessage, shortOption, longOption)
{
}

/*
 * ======================================================
 * Verbose option
 * ======================================================
 */

void
CommandLine::VerboseOption::run ()
{
  Debug::getInstance ()->setVerbose ();
}

CommandLine::VerboseOption::VerboseOption (std::string helpMessage,
    std::string shortOption, std::string longOption) :
  CommandLineOption (helpMessage, shortOption, longOption)
{
}

/*
 * ======================================================
 * Debug option
 * ======================================================
 */

void
CommandLine::DebugOption::run ()
{
  Debug::getInstance ()->setDebugLevel (getParameter ());
}

CommandLine::DebugOption::DebugOption (std::string helpMessage,
    std::string parameterName, std::string shortOption, std::string longOption) :
  CommandLineOptionWithParameters (helpMessage, parameterName, shortOption,
      longOption)
{
}

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
CommandLine::addHelpOption ()
{
  HelpOption * help = new HelpOption ("Display this message", "h", "help");

  addOption (help);
}

void
CommandLine::addVerboseOption ()
{
  VerboseOption * verbose = new VerboseOption ("Be verbose", "v", "verbose");

  addOption (verbose);
}

void
CommandLine::addDebugOption ()
{
  DebugOption * debug = new DebugOption ("Set the debug level", "n", "d",
      "debug");

  addOption (debug);
}

CommandLine::CommandLine ()
{
  addHelpOption ();

  addVerboseOption ();

  addDebugOption ();
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
CommandLine::outputOptions ()
{
  using std::map;
  using std::string;
  using std::cout;
  using std::endl;

  /*
   * ======================================================
   * First output the usage line
   * ======================================================
   */

  if (toolName.empty () == false)
  {
    cout << "Usage: " + toolName;
  }
  else
  {
    cout << "Options:";
  }

  for (map <string, CommandLineOption *>::iterator it = otherArguments.begin (); it
      != otherArguments.end (); ++it)
  {
    CommandLineOption * option = it->second;

    CommandLineOptionWithParameters * paramterisedOption =
        dynamic_cast <CommandLineOptionWithParameters *> (option);\

    if (option->hasShortOption ())
    {
      cout << " [" + option->getShortOption ();

      if (paramterisedOption != NULL)
      {
        cout << " <" + paramterisedOption->getParameterName () + ">";
      }

      cout << "]";
    }

    if (option->hasLongOption ())
    {
      cout << " [" + option->getLongOption ();

      if (paramterisedOption != NULL)
      {
        cout << " <" + paramterisedOption->getParameterName () + ">";
      }

      cout << "]";
    }
  }

  cout << endl << endl;

  /*
   * ======================================================
   * Now detail the options
   * ======================================================
   */

  int maxLengthOfLongOption = 0;

  for (map <string, CommandLineOption *>::iterator it = otherArguments.begin (); it
      != otherArguments.end (); ++it)
  {
    CommandLineOption * option = it->second;

    if (option->hasLongOption ())
    {
      int length = option->getLongOption ().size ();

      if (length > maxLengthOfLongOption)
      {
        maxLengthOfLongOption = length;
      }
    }
  }

  for (map <string, CommandLineOption *>::iterator it = otherArguments.begin (); it
      != otherArguments.end (); ++it)
  {
    CommandLineOption * option = it->second;

    int blanksToPrint = maxLengthOfLongOption + 1;

    if (option->hasShortOption ())
    {
      cout << "[" + option->getShortOption () + "]";
    }

    if (option->hasLongOption ())
    {
      if (option->hasShortOption ())
      {
        cout << " ";
      }
      else
      {
        cout << "     ";
      }

      cout << "[" + option->getLongOption () + "]";

      blanksToPrint -= option->getLongOption ().size ();
    }

    for (int i = 0; i < blanksToPrint; ++i)
    {
      cout << " ";
    }

    cout << option->getHelpMessage () + "." << endl;
  }

  exit (0);
}

unsigned int
CommandLine::getNumberOfArguments () const
{
  return ROSEArguments.size ();
}

char **
CommandLine::getArguments () const
{
  using std::string;
  using std::vector;

  /*
   * ======================================================
   * Allocate enough space for all the command-line
   * arguments recognised as ROSE ones
   * ======================================================
   */
  char ** argv = new char*[ROSEArguments.size ()];

  int i = 0;
  for (vector <string>::const_iterator it = ROSEArguments.begin (); it
      != ROSEArguments.end (); ++it)
  {
    /*
     * ======================================================
     * Allocate space for the character array which represents
     * a command-line argument
     * ======================================================
     */
    argv[i] = new char[it->size ()];

    /*
     * ======================================================
     * Have to do a 'strcpy' because 'c_str' returns a pointer
     * to a constant character array and we need a modifiable
     * character array
     * ======================================================
     */
    strcpy (argv[i], (*it).c_str ());

    /*
     * ======================================================
     * Advance the index into argv
     * ======================================================
     */
    ++i;
  }

  return argv;
}

void
CommandLine::parse (int argc, char ** argv)
{
  using std::string;

  CommandLineOptionWithParameters * paramterisedOption = NULL;

  for (int i = 0; i < argc; ++i)
  {
    if (paramterisedOption != NULL)
    {
      /*
       * ======================================================
       * The n-1th option was a parameterised option so grab
       * its parameter
       * ======================================================
       */

      paramterisedOption->setParameter (argv[i]);

      paramterisedOption->run ();

      paramterisedOption = NULL;
    }
    else if (otherArguments.find (argv[i]) == otherArguments.end ())
    {
      /*
       * ======================================================
       * Do not recognise the option so assume it is a ROSE flag
       * ======================================================
       */

      ROSEArguments.push_back (argv[i]);
    }
    else
    {
      /*
       * ======================================================
       * Get the option
       * ======================================================
       */

      CommandLineOption * option = otherArguments[argv[i]];

      paramterisedOption
          = dynamic_cast <CommandLineOptionWithParameters *> (option);

      if (paramterisedOption == NULL)
      {
        option->run ();
      }
    }
  }
}

void
CommandLine::addOption (CommandLineOption * option)
{
  if (option->getShortOption ().empty () == false)
  {
    otherArguments[option->getShortOption ()] = option;
  }
  else
  {
    otherArguments[option->getLongOption ()] = option;
  }
}

void
CommandLine::setToolName (std::string const & toolName)
{
  this->toolName = toolName;
}

CommandLine *
CommandLine::getInstance ()
{
  if (globalsInstance == NULL)
  {
    globalsInstance = new CommandLine ();
  }
  return globalsInstance;
}
