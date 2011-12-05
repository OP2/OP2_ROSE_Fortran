
void
save_soln(float *q, float *qold);

void 
res_calc(float *x1,  float *x2,  float *q1,  float *q2,
                     float *adt1,float *adt2,float *res1,float *res2);

void 
bres_calc(float *x1,  float *x2,  float *q1,
                      float *adt1,float *res1,int *bound);

void
adt_calc(float *x1,float *x2,float *x3,float *x4,float *q,float *adt);

void
update(float *qold, float *q, float *res, float *adt, float *rms);

void 
fusedOne(float *q, float *qold, float *x1,float *x2,float *x3,float *x4,float *adt);

void
fusedTwo(float *qold, float *q, float *res, float *adt, float *rms, float *x1,float *x2,float *x3,float *x4);

