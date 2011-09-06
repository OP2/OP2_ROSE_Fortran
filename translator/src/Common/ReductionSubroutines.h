/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef REDUCTION_SUBROUTINES_H
#define REDUCTION_SUBROUTINES_H

#include <rose.h>
#include <Reduction.h>
#include <Debug.h>

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

        Debug::getInstance ()->errorMessage (
            "Unable to find reduction subroutine for reduction", __FILE__, __LINE__);
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
