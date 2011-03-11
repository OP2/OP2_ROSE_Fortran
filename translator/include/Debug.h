/*
 * Written by Adam Betts
 *
 * Debug functionality provided by singleton pattern as we should only ever
 * create a single Debug class
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <string>

class Debug
{
  private:
    static Debug *debugInstance;
    bool verbose;
    int debugLevel;

    /*
     * Private constructor ensures users can never
     * create multiple instances
     */
    Debug ();

  public:
    /*
     * The lowest and highest debug levels. The lowest signifies all
     * debug messages disabled. Increasing levels of severity are
     * indicated by increasing numbers.
     */
    const static int LOWEST_DEBUG_LEVEL = 0;
    const static int HIGHEST_DEBUG_LEVEL = 10;

    /*
     * This always returns a single instance of the Debug
     * class to make it compliant with the singleton pattern
     */
    static Debug*
    getInstance ();

    /*
     * Set verbose mode on or off
     */
    void
    setVerbose (bool verbose);

    /*
     * Is verbose enabled?
     */
    bool
    isVerbose ();

    /*
     * Set the debug level
     */
    void
    setDebugLevel (int debugLevel);

    /*
     * What is the debug level?
     */
    int
    getDebugLevel ();

    /*
     * Print this message if verbose mode enabled
     */
    void
    verboseMessage (std::string message);

    /*
     * Print this message if the debug level has been
     * set at this level or below, unless the debug level
     * equals 'LOWEST_DEBUG_LEVEL', in which case the message
     * is ignored
     */
    void
    debugMessage (std::string message, int debugLevel);

    /*
     * Print this message then exit
     */
    void
    errorMessage (std::string message);
};

#endif /* DEBUG_H_ */
