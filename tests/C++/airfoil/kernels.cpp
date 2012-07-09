#include <math.h>

#include "real.h"

extern REAL *eps;
extern REAL *alpha;
extern REAL *cfl;
extern REAL *gam;
extern REAL *gm1;
extern REAL *qinf;

void
save_soln (REAL *q, REAL *qold)
{
  for (int n = 0; n < 4; n++)
    qold[n] = q[n];
}

void
res_calc (REAL *x1, REAL *x2, REAL *q1, REAL *q2, REAL *adt1, REAL *adt2,
    REAL *res1, REAL *res2)
{
  REAL dx, dy, mu, ri, p1, vol1, p2, vol2, f;

  dx = x1[0] - x2[0];
  dy = x1[1] - x2[1];

  ri = 1.0f / q1[0];
  p1 = (*gm1) * (q1[3] - 0.5f * ri * (q1[1] * q1[1] + q1[2] * q1[2]));
  vol1 = ri * (q1[1] * dy - q1[2] * dx);

  ri = 1.0f / q2[0];
  p2 = (*gm1) * (q2[3] - 0.5f * ri * (q2[1] * q2[1] + q2[2] * q2[2]));
  vol2 = ri * (q2[1] * dy - q2[2] * dx);

  mu = 0.5f * ((*adt1) + (*adt2)) * (*eps);

  f = 0.5f * (vol1 * q1[0] + vol2 * q2[0]) + mu * (q1[0] - q2[0]);
  res1[0] += f;
  res2[0] -= f;
  f = 0.5f * (vol1 * q1[1] + p1 * dy + vol2 * q2[1] + p2 * dy) + mu * (q1[1]
      - q2[1]);
  res1[1] += f;
  res2[1] -= f;
  f = 0.5f * (vol1 * q1[2] - p1 * dx + vol2 * q2[2] - p2 * dx) + mu * (q1[2]
      - q2[2]);
  res1[2] += f;
  res2[2] -= f;
  f = 0.5f * (vol1 * (q1[3] + p1) + vol2 * (q2[3] + p2)) + mu * (q1[3] - q2[3]);
  res1[3] += f;
  res2[3] -= f;
}

void
bres_calc (REAL *x1, REAL *x2, REAL *q1, REAL *adt1, REAL *res1,
    int *bound)
{
  REAL dx, dy, mu, ri, p1, vol1, p2, vol2, f;

  dx = x1[0] - x2[0];
  dy = x1[1] - x2[1];

  ri = 1.0f / q1[0];
  p1 = (*gm1) * (q1[3] - 0.5f * ri * (q1[1] * q1[1] + q1[2] * q1[2]));

  if (*bound == 1)
  {
    res1[1] += +p1 * dy;
    res1[2] += -p1 * dx;
  }
  else
  {
    vol1 = ri * (q1[1] * dy - q1[2] * dx);

    ri = 1.0f / qinf[0];
    p2 = (*gm1) * (qinf[3] - 0.5f * ri * (qinf[1] * qinf[1] + qinf[2] * qinf[2]));
    vol2 = ri * (qinf[1] * dy - qinf[2] * dx);

    mu = (*adt1) * (*eps);

    f = 0.5f * (vol1 * q1[0] + vol2 * qinf[0]) + mu * (q1[0] - qinf[0]);
    res1[0] += f;
    f = 0.5f * (vol1 * q1[1] + p1 * dy + vol2 * qinf[1] + p2 * dy) + mu
        * (q1[1] - qinf[1]);
    res1[1] += f;
    f = 0.5f * (vol1 * q1[2] - p1 * dx + vol2 * qinf[2] - p2 * dx) + mu
        * (q1[2] - qinf[2]);
    res1[2] += f;
    f = 0.5f * (vol1 * (q1[3] + p1) + vol2 * (qinf[3] + p2)) + mu * (q1[3]
        - qinf[3]);
    res1[3] += f;
  }
}

