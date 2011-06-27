/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Debug functionality provided by singleton pattern as we should only ever
 * create a single Debug class
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <string>

class Debug
{
  private:

    static Debug * debugInstance;

    bool verbose;

    int debugLevel;

    /*
     * ======================================================
     * The lowest and highest debug levels. The lowest signifies
     * all debug messages disabled. Increasing levels of severity
     * are indicated by increasing numbers
     * ======================================================
     */
    static int const LOWEST_DEBUG_LEVEL = 0;

    static int const HIGHEST_DEBUG_LEVEL = 10;


  private:

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
     * Set verbose mode on
     * ======================================================
     */
    void
    setVerbose ();

    /*
     * ======================================================
     * Is verbose enabled?
     * ======================================================
     */
    bool
    isVerbose () const;

    /*
     * ======================================================
     * Set the debug level
     * ======================================================
     */
    void
    setDebugLevel (std::string debugLevel);

    /*
     * ======================================================
     * What is the debug level?
     * ======================================================
     */
    int
    getDebugLevel () const;

    /*
     * ======================================================
     * Print this message if verbose mode enabled
     * ======================================================
     */
    void
    verboseMessage (std::string const & message) const;

    /*
     * ======================================================
     * Print this message if the debug level has been
     * set at this level or below, unless the debug level
     * equals 'LOWEST_DEBUG_LEVEL', in which case the message
     * is ignored
     * ======================================================
     */
    void
    debugMessage (std::string const & message, int const debugLevel) const;

    /*
     * ======================================================
     * Print this message then exit
     * ======================================================
     */
    void
    errorMessage (std::string const & message) const;
};

#endif
