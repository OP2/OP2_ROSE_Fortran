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
