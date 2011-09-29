#include <sys/types.h>
#include <err.h>
#include <unistd.h>

char *TCID = "syscall.224";
int TST_TOTAL = 1;


int main() {
  int result = syscall(224);
  
  if( result == -1 )
    err(1,"gettid syscall(224) failed");
  return 0;
}
