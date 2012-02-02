


/*  Open source copyright declaration based on BSD open source template:
 *  http://www.opensource.org/licenses/bsd-license.php
 * 
 * Copyright (c) 2011-2012, Adam Betts, Carlo Bertolli
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <CommandLineOption.h>
#include <iostream>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

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
    int blank = shortOption.find (" ");

    if (blank != string::npos)
    {
      cerr << "Error: the short option '" + shortOption
          + "' has a blank space at position " + lexical_cast <string> (blank)
          + ".\n";

      exit (1);
    }

    if (starts_with (shortOption, "-"))
    {
      cerr << "Error: the short option '" + shortOption
          + "' starts with leading '-' characters.\n";

      exit (1);
    }

    blank = longOption.find (" ");

    if (blank != string::npos)
    {
      cerr << "Error: the long option '" + longOption
          + "' has a blank space at position " + lexical_cast <string> (blank)
          + ".\n";

      exit (1);
    }

    if (starts_with (longOption, "-"))
    {
      cerr << "Error: the long option '" + longOption
          + "' starts with leading '-' characters.\n";

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
