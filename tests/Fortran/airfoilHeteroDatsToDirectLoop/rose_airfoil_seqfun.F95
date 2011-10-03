MODULE airfoil_seq
  USE constantVars
  CONTAINS
  SUBROUTINE initialise_flow_field(ncell,q,res)
  IMPLICIT NONE
  INTEGER(kind=4) :: ncell
  REAL(kind=8) :: q(:)
  REAL(kind=8) :: res(:)
  REAL(kind=8) :: p, r, u, e
  INTEGER(kind=4) :: n, m
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
      q(4 * n + m - 4) = qinf(m)
      res(4 * n + m - 4) = 0.0
    END DO

  END DO

END SUBROUTINE initialise_flow_field

! save old solution
SUBROUTINE save_soln(q,fakeInt,qold) bind(C)
IMPLICIT NONE
REAL(kind=8) :: q(4)
INTEGER(kind=4) :: fakeInt(10)
REAL(kind=8) :: qold(4)
INTEGER(kind=4) :: i

DO i = 1, 4
  qold(i) = q(i)
END DO

DO i = 1, 10
  fakeInt(i) = 1
END DO

END SUBROUTINE save_soln

SUBROUTINE adt_calc(x1,x2,x3,x4,q,adt) bind(C)
IMPLICIT NONE
REAL(kind=8) :: x1(*)
REAL(kind=8) :: x2(*)
REAL(kind=8) :: x3(*)
REAL(kind=8) :: x4(*)
REAL(kind=8) :: q(*)
REAL(kind=8) :: adt(*)
REAL(kind=8) :: dx, dy, ri, u, v, c
ri = 1.0 / q(1)
u = ri * q(2)
v = ri * q(3)
c = sqrt(gam * gm1 * (ri * q(4) - 0.5 * (u * u + v * v)))
dx = x2(1) - x1(1)
dy = x2(2) - x1(2)
adt(1) = abs(u * dy - v * dx) + c * sqrt(dx * dx + dy * dy)
dx = x3(1) - x2(1)
dy = x3(2) - x2(2)
adt(1) = adt(1) + abs(u * dy - v * dx) + c * sqrt(dx * dx + dy * dy)
dx = x4(1) - x3(1)
dy = x4(2) - x3(2)
adt(1) = adt(1) + abs(u * dy - v * dx) + c * sqrt(dx * dx + dy * dy)
dx = x1(1) - x4(1)
dy = x1(2) - x4(2)
adt(1) = adt(1) + abs(u * dy - v * dx) + c * sqrt(dx * dx + dy * dy)
adt(1) = adt(1) / cfl
END SUBROUTINE adt_calc

SUBROUTINE res_calc(x1,x2,q1,q2,adt1,adt2,res1,res2) bind(C)
IMPLICIT NONE
REAL(kind=8) :: x1(*)
REAL(kind=8) :: x2(*)
REAL(kind=8) :: q1(*)
REAL(kind=8) :: q2(*)
REAL(kind=8) :: adt1(*)
REAL(kind=8) :: adt2(*)
REAL(kind=8) :: res1(*)
REAL(kind=8) :: res2(*)
REAL(kind=8) :: dx, dy, mu, ri, p1, vol1, p2, vol2, f
dx = x1(1) - x2(1)
dy = x1(2) - x2(2)
ri = 1.0 / q1(1)
p1 = gm1 * (q1(4) - 0.5 * ri * (q1(2) * q1(2) + q1(3) * q1(3)))
vol1 = ri * (q1(2) * dy - q1(3) * dx)
ri = 1.0 / q2(1)
p2 = gm1 * (q2(4) - 0.5 * ri * (q2(2) * q2(2) + q2(3) * q2(3)))
vol2 = ri * (q2(2) * dy - q2(3) * dx)
mu = 0.5 * (adt1(1) + adt2(1)) * eps
f = 0.5 * (vol1 * q1(1) + vol2 * q2(1)) + mu * (q1(1) - q2(1))
res1(1) = res1(1) + f
res2(1) = res2(1) - f
f = 0.5 * (vol1 * q1(2) + p1 * dy + vol2 * q2(2) + p2 * dy) + mu * (q1(2) - q2(2))
res1(2) = res1(2) + f
res2(2) = res2(2) - f
f = 0.5 * (vol1 * q1(3) - p1 * dx + vol2 * q2(3) - p2 * dx) + mu * (q1(3) - q2(3))
res1(3) = res1(3) + f
res2(3) = res2(3) - f
f = 0.5 * (vol1 * (q1(4) + p1) + vol2 * (q2(4) + p2)) + mu * (q1(4) - q2(4))
res1(4) = res1(4) + f
res2(4) = res2(4) - f
END SUBROUTINE res_calc

SUBROUTINE bres_calc(x1,x2,q1,adt1,res1,bound) bind(C)
IMPLICIT NONE
REAL(kind=8) :: x1(*)
REAL(kind=8) :: x2(*)
REAL(kind=8) :: q1(*)
REAL(kind=8) :: adt1(*)
REAL(kind=8) :: res1(*)
INTEGER(kind=4) :: bound(*)
REAL(kind=8) :: dx, dy, mu, ri, p1, vol1, p2, vol2, f
dx = x1(1) - x2(1)
dy = x1(2) - x2(2)
ri = 1.0 / q1(1)
p1 = gm1 * (q1(4) - 0.5 * ri * (q1(2) * q1(2) + q1(3) * q1(3)))

IF (bound(1) .EQ. 1) THEN
res1(2) = res1(2) + +(p1 * dy)
res1(3) = res1(3) + -(p1 * dx)
ELSE
vol1 = ri * (q1(2) * dy - q1(3) * dx)
ri = 1.0 / qinf(1)
p2 = gm1 * (qinf(4) - 0.5 * ri * (qinf(2) * qinf(2) + qinf(3) * qinf(3)))
vol2 = ri * (qinf(2) * dy - qinf(3) * dx)
mu = adt1(1) * eps
f = 0.5 * (vol1 * q1(1) + vol2 * qinf(1)) + mu * (q1(1) - qinf(1))
res1(1) = res1(1) + f
f = 0.5 * (vol1 * q1(2) + p1 * dy + vol2 * qinf(2) + p2 * dy) + mu * (q1(2) - qinf(2))
res1(2) = res1(2) + f
f = 0.5 * (vol1 * q1(3) - p1 * dx + vol2 * qinf(3) - p2 * dx) + mu * (q1(3) - qinf(3))
res1(3) = res1(3) + f
f = 0.5 * (vol1 * (q1(4) + p1) + vol2 * (qinf(4) + p2)) + mu * (q1(4) - qinf(4))
res1(4) = res1(4) + f
END IF

END SUBROUTINE bres_calc

SUBROUTINE update(qold,q,res,adt,rms) bind(C)
IMPLICIT NONE
REAL(kind=8) :: qold(*)
REAL(kind=8) :: q(*)
REAL(kind=8) :: res(*)
REAL(kind=8) :: adt(*)
REAL(kind=8) :: rms(*)
REAL(kind=8) :: del, adti
INTEGER(kind=4) :: i
adti = 1.0 / adt(1)

DO i = 1, 4
del = adti * res(i)
q(i) = qold(i) - del
res(i) = 0.0
rms(1) = rms(1) + del * del
END DO

END SUBROUTINE update

END MODULE airfoil_seq

