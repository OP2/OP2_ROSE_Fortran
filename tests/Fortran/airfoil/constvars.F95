
module CONSTANTVARS

  real(8) :: gam = 1.4

  real(8) :: gm1 = 1.0

  real(8) :: cfl = 0.9

  real(8) :: eps = 0.05

  real(8) :: mach = 0.4

  real(8) :: alpha

  real(8) :: qinf(4)

contains

SUBROUTINE initialise_flow_field(ncell,q,res)
  IMPLICIT NONE
  INTEGER(kind=4) :: ncell
  REAL(kind=8), DIMENSION(:) :: q
  REAL(kind=8), DIMENSION(:) :: res
  REAL(kind=8) :: p,r,u,e
  INTEGER(kind=4) :: n,m,i
  gam = 1.4
  gm1 = gam - 1.0
  cfl = 0.9
  eps = 0.05
  mach = 0.4
  alpha = 3.0 * atan(1.0) / 45.0
  p = 1.0
  r = 1.0
  u = sqrt(gam * p / r) * mach
  e = p / (r * gm1) + 0.5 * u * u
  qinf(1) = r
  qinf(2) = r * u
  qinf(3) = 0.0
  qinf(4) = r * e

  DO n = 1, ncell

    DO m = 1, 4
      i = 4 * n + m - 4
      q(i) = qinf(m)
      res(i) = 0.0
    END DO

  END DO

END SUBROUTINE initialise_flow_field



end module CONSTANTVARS
