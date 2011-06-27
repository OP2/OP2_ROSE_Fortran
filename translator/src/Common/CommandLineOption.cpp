#include <iostream>
#include <cstdlib>
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
  using std::cout;

  if (shortOption.empty () && longOption.empty ())
  {
    cout << "Both short and long options cannot be empty\n";
    exit (1);
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
