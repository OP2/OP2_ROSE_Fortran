MODULE OP2_CONSTANTS

  real(8) :: gam = 1.4

  real(8) :: gm1 = 1.0

  real(8) :: cfl = 0.9

  real(8) :: eps = 0.05

  real(8) :: mach = 0.4

  real(8) :: alpha

  real(8) :: qinf(4)

CONTAINS

  SUBROUTINE initialise_flow_field()
    IMPLICIT NONE
    
    REAL(kind=8)    :: p,r,u,e
    INTEGER(kind=4) :: n,m,i
  
    CALL op_decl_const (1, gam)
    CALL op_decl_const (1, gm1)
    CALL op_decl_const (1, cfl)
    CALL op_decl_const (1, eps)
    CALL op_decl_const (1, mach)
    CALL op_decl_const (1, alpha)
    CALL op_decl_const (4, qinf)
  
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

  END SUBROUTINE initialise_flow_field
  
end module OP2_CONSTANTS
