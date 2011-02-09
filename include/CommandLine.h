/*
 * Written by Adam Betts
 *
 * Allows debug and verbose options to be passed on ROSE command line.
 * Could be extended to include others
 */

#ifndef COMMANDLINE_H_
#define COMMANDLINE_H_

#include <vector>
#include <string>

class CommandLine
{
  private:
    std::vector<std::string> ROSE_arguments;

  public:
    CommandLine(int argc, char **argv);

    /*
     * Effectively returns 'argc' after we have stripped
     * out the extra command-line options our tool accepts
     */
    int
    getNumberOfArguments();

    /*
     * Effectively returns 'argv' after we have stripped
     * out the extra command-line options our tool accepts
     */
    char**
    getArguments();
};


#endif /* COMMANDLINE_H_ */
