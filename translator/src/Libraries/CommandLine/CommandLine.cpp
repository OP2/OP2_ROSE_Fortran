


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


#include <CommandLine.h>
#include <CommandLineOption.h>
#include <CommandLineOptionWithParameters.h>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <Debug.h>
#include <Globals.h>

CommandLine * CommandLine::globalsInstance = NULL;

void
CommandLine::addHelpOption ()
{
  class HelpOption: public CommandLineOption
  {
    public:

      virtual void
      run ()
      {
        CommandLine::getInstance ()->outputOptions ();
      }

      HelpOption (std::string helpMessage, std::string shortOption,
          std::string longOption) :
        CommandLineOption (helpMessage, shortOption, longOption)
      {
      }
  };

  HelpOption * help = new HelpOption ("Display this message", "h", "help");

  addOption (help);
}

void
CommandLine::addDebugOption ()
{
  using boost::lexical_cast;
  using std::string;

  class DebugOption: public CommandLineOptionWithParameters
  {
    public:

      virtual void
      run ()
      {
        Debug::getInstance ()->setDebugLevel (getParameter ());
      }

      DebugOption (std::string helpMessage, std::string parameterName,
          std::string shortOption, std::string longOption) :
        CommandLineOptionWithParameters (helpMessage, parameterName,
            shortOption, longOption)
      {
      }
  };

  string const parameterName = "n";

  /* ASK ADAM */
  string const helpMessage = "Set the debug level. Valid values of "
      + parameterName + " are in the range " + lexical_cast <string> (
      Debug::LOWEST_DEBUG_LEVEL) + ".." + lexical_cast <string> (
      Debug::HIGHEST_DEBUG_LEVEL);
      
      
  DebugOption * debug = new DebugOption (helpMessage, parameterName, "d",
      "debug");

  addOption (debug);
}

CommandLine::CommandLine ()
{
  addHelpOption ();

  addDebugOption ();
}

void
CommandLine::outputOptions ()
{
  using std::map;
  using std::string;
  using std::cout;
  using std::endl;

  int const optionExtraSpaces = 2;
  int const parameterExtraSpaces = 3;
  int maxLengthOfShortOption = 0;
  int maxLengthOfLongOption = 0;

  cout << "Options:";

  for (map <string, CommandLineOption *>::iterator it = otherArguments.begin (); it
      != otherArguments.end (); ++it)
  {
    CommandLineOption * option = it->second;

    CommandLineOptionWithParameters * paramterisedOption =
        dynamic_cast <CommandLineOptionWithParameters *> (option);

    if (option->hasShortOption ())
    {
      int length = option->getShortOption ().size () + optionExtraSpaces;

      cout << " [" + option->getShortOption ();

      if (paramterisedOption != NULL)
      {
        length += paramterisedOption->getParameterName ().size ()
            + parameterExtraSpaces;

        cout << " <" + paramterisedOption->getParameterName () + ">";
      }

      cout << "]";

      if (length > maxLengthOfShortOption)
      {
        maxLengthOfShortOption = length;
      }
    }

    if (option->hasLongOption ())
    {
      int length = option->getLongOption ().size () + optionExtraSpaces;

      cout << " [" + option->getLongOption ();

      if (paramterisedOption != NULL)
      {
        length += paramterisedOption->getParameterName ().size ()
            + parameterExtraSpaces;

        cout << " <" + paramterisedOption->getParameterName () + ">";
      }

      cout << "]";

      if (length > maxLengthOfLongOption)
      {
        maxLengthOfLongOption = length;
      }
    }
  }

  cout << endl << endl;

  /*
   * ======================================================
   * Now detail the options
   * ======================================================
   */

  for (map <string, CommandLineOption *>::iterator it = otherArguments.begin (); it
      != otherArguments.end (); ++it)
  {
    CommandLineOption * option = it->second;

    CommandLineOptionWithParameters * paramterisedOption =
        dynamic_cast <CommandLineOptionWithParameters *> (option);

    int length = 0;

    if (option->hasShortOption ())
    {
      length = option->getShortOption ().size () + optionExtraSpaces;

      cout << "[" + option->getShortOption ();

      if (paramterisedOption != NULL)
      {
        length += paramterisedOption->getParameterName ().size ()
            + parameterExtraSpaces;

        cout << " <" + paramterisedOption->getParameterName () + ">";
      }

      cout << "]";
    }

    for (int i = length; i < maxLengthOfShortOption + 1; ++i)
    {
      cout << " ";
    }

    length = 0;

    if (option->hasLongOption ())
    {
      length = option->getLongOption ().size () + optionExtraSpaces;

      cout << "[" + option->getLongOption ();

      if (paramterisedOption != NULL)
      {
        length += paramterisedOption->getParameterName ().size ()
            + parameterExtraSpaces;

        cout << " <" + paramterisedOption->getParameterName () + ">";
      }

      cout << "]";

      for (int i = length; i < maxLengthOfLongOption + 1; ++i)
      {
        cout << " ";
      }
    }

    cout << option->getHelpMessage () + "." << endl;
  }

  exit (0);
}

unsigned int
CommandLine::getNumberOfRoseArguments () const
{
  return ROSEArguments.size ();
}

void
CommandLine::getRoseArguments (std::vector <std::string> & argv)
{
  argv.assign (ROSEArguments.begin (), ROSEArguments.end ());
}

void
CommandLine::parse (int argc, char ** argv)
{
  using boost::to_lower_copy;
  using boost::filesystem::path;
  using boost::filesystem::system_complete;
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
    else
    {
      string const & arg = to_lower_copy (string (argv[i]));

      if (otherArguments.find (arg) == otherArguments.end ())
      {
        /*
         * ======================================================
         * Do not recognise the option so assume it is a ROSE flag
         * ======================================================
         */

        Debug::getInstance ()->debugMessage (string (argv[i])
            + " is a ROSE option", Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

        path p = system_complete (path (argv[i]));

        ROSEArguments.push_back (argv[i]);

#if BOOST_FILESYSTEM_VERSION == 3
	string filename = p.filename ().string ();
#else
	string filename = p.filename ();
#endif

        Globals::getInstance ()->addInputFile (filename);
      }
      else
      {
        /*
         * ======================================================
         * Get the option
         * ======================================================
         */

        CommandLineOption * option = otherArguments[arg];

        paramterisedOption
            = dynamic_cast <CommandLineOptionWithParameters *> (option);

        if (paramterisedOption == NULL)
        {
          option->run ();
        }
      }
    }
  }
}

void
CommandLine::addOption (CommandLineOption * option)
{
  using boost::to_lower_copy;
  using std::string;

  if (option->getShortOption ().empty () == false)
  {
    string const opt = to_lower_copy (option->getShortOption ());

    otherArguments[opt] = option;
  }
  else
  {
    string const opt = to_lower_copy (option->getLongOption ());

    otherArguments[opt] = option;
  }
}

void
CommandLine::addUDrawGraphOption ()
{
  class UDrawGraphOption: public CommandLineOption
  {
    public:

      virtual void
      run ()
      {
        Globals::getInstance ()->setOutputUDrawGraphs ();
      }

      UDrawGraphOption (std::string helpMessage, std::string shortOption,
          std::string longOption) :
        CommandLineOption (helpMessage, shortOption, longOption)
      {
      }
  };

  UDrawGraphOption * udraw = new UDrawGraphOption ("Generate uDraw graphs",
      "u", "udraw");

  addOption (udraw);
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
