struct point
{
	int index;
	int part;
	vector<int> watcher[2];

	point()
	{
		index = 0;
		part = 0;
	}
	
	point(const point& ref)
	{
		index = ref.index;
		part = ref.part;
	}
};

void input(int maxnode,int maxcell,int maxedge, int &nnode, int &ncell, int &nedge,
           float *x, float *q, int *cell, int *edge, int *ecell, int *boun) {

  float p,r,u,e, xt,yt;

// set global constants

  gam = 1.4f;
  gm1 = gam - 1.0f;
  cfl = 0.9f;
  eps = 0.05f;

// read in data from grid file

  FILE *fp;
  fp = fopen("grid.dat","r");

  fscanf(fp,"%d %d %d \n",&nnode, &ncell, &nedge);
  ncell = ncell+1;   // far-field dummy cell

  // printf("maxnode = %d, maxcell = %d, maxedge = %d \n",maxnode,maxcell,maxedge);
  // printf("nnode   = %d, ncell   = %d, nedge   = %d \n",nnode,ncell,nedge);

  if(nnode>maxnode) {printf("error:maxnode too small\n"); exit(1);}
  if(ncell>maxcell) {printf("error:maxcell too small\n"); exit(1);}
  if(nedge>maxedge) {printf("error:maxedge too small\n"); exit(1);}

  for (int n=0; n<nnode; n++)
    fscanf(fp,"%f %f \n",&x[2*n], &x[2*n+1]);

  for (int n=0; n<ncell-1; n++) {
    fscanf(fp,"%d %d %d %d \n",&cell[4*n], &cell[4*n+1], &cell[4*n+2], &cell[4*n+3]);
    for(int m=0; m<4; m++) cell[4*n+m]--;             // FORTRAN->C offset
  }
  for (int n=0; n<4; n++) cell[4*(ncell-1)+n] = 0;    // far-field dummy cell

  for (int n=0; n<nedge; n++) {
    fscanf(fp,"%d %d %d %d %d \n",&edge[2*n],&edge[2*n+1],&ecell[2*n],&ecell[2*n+1],&boun[n]);
    for(int m=0; m<2; m++) edge[2*n+m]--;             // FORTRAN->C offset
    for(int m=0; m<2; m++) ecell[2*n+m]--;            // FORTRAN->C offset

    if (boun[n]==1) {
      ecell[2*n] = ecell[2*n+1];  // set interior dummy cell to point to one outside
    }
    else if (boun[n]==2) {
      ecell[2*n+1] = ncell-1;     // set exterior dummy cell to point to far-field cell
      boun[n] = 0;                // then treat like a regular edge
    }
  }
	/*	
	for(int n=0; n<nedge/2; n++)
	{
		for(int m=0; m<2; m++)
		{
			int temp = edge[2*n+m];
			edge[2*n+m] = edge[2*((nedge-1)-n)+m];
			edge[2*((nedge-1)-n)+m] = temp;

			temp = ecell[2*n+m];
			ecell[2*n+m] = ecell[2*((nedge-1)-n)+m];
			ecell[2*((nedge-1)-n)+m] = temp;

			temp = boun[n];
			boun[n] = boun[(nedge-1)-n)];
			boun[((nedge-1)-n)] = temp;
		}
	}
	*/
  fclose(fp);

// read in data from flow file, initialising if necessary

  fp = fopen("flow.dat","r");
  fscanf(fp,"%f %f %f %f \n",&p,&r,&mach,&alpha);
  alpha = alpha*atan(1.0f)/45.0f;
  p = 1.0f;
  r = 1.0f;
  u = sqrt(gam*p/r)*mach;
  e = p/(r*gm1) + 0.5f*u*u;

  for (int n=0; n<ncell; n++) {
    q[4*n  ] = r;
    q[4*n+1] = r*u;
    q[4*n+2] = 0.0f;
    q[4*n+3] = r*e;
  }

  fclose(fp);

// rotate grid to specified angle of attack

  for (int n=0; n<nnode; n++) {
    xt = x[2*n  ];
    yt = x[2*n+1];
    x[2*n  ] = cos(alpha)*xt + sin(alpha)*yt;
    x[2*n+1] = cos(alpha)*yt - sin(alpha)*xt;
  }
}


void intput_partition_info(const char* number, int nnode, int ncell, int& nparts, point *partnode, point *partcell)
{
	FILE *fp;
	char file1[100];
	strcpy(file1, "./dparts/metis.mesh.npart.");
	strcat(file1, number);

	char file2[100];
	strcpy(file2, "./dparts/metis.mesh.epart.");
	strcat(file2, number);

  fp = fopen(file1,"r");
	for(int i=0; i<nnode; i++)
	{
		fscanf(fp, "%d\n", &partnode[i].part);
		partnode[i].index = i;
	}
	fclose(fp);
	
	fp = fopen(file2,"r");
	for(int i=0; i<ncell; i++)
	{
		fscanf(fp, "%d\n", &partcell[i].part);
		partcell[i].index = i;
	}
	fclose(fp);
	
	nparts = atoi(const_cast<char*>(number));
}
