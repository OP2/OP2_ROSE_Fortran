/* Source file to generate input for demo3 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Define only if you need to check the solution reported by demo3 */
#undef CHECK_SOLUTION

int right_shift(int x, int nbits)
{
    return x >> nbits;
}

/* Function to analyze.  This function is not normally called. */
int payload(int seed)
{
    seed += (seed << 3);
    seed ^= right_shift(seed, 11);
    seed += (seed << 15);
    return seed;
}


int main()
{
#ifdef CHECK_SOLUTION
    int n = 123456;
    int p = payload(n);
    printf("f(%d)=%d\n", n, p);
#endif

    return 0;
}

