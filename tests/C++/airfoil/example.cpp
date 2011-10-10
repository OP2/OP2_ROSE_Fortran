#include <math.h>

int
foo (int x)
{
  int i = 1;
  int j = sqrt(i);
  return i + x * 10;
}

int
main ()
{
  int i = foo(10);

  int j = i * 100;

  if (j > i)
  { 
     int k = 10;
  }

  return 1;
}
