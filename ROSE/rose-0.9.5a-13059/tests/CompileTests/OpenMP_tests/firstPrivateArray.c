/*
Array typed firstprivate variables:
 element-by-element copy.

Contributed by Pranav Tendulkar
pranav@ics.forth.gr

4/12/2010
*/
int array[100];
int main()
{
  array[10] = 10;
#pragma omp parallel firstprivate(array)
  {
    int i;
    for(i=0;i<100;i++)
      array[i] += i;
  }
  return 0;
}

