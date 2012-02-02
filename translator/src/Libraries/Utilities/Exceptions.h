


/*  Open source copyright declaration based on BSD open source template:
 *  http://www.opensource.org/licenses/bsd-license.php
 * 
 * Copyright (c) 2011-2012, Adam Betts, Carlo Bertolli
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


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

  namespace ASTParsing
  {
    class NoSourceFileException: public std::runtime_error
    {
      public:

        static unsigned int const returnValue = 18;

      public:

        NoSourceFileException (const std::string& msg) :
          std::runtime_error (msg)
        {
        }
    };
  }
}

#endif
