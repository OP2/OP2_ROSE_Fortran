#include <iostream>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <CommandLineOption.h>
#include <CommandLine.h>

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

std::string const &
CommandLineOption::getHelpMessage () const
{
  return helpMessage;
}

std::string const &
CommandLineOption::getLongOption () const
{
  return longOption;
}

std::string const &
CommandLineOption::getShortOption () const
{
  return shortOption;
}

bool
CommandLineOption::hasShortOption () const
{
  return shortOption.empty () == false;
}

bool
CommandLineOption::hasLongOption () const
{
  return longOption.empty () == false;
}

CommandLineOption::CommandLineOption (std::string helpMessage,
    std::string shortOption, std::string longOption) :
  helpMessage (helpMessage)
{
  using boost::starts_with;
  using boost::lexical_cast;
  using std::cerr;
  using std::string;

  if (shortOption.empty () && longOption.empty ())
  {
    cerr
        << "Error: You have added an option in which both its short and long options are empty.\n";
    exit (1);
  }
  else
  {
    if (shortOption.find (" ") != string::npos)
    {
      cerr << "Error: the short option '" + shortOption
          + "' has a blank space inside it.\n";
      exit (1);
    }

    if (starts_with (shortOption, "-"))
    {
      cerr << "Error: the short option '" + shortOption
          + "' starts with trailing '-' characters.\n";
      exit (1);
    }

    if (longOption.find (" ") != string::npos)
    {
      cerr << "Error: the long option '" + longOption
          + "' has a blank space inside it.\n";
      exit (1);
    }

    if (starts_with (longOption, "-"))
    {
      cerr << "Error: the long option '" + longOption
          + "' starts with trailing '-' characters.\n";
      exit (1);
    }
  }

  if (shortOption.empty () == false)
  {
    this->shortOption = "-" + shortOption;
  }

  if (longOption.empty () == false)
  {
    this->longOption = "--" + longOption;
  }
}
