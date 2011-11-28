#ifndef TRANSLATOR_COMMAND_LINE_OPTIONS_H
#define TRANSLATOR_COMMAND_LINE_OPTIONS_H

#include "Globals.h"
#include "TargetLanguage.h"
#include "CommandLineOption.h"
#include "CommandLineOptionWithParameters.h"

class FreeVariablesFileOption: public CommandLineOptionWithParameters
{
  public:

    virtual void
    run ()
    {
      Globals::getInstance ()->setFreeVariablesFilename (getParameter ());
    }

    FreeVariablesFileOption (std::string helpMessage) :
      CommandLineOptionWithParameters (helpMessage, "file", "f",
          "free-variables")
    {
    }
};

class OxfordOption: public CommandLineOption
{
  public:

    virtual void
    run ()
    {
      Globals::getInstance ()->setRenderOxfordAPICalls ();
    }

    OxfordOption (std::string helpMessage, std::string longOption) :
      CommandLineOption (helpMessage, "", longOption)
    {
    }
};

class CUDAOption: public CommandLineOption
{
  public:

    virtual void
    run ()
    {
      Globals::getInstance ()->setTargetBackend (TargetLanguage::CUDA);
    }

    CUDAOption (std::string helpMessage, std::string longOption) :
      CommandLineOption (helpMessage, "", longOption)
    {
    }
};

class OpenMPOption: public CommandLineOption
{
  public:

    virtual void
    run ()
    {
      Globals::getInstance ()->setTargetBackend (TargetLanguage::OPENMP);
    }

    OpenMPOption (std::string helpMessage, std::string longOption) :
      CommandLineOption (helpMessage, "", longOption)
    {
    }
};

class OpenCLOption: public CommandLineOption
{
  public:

    virtual void
    run ()
    {
      Globals::getInstance ()->setTargetBackend (TargetLanguage::OPENCL);
    }

    OpenCLOption (std::string helpMessage, std::string longOption) :
      CommandLineOption (helpMessage, "", longOption)
    {
    }
};

#endif
