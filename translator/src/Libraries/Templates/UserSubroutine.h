


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
#ifndef USER_SUBROUTINE_H
#define USER_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>
#include <vector>

using namespace std;

template <typename TSubroutineHeader, typename TDeclarations>
  class UserSubroutine: public Subroutine <TSubroutineHeader>
  {
    
    protected:

      TDeclarations * declarations;

      ParallelLoop * parallelLoop;

      TSubroutineHeader * originalSubroutine;

      /*
       * ======================================================
       * List of functions called in the user subroutine
       * to be appended to the generated files
       * ======================================================
       */
  
      vector < TSubroutineHeader * > calledRoutines;
      
    protected:

      UserSubroutine (ParallelLoop * parallelLoop, TDeclarations * declarations) :
        Subroutine <TSubroutineHeader> (parallelLoop->getUserSubroutineName ()
            + "_modified"), declarations (declarations), parallelLoop (
            parallelLoop)
      {
      }

      /*
       * ======================================================
       * In this case we do not modify the name
       * + "_modified")
       * ======================================================
       */

      UserSubroutine (ParallelLoop * parallelLoop, TDeclarations * declarations, string subroutineName) :
        Subroutine <TSubroutineHeader> (subroutineName)
            , declarations (declarations), parallelLoop (
            parallelLoop)
      {
      }

  };

#endif
