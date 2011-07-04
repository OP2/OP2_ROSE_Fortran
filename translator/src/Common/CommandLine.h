/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Allows extra options to be supplied to our tool
 */

#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <vector>
#include <string>
#include <map>
#include <CommandLineOption.h>
#include <CommandLineOptionWithParameters.h>

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
    getNumberOfArguments () const;

    /*
     * ======================================================
     * Effectively returns 'argv' after we have stripped
     * out the extra command-line options our tool accepts
     * ======================================================
     */
    char **
    getArguments () const;

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
