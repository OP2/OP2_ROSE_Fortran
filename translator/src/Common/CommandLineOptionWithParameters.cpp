#include <CommandLineOptionWithParameters.h>

std::string const &
CommandLineOptionWithParameters::getParameterName () const
{
  return parameterName;
}

std::string const &
CommandLineOptionWithParameters::getParameter () const
{
  return parameter;
}

void
CommandLineOptionWithParameters::setParameter (std::string const & parameter)
{
  this->parameter = parameter;
}

CommandLineOptionWithParameters::CommandLineOptionWithParameters (
    std::string helpMessage, std::string parameterName,
    std::string shortOption, std::string longOption) :
  CommandLineOption (helpMessage, shortOption, longOption), parameterName (
      parameterName)
{
}
