SUBROUTINE getSetSizes(nnode,ncell,nedge,nbedge)
IMPLICIT NONE
INTEGER(kind=4), PARAMETER :: MAX_PWD_LEN = 255
INTEGER(kind=4), INTENT(OUT) :: nnode, ncell, nedge, nbedge
INTEGER(kind=4), PARAMETER :: FILE_ID = 10
CHARACTER :: currDir*(MAX_PWD_LEN)
CALL get_environment_variable("WORK",currDir)
currDir = trim(currDir) // '/airfoil-seq-c/new_grid.dat'
OPEN (UNIT=FILE_ID, FILE=currDir)
READ (UNIT=FILE_ID, FMT="(1x,I6,1x,I6,1x,I7,1x,I4)") nnode,ncell,nedge,nbedge
END SUBROUTINE getSetSizes

SUBROUTINE getSetInfo(nnode,ncell,nedge,nbedge,cell,edge,ecell,bedge,becell,bound,x,q,qold,res,adt)
IMPLICIT NONE
INTEGER(kind=4), INTENT(IN) :: nnode, ncell, nedge, nbedge
INTEGER(kind=4) :: cell(4 * ncell)
INTEGER(kind=4) :: edge(2 * nedge)
INTEGER(kind=4) :: ecell(2 * nedge)
INTEGER(kind=4) :: bedge(2 * nbedge)
INTEGER(kind=4) :: becell(nbedge)
INTEGER(kind=4) :: bound(nbedge)
REAL(kind=8) :: x(2 * nnode)
REAL(kind=8) :: q(4 * ncell)
REAL(kind=8) :: qold(4 * ncell)
REAL(kind=8) :: res(4 * ncell)
REAL(kind=8) :: adt(ncell)
INTEGER(kind=4), PARAMETER :: FILE_ID = 10
INTEGER(kind=4) :: i, f_array

DO i = 1, nnode
READ (UNIT=FILE_ID, FMT=*) x(2 * i - 1),x(2 * i)
END DO

DO i = 1, ncell
READ (UNIT=FILE_ID, FMT=*) cell(4 * i - 3),cell(4 * i + 1 - 3),cell(4 * i + 2 - 3),cell(4 * i + 3 - 3)
END DO

DO i = 1, nedge
READ (UNIT=FILE_ID, FMT=*) edge(2 * i - 1),edge(2 * i + 1 - 1),ecell(2 * i - 1),ecell(2 * i + 1 - 1)
END DO

DO i = 1, nbedge
READ (UNIT=FILE_ID, FMT=*) bedge(2 * i - 1),bedge(2 * i + 1 - 1),becell(i),bound(i)
END DO

CLOSE (UNIT=FILE_ID)
END SUBROUTINE getSetInfo

