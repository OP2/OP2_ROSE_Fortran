#include <boost/algorithm/string.hpp>
#include <rose.h>
#include <Debug.h>
#include <CommandLine.h>
#include <CommandLineOption.h>
#include <CommandLineOptionWithParameters.h>

void
addCommandLineOptions ()
{
}

void
processUserSelections (SgProject * project)
{
  if (project->get_Fortran_only () == true)
  {
    Debug::getInstance ()->debugMessage ("Fortran project detected",
        Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

    project->unparse ();
  }
  else
  {
    Debug::getInstance ()->errorMessage (
        "The translator does not supported the programming language of the given files");
  }
}

int
main (int argc, char ** argv)
{
  addCommandLineOptions ();

  CommandLine::getInstance ()->parse (argc, argv);

  /*
   * ======================================================
   * Pass the pre-processed command-line arguments and NOT
   * 'argc' and 'argv', otherwise ROSE will complain it does
   * not recognise particular options
   * ======================================================
   */

  std::vector <std::string> args;

  CommandLine::getInstance ()->getRoseArguments (args);

  SgProject * project = frontend (args);

  ROSE_ASSERT (project != NULL);

  Debug::getInstance ()->debugMessage ("Translation starting",
      Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

  processUserSelections (project);

  Debug::getInstance ()->debugMessage ("Translation completed",
      Debug::VERBOSE_LEVEL, __FILE__, __LINE__);

  return 0;
}

