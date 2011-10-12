#pragma once
#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace Exceptions
{
  namespace CommandLine
  {
    class MutuallyExclusiveException: public std::runtime_error
    {
      public:

        MutuallyExclusiveException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class BackendException: public std::runtime_error
    {
      public:

        BackendException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class FrontendException: public std::runtime_error
    {
      public:

        FrontendException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };
  }

  namespace ParallelLoop
  {
    class OpGblWriteException: public std::runtime_error
    {
      public:

        OpGblWriteException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class OpGblReadWriteException: public std::runtime_error
    {
      public:

        OpGblReadWriteException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class UnknownAccessException: public std::runtime_error
    {
      public:

        UnknownAccessException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class UnsupportedBaseTypeException: public std::runtime_error
    {
      public:

        UnsupportedBaseTypeException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };
  }

  namespace CUDA
  {
    class ThreadBlockDimensionException: public std::runtime_error
    {
      public:

        ThreadBlockDimensionException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class GridDimensionException: public std::runtime_error
    {
      public:

        GridDimensionException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class SharedVariableTypeException: public std::runtime_error
    {
      public:

        SharedVariableTypeException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class SharedMemorySizeException: public std::runtime_error
    {
      public:

        SharedMemorySizeException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };
  }

  namespace CodeGeneration
  {
    class FileCreationException: public std::runtime_error
    {
      public:

        FileCreationException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class UnknownVariableException: public std::runtime_error
    {
      public:

        UnknownVariableException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class UnknownSubroutineException: public std::runtime_error
    {
      public:

        UnknownSubroutineException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class UnfoundStatementException: public std::runtime_error
    {
      public:

        UnfoundStatementException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class FortranVariableAttributeException: public std::runtime_error
    {
      public:

        FortranVariableAttributeException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };
  }
}

#endif
