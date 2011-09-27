SUBROUTINE getSetSizes(nnode, ncell, nedge, nbedge)

IMPLICIT NONE

INTEGER(kind=4), PARAMETER :: MAX_PWD_LEN = 255
INTEGER(kind=4), INTENT(OUT) :: nnode,ncell,nedge,nbedge
INTEGER(kind=4), PARAMETER :: FILE_ID = 10
character(len=MAX_PWD_LEN) :: currDir
CALL get_environment_variable("WORK",  currDir)

currDir = trim(currDir) // '/airfoil-seq-c/new_grid.dat'
OPEN (UNIT=FILE_ID, FILE=currDir) 
READ (UNIT=FILE_ID, FMT="(1x,I6,1x,I6,1x,I7,1x,I4)") nnode,ncell,nedge,nbedge
END SUBROUTINE getSetSizes

SUBROUTINE getSetInfo(nnode, ncell, nedge, nbedge, cell, edge, ecell, bedge, becell, bound, x, q, qold, res, adt)

IMPLICIT NONE

INTEGER(kind=4), INTENT(IN) :: nnode,ncell,nedge,nbedge
INTEGER(kind=4), DIMENSION(4 * ncell) :: cell
INTEGER(kind=4), DIMENSION(2 * nedge) :: edge
INTEGER(kind=4), DIMENSION(2 * nedge) :: ecell
INTEGER(kind=4), DIMENSION(2 * nbedge) :: bedge
INTEGER(kind=4), DIMENSION(nbedge) :: becell
INTEGER(kind=4), DIMENSION(nbedge) :: bound
REAL(kind=8), DIMENSION(2 * nnode) :: x
REAL(kind=8), DIMENSION(4 * ncell) :: q
REAL(kind=8), DIMENSION(4 * ncell) :: qold
REAL(kind=8), DIMENSION(4 * ncell) :: res
REAL(kind=8), DIMENSION(ncell) :: adt
INTEGER(kind=4), PARAMETER :: FILE_ID = 10
INTEGER(kind=4) :: i,f_array

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