void
adt_calc (REAL *x1, REAL *x2, REAL *x3, REAL *x4, REAL *q, REAL *adt)
{
  REAL dx, dy, ri, u, v, c;

  ri = 1.0f / q[0];
  u = ri * q[1];
  v = ri * q[2];
  c = sqrt ((*gam) * (*gm1) * (ri * q[3] - 0.5f * (u * u + v * v)));

  dx = x2[0] - x1[0];
  dy = x2[1] - x1[1];
  *adt = fabs (u * dy - v * dx) + c * sqrt (dx * dx + dy * dy);

  dx = x3[0] - x2[0];
  dy = x3[1] - x2[1];
  *adt += fabs (u * dy - v * dx) + c * sqrt (dx * dx + dy * dy);

  dx = x4[0] - x3[0];
  dy = x4[1] - x3[1];
  *adt += fabs (u * dy - v * dx) + c * sqrt (dx * dx + dy * dy);

  dx = x1[0] - x4[0];
  dy = x1[1] - x4[1];
  *adt += fabs (u * dy - v * dx) + c * sqrt (dx * dx + dy * dy);

  *adt = (*adt) / (*cfl);
}

void
update (REAL *qold, REAL *q, REAL *res, REAL *adt, REAL *rms)
{
  REAL del, adti;

  adti = 1.0f / (*adt);

  for (int n = 0; n < 4; n++)
  {
    del = adti * res[n];
    q[n] = qold[n] - del;
    res[n] = 0.0f;
    *rms += del * del;
  }
}

void
fusedOne (REAL *q, REAL *qold, REAL *x1, REAL *x2, REAL *x3, REAL *x4,
    REAL *adt)
{
  for (int n = 0; n < 4; n++)
  {
    qold[n] = q[n];
  }

  REAL dx, dy, ri, u, v, c;

  ri = 1.0f / q[0];
  u = ri * q[1];
  v = ri * q[2];
  c = sqrt ((*gam) * (*gm1) * (ri * q[3] - 0.5f * (u * u + v * v)));

  dx = x2[0] - x1[0];
  dy = x2[1] - x1[1];
  *adt = fabs (u * dy - v * dx) + c * sqrt (dx * dx + dy * dy);

  dx = x3[0] - x2[0];
  dy = x3[1] - x2[1];
  *adt += fabs (u * dy - v * dx) + c * sqrt (dx * dx + dy * dy);

  dx = x4[0] - x3[0];
  dy = x4[1] - x3[1];
  *adt += fabs (u * dy - v * dx) + c * sqrt (dx * dx + dy * dy);

  dx = x1[0] - x4[0];
  dy = x1[1] - x4[1];
  *adt += fabs (u * dy - v * dx) + c * sqrt (dx * dx + dy * dy);

  *adt = (*adt) / (*cfl);
}

void
fusedTwo (REAL *qold, REAL *q, REAL *res, REAL *adt, REAL *rms, REAL *x1,
    REAL *x2, REAL *x3, REAL *x4)
{
  REAL del, adti;
  REAL dx, dy, ri, u, v, c;

  adti = 1.0f / (*adt);

  for (int n = 0; n < 4; n++)
  {
    del = adti * res[n];
    q[n] = qold[n] - del;
    res[n] = 0.0f;
    *rms += del * del;
  }

  ri = 1.0f / q[0];
  u = ri * q[1];
  v = ri * q[2];
  c = sqrt ((*gam) * (*gm1) * (ri * q[3] - 0.5f * (u * u + v * v)));

  dx = x2[0] - x1[0];
  dy = x2[1] - x1[1];
  *adt = fabs (u * dy - v * dx) + c * sqrt (dx * dx + dy * dy);

  dx = x3[0] - x2[0];
  dy = x3[1] - x2[1];
  *adt += fabs (u * dy - v * dx) + c * sqrt (dx * dx + dy * dy);

  dx = x4[0] - x3[0];
  dy = x4[1] - x3[1];
  *adt += fabs (u * dy - v * dx) + c * sqrt (dx * dx + dy * dy);

  dx = x1[0] - x4[0];
  dy = x1[1] - x4[1];
  *adt += fabs (u * dy - v * dx) + c * sqrt (dx * dx + dy * dy);

  *adt = (*adt) / (*cfl);
}

