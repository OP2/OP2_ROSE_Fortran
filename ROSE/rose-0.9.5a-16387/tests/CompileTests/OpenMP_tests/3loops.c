/* Test if the loop index variables are treated as private variables 
 * */
#include <stdio.h>
#if defined (_OPENMP)
#include <omp.h>
#endif

int main(void)
{
  int i,jj,kkk;
  double a[10][9][8];

#pragma omp parallel for
  for(i=0;i<10;i++){
    for(jj=0;jj<9;jj++){
      for (kkk=0;kkk<8;kkk++){
        a[i][jj][kkk]=9.9;
        //      printf("a[%d][%d][%d]=%f ",i,jj,kkk,a[i][jj][kkk]);
      } 
    }
  }
 return 0;
}
