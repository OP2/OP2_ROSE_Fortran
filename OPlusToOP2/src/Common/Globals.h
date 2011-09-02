/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Class storing global variables set by the user on the command line.
 * Implemented by singleton pattern as we should only ever
 * create a single instance
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <algorithm>
#include <vector>
#include <string>

class Globals
{
  private:

    static Globals * globalsInstance;

    std::vector <std::string> inputFiles;

  private:

    /*
     * ======================================================
     * Private constructor ensures users can never create
     * multiple instances
     * ======================================================
     */
    Globals ();

  public:

    /*
     * ======================================================
     * This always returns a single instance of the Globals
     * class to make it compliant with the singleton pattern
     * ======================================================
     */
    static Globals *
    getInstance ();

    void
    addInputFile (std::string const & fileName);

    bool
    isInputFile (std::string const & fileName) const;
};

#endif
