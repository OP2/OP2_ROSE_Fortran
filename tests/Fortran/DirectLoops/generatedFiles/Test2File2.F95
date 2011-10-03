PROGRAM Test2
  USE OP2_C
  USE GENERATED_MODULE
  INTEGER(kind=4) :: iter
  TYPE ( op_set )  :: edges
  TYPE ( op_dat )  :: x, y, z, k
  INTEGER(kind=4) :: numEdges, dimX, dimY, dimZ, dimK
  REAL(kind=8), ALLOCATABLE :: xd(:), yd(:), zd(:), kd(:)
  CHARACTER :: name*(2) = 'NO'
  CHARACTER :: real8Array_real8Array_real8Array_kernel1_user*(40) = "real8Array_real8Array_real8Array_kernel1"
  CHARACTER :: real8Array_real8Array_real8Array_kernel2_user*(40) = "real8Array_real8Array_real8Array_kernel2"
  numEdges = 1000000
  dimX = 6
  dimY = 6
  dimZ = 6
  dimK = 6
  allocate( xd(dimX * numEdges) )
  allocate( yd(dimY * numEdges) )
  allocate( zd(dimZ * numEdges) )
  allocate( kd(dimK * numEdges) )

  DO iter = 1, numEdges * 6
    xd(iter) = iter
    yd(iter) = iter
    zd(iter) = iter
    kd(iter) = iter
  END DO

  CALL op_decl_set(numEdges,edges,name)
  CALL op_decl_dat(edges,6,xd,x,name)
  CALL op_decl_dat(edges,6,yd,y,name)
  CALL op_decl_dat(edges,6,zd,z,name)
  CALL op_decl_dat(edges,6,kd,k,name)

  DO iter = 1, 1000
    CALL s97199258(real8Array_real8Array_real8Array_kernel1_user,edges,x,-1,OP_ID,OP_READ,y,-1,OP_ID,OP_READ,z,-1,OP_ID&
    &,OP_INC)
    CALL s2204063284(real8Array_real8Array_real8Array_kernel2_user,edges,x,-1,OP_ID,OP_READ,y,-1,OP_ID,OP_READ,k,-1,OP_&
    &ID,OP_INC)
  END DO

END PROGRAM

