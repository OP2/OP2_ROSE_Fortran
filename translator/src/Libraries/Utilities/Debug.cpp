


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


#include "Debug.h"
#include <iostream>
#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

Debug * Debug::debugInstance = NULL;

int const Debug::LOWEST_DEBUG_LEVEL;
int const Debug::HIGHEST_DEBUG_LEVEL;
int const Debug::VERBOSE_LEVEL;
int const Debug::CONSTRUCTOR_LEVEL;
int const Debug::FUNCTION_LEVEL;
int const Debug::OUTER_LOOP_LEVEL;
int const Debug::INNER_LOOP_LEVEL;

std::string
Debug::getFileName (std::string const & filePath)
{
  return filePath.substr (filePath.find_last_of ('/') + 1);
}

Debug::Debug ()
{
  debugLevel = 0;
}

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
