PROGRAM airfoil
  USE OP2_C
  USE constantVars
  USE ISO_C_BINDING
  USE GENERATED_MODULE
  IMPLICIT NONE
  intrinsic sqrt, real
  !external save_soln, adt_calc, res_calc, bres_calc, update
!	integer(c_int) :: retDebug, dataSize
!!	character(kind=c_char) :: debfilename(20)
!	real(c_double) :: datad
!	integer(4) :: debugiter
  INTEGER(kind=4) :: iter,k,i
  INTEGER(kind=4), PARAMETER :: maxnode = 9900
  INTEGER(kind=4), PARAMETER :: maxcell = 9702 + 1
  INTEGER(kind=4), PARAMETER :: maxedge = 19502
  INTEGER(kind=4), PARAMETER :: iterationNumber = 1000
  INTEGER(kind=4) :: nnode,ncell,nbedge,nedge,niter
  REAL(kind=8) :: ncellr
  ! integer references (valid inside the OP2 library) for op_set
  TYPE ( op_set )  :: nodes,edges,bedges,cells
  ! integer references (valid inside the OP2 library) for pointers between data sets
  TYPE ( op_map )  :: pedge,pecell,pcell,pbedge,pbecell
  ! integer reference (valid inside the OP2 library) for op_data 
  TYPE ( op_dat )  :: p_bound,p_x,p_q,p_qold,p_adt,p_res,p_rms
  ! arrays used in data
  INTEGER(kind=4), DIMENSION(:), ALLOCATABLE, TARGET :: ecell,bound,edge,bedge,becell,cell
  REAL(kind=8), DIMENSION(:), ALLOCATABLE, TARGET :: x,q,qold,adt,res,rms
  character(len=10,kind=c_char) :: savesolnName = 'save_soln' // C_NULL_CHAR
  character(len=9,kind=c_char) :: adtcalcName = 'adt_calc' // C_NULL_CHAR
  character(len=9,kind=c_char) :: rescalcName = 'res_calc' // C_NULL_CHAR
  character(len=10,kind=c_char) :: brescalcName = 'bres_calc' // C_NULL_CHAR
  character(len=7,kind=c_char) :: updateName = 'update' // C_NULL_CHAR
  character(len=6,kind=c_char) :: nodesName = 'nodes' // C_NULL_CHAR
  character(len=6,kind=c_char) :: edgesName = 'edges' // C_NULL_CHAR
  character(len=7,kind=c_char) :: bedgesName = 'bedges' // C_NULL_CHAR
  character(len=6,kind=c_char) :: cellsName = 'cells' // C_NULL_CHAR
  character(len=6,kind=c_char) :: pedgeName = 'pedge' // C_NULL_CHAR
  character(len=7,kind=c_char) :: pecellName = 'pecell' // C_NULL_CHAR
  character(len=6,kind=c_char) :: pcellName = 'pcell' // C_NULL_CHAR
  character(len=7,kind=c_char) :: pbedgeName = 'pbedge' // C_NULL_CHAR
  character(len=8,kind=c_char) :: pbecellName = 'pbecell' // C_NULL_CHAR
  character(len=6,kind=c_char) :: boundName = 'bound' // C_NULL_CHAR
  character(len=2,kind=c_char) :: xName = 'x' // C_NULL_CHAR
  character(len=2,kind=c_char) :: qName = 'q' // C_NULL_CHAR
  character(len=5,kind=c_char) :: qoldName = 'qold' // C_NULL_CHAR
  character(len=4,kind=c_char) :: adtName = 'adt' // C_NULL_CHAR
  character(len=4,kind=c_char) :: resName = 'res' // C_NULL_CHAR
  character(len=8) :: adt_calc_user = "adt_calc"
  character(len=9) :: bres_calc_user = "bres_calc"
  character(len=8) :: res_calc_user = "res_calc"
  character(len=9) :: save_soln_user = "save_soln"
  character(len=6) :: update_user = "update"
  ! read set sizes from input file (input is subdivided in two routines as we cannot allocate arrays in subroutines in
  ! fortran 90)
  CALL getSetSizes(nnode,  ncell,  nedge,  nbedge)
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
  allocate( rms(1) )
  ! fill up arrays from file
  CALL getSetInfo(nnode,  ncell,  nedge,  nbedge,  cell,  edge,  ecell,  bedge,  becell,  bound,  x&
  &,  q,  qold,  res,  adt)
  ! set constants and initialise flow field and residual
  CALL initialise_flow_field(ncell,  q,  res)

  DO iter = 1, 4 * ncell
    res(iter) = 0.0
  END DO

  ! OP initialisation
  CALL op_init()
  ! declare sets, pointers, datasets and global constants (for now, no new partition info)
  CALL op_decl_set(nnode,  nodes,  nodesName)
  CALL op_decl_set(nedge,  edges,  edgesName)
  CALL op_decl_set(nbedge,  bedges,  bedgesName)
  CALL op_decl_set(ncell,  cells,  cellsName)
  CALL op_decl_map(edges,  nodes,  2,  edge,  pedge,  pedgeName)
  CALL op_decl_map(edges,  cells,  2,  ecell,  pecell,  pecellName)
  CALL op_decl_map(bedges,  nodes,  2,  bedge,  pbedge,  pbedgeName)
  CALL op_decl_map(bedges,  cells,  1,  becell,  pbecell,  pecellName)
  CALL op_decl_map(cells,  nodes,  4,  cell,  pcell,  pcellName)
  CALL op_decl_dat(bedges,  1,  bound,  p_bound,  boundName)
  CALL op_decl_dat(nodes,  2,  x,  p_x,  xName)
  CALL op_decl_dat(cells,  4,  q,  p_q,  qName)
  CALL op_decl_dat(cells,  4,  qold,  p_qold,  qoldName)
  CALL op_decl_dat(cells,  1,  adt,  p_adt,  adtName)
  CALL op_decl_dat(cells,  4,  res,  p_res,  resName)
  CALL op_decl_gbl(rms,  1,  p_rms)
  CALL op_decl_const(1,  gam)
  CALL op_decl_const(1,  gm1)
  CALL op_decl_const(1,  cfl)
  CALL op_decl_const(1,  eps)
  CALL op_decl_const(1,  mach)
  CALL op_decl_const(1,  alpha)
  CALL op_decl_const(4,  qinf)
  ! main time-marching loop

  DO niter = 1, iterationNumber
    ! save old flow solution
    CALL save_soln_host(save_soln_user,  cells,  p_q,  -1,  OP_ID,  OP_READ,  p_qold,  -1,  OP_ID, &
    & OP_WRITE)
    ! predictor/corrector update loop

    DO k = 1, 2
      ! calculate area/timstep
      CALL adt_calc_host(adt_calc_user,  cells,  p_x,  1,  pcell,  OP_READ,  p_x,  2,  pcell,  OP_R&
      &EAD,  p_x,  3,  pcell,  OP_READ,  p_x,  4,  pcell,  OP_READ,  p_q,  -1,  OP_ID,  OP_READ,  p&
      &_adt,  -1,  OP_ID,  OP_WRITE)
      ! calculate flux residual
      CALL res_calc_host(res_calc_user,  edges,  p_x,  1,  pedge,  OP_READ,  p_x,  2,  pedge,  OP_R&
      &EAD,  p_q,  1,  pecell,  OP_READ,  p_q,  2,  pecell,  OP_READ,  p_adt,  1,  pecell,  OP_READ&
      &,  p_adt,  2,  pecell,  OP_READ,  p_res,  1,  pecell,  OP_INC,  p_res,  2,  pecell,  OP_INC)
      CALL bres_calc_host(bres_calc_user,  bedges,  p_x,  1,  pbedge,  OP_READ,  p_x,  2,  pbedge, &
      & OP_READ,  p_q,  1,  pbecell,  OP_READ,  p_adt,  1,  pbecell,  OP_READ,  p_res,  1,  pbecell&
      &,  OP_INC,  p_bound,  -1,  OP_ID,  OP_READ)
      ! update flow field
      rms(1) = 0.0
      CALL update_host(update_user,  cells,  p_qold,  -1,  OP_ID,  OP_READ,  p_q,  -1,  OP_ID,  OP_&
      &WRITE,  p_res,  -1,  OP_ID,  OP_RW,  p_adt,  -1,  OP_ID,  OP_READ,  p_rms,  -1,  OP_GBL,  OP&
      &_INC)
    END DO

    ncellr = real(ncell)
    rms(1) = sqrt(rms(1) / ncellr)
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

