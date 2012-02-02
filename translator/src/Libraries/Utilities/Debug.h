


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


#pragma once
#ifndef DEBUG_H
#define DEBUG_H

#include <string>

class Debug
{
  private:

    static Debug * debugInstance;

    int debugLevel;

  public:

    /*
     * ======================================================
     * The lowest and highest debug levels. The lowest signifies
     * all debug messages disabled. Increasing levels of severity
     * are indicated by increasing numbers
     * ======================================================
     */
    static int const LOWEST_DEBUG_LEVEL = 0;

    static int const HIGHEST_DEBUG_LEVEL = 5;

    static int const VERBOSE_LEVEL = 1;

    static int const CONSTRUCTOR_LEVEL = 2;

    static int const FUNCTION_LEVEL = 3;

    static int const OUTER_LOOP_LEVEL = 4;

    static int const INNER_LOOP_LEVEL = 4;

  private:

    /*
     * ======================================================
     * Strips the file name from an absolute directory path
     * ======================================================
     */
    std::string
    getFileName (std::string const & filePath);

    /*
     * ======================================================
     * Private constructor ensures users can never create
     * multiple instances
     * ======================================================
     */
    Debug ();

  public:

    /*
     * ======================================================
     * This always returns a single instance of the Debug
     * class to make it compliant with the singleton pattern
     * ======================================================
     */
    static Debug *
    getInstance ();

    /*
     * ======================================================
     * Set the debug level
     * ======================================================
     */
    void
    setDebugLevel (std::string debugLevel);

    /*
     * ======================================================
     * Print this message if the debug level has been
     * set at this level or below, unless the debug level
     * equals 'LOWEST_DEBUG_LEVEL', in which case the message
     * is ignored
     * ======================================================
     */
    void
    debugMessage (std::string const & message, int const debugLevel,
        std::string const & filePath, int const lineNumber) const;
};

#endif
