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

        static unsigned int const returnValue = 1;

      public:

        MutuallyExclusiveException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class LanguageException: public std::runtime_error
    {
      public:

        static unsigned int const returnValue = 2;

      public:

        LanguageException (const std::string& msg) :
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

        static unsigned int const returnValue = 4;

      public:

        OpGblWriteException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class OpGblReadWriteException: public std::runtime_error
    {
      public:

        static unsigned int const returnValue = 5;

      public:

        OpGblReadWriteException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class UnknownAccessException: public std::runtime_error
    {
      public:

        static unsigned int const returnValue = 6;

      public:

        UnknownAccessException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class UnsupportedBaseTypeException: public std::runtime_error
    {
      public:

        static unsigned int const returnValue = 7;

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

        static unsigned int const returnValue = 8;

      public:

        ThreadBlockDimensionException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class GridDimensionException: public std::runtime_error
    {
      public:

        static unsigned int const returnValue = 9;

      public:

        GridDimensionException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class SharedVariableTypeException: public std::runtime_error
    {
      public:

        static unsigned int const returnValue = 10;

      public:

        SharedVariableTypeException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class SharedMemorySizeException: public std::runtime_error
    {
      public:

        static unsigned int const returnValue = 11;

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

        static unsigned int const returnValue = 12;

      public:

        FileCreationException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class UnknownVariableException: public std::runtime_error
    {
      public:

        static unsigned int const returnValue = 13;

      public:

        UnknownVariableException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class DuplicateVariableException: public std::runtime_error
    {
      public:

        static unsigned int const returnValue = 14;

      public:

        DuplicateVariableException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class UnknownSubroutineException: public std::runtime_error
    {
      public:

        static unsigned int const returnValue = 15;

      public:

        UnknownSubroutineException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class UnfoundStatementException: public std::runtime_error
    {
      public:

        static unsigned int const returnValue = 16;

      public:

        UnfoundStatementException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };

    class FortranVariableAttributeException: public std::runtime_error
    {
      public:

        static unsigned int const returnValue = 17;

      public:

        FortranVariableAttributeException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };
  }
}

#endif
