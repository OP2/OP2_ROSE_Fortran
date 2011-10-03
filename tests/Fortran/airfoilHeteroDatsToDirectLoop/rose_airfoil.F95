PROGRAM airfoil
  USE OP2_C
  USE constantVars
  USE ISO_C_BINDING
  USE GENERATED_MODULE
  IMPLICIT NONE
  intrinsic sqrt, real
  INTEGER(kind=4) :: iter, k, i
  INTEGER(kind=4), PARAMETER :: maxnode = 9900
  INTEGER(kind=4), PARAMETER :: maxcell = 9702 + 1
  INTEGER(kind=4), PARAMETER :: maxedge = 19502
  INTEGER(kind=4), PARAMETER :: iterationNumber = 1000
  INTEGER(kind=4) :: nnode, ncell, nbedge, nedge, niter
  REAL(kind=8) :: ncellr
  ! integer references (valid inside the OP2 library) for op_set
  TYPE ( op_set )  :: nodes, edges, bedges, cells
  ! integer references (valid inside the OP2 library) for pointers between data sets
  TYPE ( op_map )  :: pedge, pecell, pcell, pbedge, pbecell
  ! integer reference (valid inside the OP2 library) for op_data 
  TYPE ( op_dat )  :: p_bound, p_x, p_q, p_qold, p_adt, p_res, p_rms, p_fakeInt
  ! arrays used in data
  INTEGER(kind=4), ALLOCATABLE, TARGET :: ecell(:), bound(:), edge(:), bedge(:), becell(:), cell(:), fakeInt(:)
  REAL(kind=8), ALLOCATABLE, TARGET :: x(:), q(:), qold(:), adt(:), res(:)
  REAL(kind=8) :: rms
  CHARACTER(kind=c_char) :: savesolnName*(10) = 'save_soln' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: adtcalcName*(9) = 'adt_calc' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: rescalcName*(9) = 'res_calc' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: brescalcName*(10) = 'bres_calc' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: updateName*(7) = 'update' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: nodesName*(6) = 'nodes' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: edgesName*(6) = 'edges' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: bedgesName*(7) = 'bedges' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: cellsName*(6) = 'cells' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: pedgeName*(6) = 'pedge' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: pecellName*(7) = 'pecell' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: pcellName*(6) = 'pcell' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: pbedgeName*(7) = 'pbedge' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: pbecellName*(8) = 'pbecell' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: boundName*(6) = 'bound' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: xName*(2) = 'x' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: qName*(2) = 'q' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: qoldName*(5) = 'qold' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: adtName*(4) = 'adt' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: resName*(4) = 'res' // C_NULL_CHAR
  CHARACTER(kind=c_char) :: fakeIntName*(3) = 'fi' // C_NULL_CHAR
  CHARACTER :: adt_calc_user*(8) = "adt_calc"
  CHARACTER :: bres_calc_user*(9) = "bres_calc"
  CHARACTER :: res_calc_user*(8) = "res_calc"
  CHARACTER :: save_soln_user*(9) = "save_soln"
  CHARACTER :: update_user*(6) = "update"
  ! read set sizes from input file (input is subdivided in two routines as we cannot allocate arrays in subroutines in
  ! fortran 90)
  CALL getSetSizes(nnode,ncell,nedge,nbedge)
  ! allocate sets (cannot allocate in subroutine in F90)
  allocate( cell(4 * ncell) )
  allocate( edge(2 * nedge) )
  allocate( ecell(2 * nedge) )
  allocate( bedge(2 * nbedge) )
  allocate( becell(nbedge) )
  allocate( bound(nbedge) )
  allocate( x(2 * nnode) )
  allocate( q(4 * ncell) )
  allocate( qold(4 * ncell) )
  allocate( res(4 * ncell) )
  allocate( adt(ncell) )
  allocate( fakeInt(10 * ncell) )
  ! fill up arrays from file
  CALL getSetInfo(nnode,ncell,nedge,nbedge,cell,edge,ecell,bedge,becell,bound,x,q,qold,res,adt)
  ! set constants and initialise flow field and residual
  CALL initialise_flow_field(ncell,q,res)

  DO iter = 1, 4 * ncell
    res(iter) = 0.0
  END DO

  DO iter = 1, 10 * ncell
    fakeInt(iter) = 0
  END DO

  ! OP initialisation
  CALL op_init()
  ! declare sets, pointers, datasets and global constants (for now, no new partition info)
  CALL op_decl_set(nnode,nodes,nodesName)
  CALL op_decl_set(nedge,edges,edgesName)
  CALL op_decl_set(nbedge,bedges,bedgesName)
  CALL op_decl_set(ncell,cells,cellsName)
  CALL op_decl_map(edges,nodes,2,edge,pedge,pedgeName)
  CALL op_decl_map(edges,cells,2,ecell,pecell,pecellName)
  CALL op_decl_map(bedges,nodes,2,bedge,pbedge,pbedgeName)
  CALL op_decl_map(bedges,cells,1,becell,pbecell,pecellName)
  CALL op_decl_map(cells,nodes,4,cell,pcell,pcellName)
  CALL op_decl_dat(bedges,1,bound,p_bound,boundName)
  CALL op_decl_dat(nodes,2,x,p_x,xName)
  CALL op_decl_dat(cells,4,q,p_q,qName)
  CALL op_decl_dat(cells,4,qold,p_qold,qoldName)
  CALL op_decl_dat(cells,1,adt,p_adt,adtName)
  CALL op_decl_dat(cells,4,res,p_res,resName)
  CALL op_decl_dat(cells,10,fakeInt,p_fakeInt,fakeIntName)
  CALL op_decl_gbl(rms,p_rms)
  CALL op_decl_const(1,gam)
  CALL op_decl_const(1,gm1)
  CALL op_decl_const(1,cfl)
  CALL op_decl_const(1,eps)
  CALL op_decl_const(1,mach)
  CALL op_decl_const(1,alpha)
  CALL op_decl_const(4,qinf)
  ! main time-marching loop

  DO niter = 1, iterationNumber
    ! save old flow solution
    CALL save_soln_host(save_soln_user,cells,p_q,-1,OP_ID,OP_READ,p_fakeInt,-1,OP_ID,OP_RW,p_qold,-1,OP_ID,OP_WRITE)
    ! predictor/corrector update loop

    DO k = 1, 2
      ! calculate area/timstep
      CALL adt_calc_host(adt_calc_user,cells,p_x,1,pcell,OP_READ,p_x,2,pcell,OP_READ,p_x,3,pcell,OP_READ,p_x,4,pcell,OP&
      &_READ,p_q,-1,OP_ID,OP_READ,p_adt,-1,OP_ID,OP_WRITE)
      ! calculate flux residual
      CALL res_calc_host(res_calc_user,edges,p_x,1,pedge,OP_READ,p_x,2,pedge,OP_READ,p_q,1,pecell,OP_READ,p_q,2,pecell,&
      &OP_READ,p_adt,1,pecell,OP_READ,p_adt,2,pecell,OP_READ,p_res,1,pecell,OP_INC,p_res,2,pecell,OP_INC)
      CALL bres_calc_host(bres_calc_user,bedges,p_x,1,pbedge,OP_READ,p_x,2,pbedge,OP_READ,p_q,1,pbecell,OP_READ,p_adt,1&
      &,pbecell,OP_READ,p_res,1,pbecell,OP_INC,p_bound,-1,OP_ID,OP_READ)
      ! update flow field
      rms = 0.0
      CALL update_host(update_user,cells,p_qold,-1,OP_ID,OP_READ,p_q,-1,OP_ID,OP_WRITE,p_res,-1,OP_ID,OP_RW,p_adt,-1,OP&
      &_ID,OP_READ,p_rms,-1,OP_GBL,OP_INC)
    END DO

    ncellr = real(ncell)
    rms = sqrt(rms / ncellr)
  END DO

! uncomment the following statements to get the result of the airfoil written in a file
!	retDebug = openfile ( C_CHAR_"/work/cbertoll/airfoil-generated/q.txt"//C_NULL_CHAR )
!
!	do debugiter = 1, 4*ncell
!
!		datad = q(debugiter)
!		retDebug = writeRealToFile ( datad )
!
!	retDebug = closefile ()
!	end do
END PROGRAM airfoil

