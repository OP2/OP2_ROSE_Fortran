


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


/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Allows extra options to be supplied to our tool
 */

#pragma once
#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <vector>
#include <string>
#include <map>

class CommandLineOption;
class CommandLineOptionWithParameters;

class CommandLine
{
  private:

    static CommandLine * globalsInstance;

    /*
     * ======================================================
     * The arguments passed to the ROSE front end (these are
     * the remaining arguments after our additional arguments
     * have been stripped from the command line)
     * ======================================================
     */
    std::vector <std::string> ROSEArguments;

    std::map <std::string, CommandLineOption *> otherArguments;

  private:

    /*
     * ======================================================
     * Adds a help option to the command line
     * ======================================================
     */
    void
    addHelpOption ();

    /*
     * ======================================================
     * Adds a debug option to the command line
     * ======================================================
     */
    void
    addDebugOption ();

    /*
     * ======================================================
     * Private constructor ensures users can never create
     * multiple instances
     * ======================================================
     */
    CommandLine ();

  public:

    /*
     * ======================================================
     * Outputs all the options
     * ======================================================
     */
    void
    outputOptions ();

    /*
     * ======================================================
     * Effectively returns 'argc' after we have stripped
     * out the extra command-line options our tool accepts
     * ======================================================
     */
    unsigned int
    getNumberOfRoseArguments () const;

    /*
     * ======================================================
     * Effectively puts 'argv' into the supplied vector after
     * we have stripped out the extra command-line options
     * our tool accepts
     * ======================================================
     */
    void
    getRoseArguments (std::vector<std::string> & argv);

    /*
     * ======================================================
     * Parse the command line presented by the user
     * ======================================================
     */
    void
    parse (int argc, char ** argv);

    /*
     * ======================================================
     * Add an option to the command line
     * ======================================================
     */
    void
    addOption (CommandLineOption * option);

    /*
     * ======================================================
     * Adds an option to the command line to generate uDraw
     * graphs of the ASTs
     * ======================================================
     */
    void
    addUDrawGraphOption ();

    /*
     * ======================================================
     * This always returns a single instance of the CommandLine
     * class to make it compliant with the singleton pattern
     * ======================================================
     */
    static CommandLine *
    getInstance ();
};

#endif
