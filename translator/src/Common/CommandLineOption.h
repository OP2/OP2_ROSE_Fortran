
#pragma once
#ifndef COMMAND_LINE_OPTION_H
#define COMMAND_LINE_OPTION_H

#include <string>

class CommandLineOption
{
  private:

    std::string helpMessage;

    std::string shortOption;

    std::string longOption;

  public:

    std::string const &
    getHelpMessage () const;

    std::string const &
    getShortOption () const;

    std::string const &
    getLongOption () const;

    bool
    hasShortOption () const;

    bool
    hasLongOption () const;

    virtual void
    run () = 0;

    CommandLineOption (std::string helpMessage, std::string shortOption = NULL,
        std::string longOption = NULL);
};

#endif
