#include <sys/types.h>
#include <err.h>

char *TCID = "syscall.201";
int TST_TOTAL = 1;


int main() {
  int result = syscall(201);

  if( result == -1 )
    err(1,"geteuid32 syscall(201) failed");
  return 0;
}
