
#pragma once
#ifndef COMMAND_LINE_OPTION_WITH_PARAMETERS_H
#define COMMAND_LINE_OPTION_WITH_PARAMETERS_H

#include <CommandLineOption.h>

class CommandLineOptionWithParameters: public CommandLineOption
{
  private:

    std::string parameterName;

    std::string parameter;

  public:

    std::string const &
    getParameterName () const;

    std::string const &
    getParameter () const;

    void
    setParameter (std::string const & parameter);

    virtual void
    run () = 0;

    CommandLineOptionWithParameters (std::string helpMessage,
        std::string parameterName, std::string shortOption = NULL,
        std::string longOption = NULL);
};

#endif
