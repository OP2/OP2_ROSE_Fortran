#pragma once
#ifndef REDUCTION_SUBROUTINES_H
#define REDUCTION_SUBROUTINES_H

#include <Reduction.h>
#include <Exceptions.h>
#include <rose.h>

template <typename TSubroutineHeader>
  class ReductionSubroutines
  {
    private:

      /*
       * ======================================================
       * Mapping from the hash key generated for a reduction
       * triplet (type x size) to its subroutine
       * ======================================================
       */
      std::map <unsigned int, TSubroutineHeader *> subroutines;

    public:

      TSubroutineHeader *
      getHeader (Reduction * reduction)
      {
        using std::find;
        using std::map;

        typename map <unsigned int, TSubroutineHeader *>::iterator it;

        for (it = subroutines.begin (); it != subroutines.end (); ++it)
        {
          if (reduction->hashKey () == it->first)
          {
            return it->second;
          }
        }

        throw Exceptions::CodeGeneration::UnknownSubroutineException (
            "Unable to find reduction subroutine for reduction");
      }

      void
      addSubroutine (Reduction * reduction, TSubroutineHeader * header)
      {
        subroutines[reduction->hashKey ()] = header;
      }

      ReductionSubroutines ()
      {
      }
  };

#endif
