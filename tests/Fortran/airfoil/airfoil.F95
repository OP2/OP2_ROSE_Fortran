program AIRFOIL
  use OP2_FORTRAN_DECLARATIONS
  use OP2_CONSTANTS
  use AIRFOIL_SEQ
  use, intrinsic :: ISO_C_BINDING

  implicit none

  intrinsic :: sqrt, real

  integer(4) :: iter, k, i

  integer(4), parameter :: maxnode = 9900
  integer(4), parameter :: maxcell = (9702+1)
  integer(4), parameter :: maxedge = 19502

  integer(4), parameter :: iterationNumber = 1000

  integer(4) :: nnode, ncell, nbedge, nedge, niter, qdim
  real(8) :: ncellr

  ! integer references (valid inside the OP2 library) for op_set
  type(op_set) :: nodes, edges, bedges, cells

  ! integer references (valid inside the OP2 library) for pointers between data sets
  type(op_map) :: pedge, pecell, pcell, pbedge, pbecell

  ! integer reference (valid inside the OP2 library) for op_data 
  type(op_dat) :: p_bound, p_x, p_q, p_qold, p_adt, p_res
  
  ! arrays used in data
  integer(4), dimension(:), allocatable, target :: ecell, bound, edge, bedge, becell, cell
  real(8), dimension(:), allocatable, target :: x, q, qold, adt, res
  real(8) :: rms

  character(kind=c_char,len=10) :: savesolnName = C_CHAR_'save_soln'//C_NULL_CHAR
  character(kind=c_char, len=9) :: adtcalcName  = C_CHAR_'adt_calc' // C_NULL_CHAR
  character(kind=c_char, len=9) :: rescalcName  = C_CHAR_'res_calc' // C_NULL_CHAR
  character(kind=c_char,len=10) :: brescalcName = C_CHAR_'bres_calc' // C_NULL_CHAR
  character(kind=c_char, len=7) :: updateName   = C_CHAR_'update' // C_NULL_CHAR

  character(kind=c_char,len=6) :: nodesName  = C_CHAR_'nodes'//C_NULL_CHAR
  character(kind=c_char,len=6) :: edgesName  = C_CHAR_'edges'//C_NULL_CHAR
  character(kind=c_char,len=7) :: bedgesName = C_CHAR_'bedges'//C_NULL_CHAR
  character(kind=c_char,len=6) :: cellsName  = C_CHAR_'cells'//C_NULL_CHAR

  character(kind=c_char,len=6) :: pedgeName   = C_CHAR_'pedge'//C_NULL_CHAR
  character(kind=c_char,len=7) :: pecellName  = C_CHAR_'pecell'//C_NULL_CHAR
  character(kind=c_char,len=6) :: pcellName   = C_CHAR_'pcell'//C_NULL_CHAR
  character(kind=c_char,len=7) :: pbedgeName  = C_CHAR_'pbedge'//C_NULL_CHAR
  character(kind=c_char,len=8) :: pbecellName = C_CHAR_'pbecell'//C_NULL_CHAR

  character(kind=c_char,len=6) :: boundName = C_CHAR_'bound'//C_NULL_CHAR
  character(kind=c_char,len=2) :: xName     = C_CHAR_'x'//C_NULL_CHAR
  character(kind=c_char,len=2) :: qName     = C_CHAR_'q'//C_NULL_CHAR
  character(kind=c_char,len=5) :: qoldName  = C_CHAR_'qold'//C_NULL_CHAR
  character(kind=c_char,len=4) :: adtName   = C_CHAR_'adt'//C_NULL_CHAR
  character(kind=c_char,len=4) :: resName   = C_CHAR_'res'//C_NULL_CHAR

  character(kind=c_char,len=4) :: gamName   = C_CHAR_'gam'//C_NULL_CHAR
  character(kind=c_char,len=4) :: gm1Name   = C_CHAR_'gm1'//C_NULL_CHAR
  character(kind=c_char,len=4) :: cflName   = C_CHAR_'cfl'//C_NULL_CHAR
  character(kind=c_char,len=4) :: epsName   = C_CHAR_'eps'//C_NULL_CHAR
  character(kind=c_char,len=5) :: machName   = C_CHAR_'mach'//C_NULL_CHAR
  character(kind=c_char,len=6) :: alphaName   = C_CHAR_'alpha'//C_NULL_CHAR
  character(kind=c_char,len=5) :: qinfName   = C_CHAR_'qinf'//C_NULL_CHAR

  integer(4) :: debugiter, retDebug
  real(8) :: datad

  ! read set sizes from input file (input is subdivided in two routines as we cannot allocate arrays in subroutines in
  ! fortran 90)
  print *, "Getting set sizes"
  call getSetSizes ( nnode, ncell, nedge, nbedge )

  print *, ncell
  ! allocate sets (cannot allocate in subroutine in F90)
  allocate ( cell ( 4 * ncell ) )
  allocate ( edge ( 2 * nedge ) )
  allocate ( ecell ( 2 * nedge ) )
  allocate ( bedge ( 2 * nbedge ) )
  allocate ( becell ( nbedge ) )
  allocate ( bound ( nbedge ) )

  allocate ( x ( 2 * nnode ) )
  allocate ( q ( 4 * ncell ) )
  allocate ( qold ( 4 * ncell ) )
  allocate ( res ( 4 * ncell ) )
  allocate ( adt ( ncell ) )

  print *, "Getting data"
  call getSetInfo ( nnode, ncell, nedge, nbedge, cell, edge, ecell, bedge, becell, bound, x, q, qold, res, adt )

  print *, "Initialising constants"
  call initialise_flow_field ( ncell, q, res )

  do iter = 1, 4*ncell
    res(iter) = 0.0
  end do 

  ! OP initialisation
  print *, "Initialising OP2"
  call op_init (7)

  ! declare sets, pointers, datasets and global constants (for now, no new partition info)
  print *, "Declaring OP2 sets"
  call op_decl_set ( nnode, nodes, nodesName )
  call op_decl_set ( nedge, edges, edgesName )
  call op_decl_set ( nbedge, bedges, bedgesName )
  call op_decl_set ( ncell, cells, cellsName )

  print *, "Declaring OP2 maps"
  call op_decl_map ( edges, nodes, 2, edge, pedge, pedgeName ) 
  call op_decl_map ( edges, cells, 2, ecell, pecell, pecellName )
  call op_decl_map ( bedges, nodes, 2, bedge, pbedge, pbedgeName )
  call op_decl_map ( bedges, cells, 1, becell, pbecell, pecellName )
  call op_decl_map ( cells, nodes, 4, cell, pcell, pcellName )

  print *, "Declaring OP2 data"
  call op_decl_dat ( bedges, 1, bound, p_bound, boundName )
  call op_decl_dat ( nodes, 2, x, p_x, xName )
  call op_decl_dat ( cells, 4, q, p_q, qName )
  call op_decl_dat ( cells, 4, qold, p_qold, qoldName )
  call op_decl_dat ( cells, 1, adt, p_adt, adtName )
  call op_decl_dat ( cells, 4, res, p_res, resName )

  print *, "Declaring OP2 constants"
  call op_decl_const (gam, 1, gamName)
  call op_decl_const (gm1, 1, gm1Name)
  call op_decl_const (cfl, 1, cflName)
  call op_decl_const (eps, 1, epsName)
  call op_decl_const (mach, 1, machName)
  call op_decl_const (alpha, 1, alphaName)
  call op_decl_const (qinf, 4, qinfName)

  qdim = 4


    ! main time-marching loop

  do niter = 1, iterationNumber

    call save_soln_subroutine ( cells, qdim, p_q, p_qold )

!     call op_par_loop_2 ( save_soln, cells, &
!                        & op_arg_dat (p_q,    -1, OP_ID, OP_READ), &
!                        & op_arg_dat (p_qold, -1, OP_ID, OP_WRITE))

    ! predictor/corrector update loop

    do k = 1, 2

      ! calculate area/timstep
      call op_par_loop_6 ( adt_calc, cells, &
                         & op_arg_dat (p_x,    1, pcell, OP_READ), &
                         & op_arg_dat (p_x,    2, pcell, OP_READ), &
                         & op_arg_dat (p_x,    3, pcell, OP_READ), &
                         & op_arg_dat (p_x,    4, pcell, OP_READ), &
                         & op_arg_dat (p_q,   -1, OP_ID, OP_READ), &
                         & op_arg_dat (p_adt, -1, OP_ID, OP_WRITE))

      ! calculate flux residual
      call op_par_loop_8 ( res_calc, edges, &
                         & op_arg_dat (p_x,    1, pedge,  OP_READ), &
                         & op_arg_dat (p_x,    2, pedge,  OP_READ), &
                         & op_arg_dat (p_q,    1, pecell, OP_READ), &
                         & op_arg_dat (p_q,    2, pecell, OP_READ), &
                         & op_arg_dat (p_adt,  1, pecell, OP_READ), &
                         & op_arg_dat (p_adt,  2, pecell, OP_READ), &
                         & op_arg_dat (p_res,  1, pecell, OP_INC),  &
                         & op_arg_dat (p_res,  2, pecell, OP_INC))

      call op_par_loop_6 ( bres_calc, bedges, &
                          & op_arg_dat (p_x,      1, pbedge,  OP_READ), &
                          & op_arg_dat (p_x,      2, pbedge,  OP_READ), &
                          & op_arg_dat (p_q,      1, pbecell, OP_READ), &
                          & op_arg_dat (p_adt,    1, pbecell, OP_READ), &
                          & op_arg_dat (p_res,    1, pbecell, OP_INC),  &
                          & op_arg_dat (p_bound, -1, OP_ID, OP_READ))

      ! update flow field
      
      rms = 0.0

      call op_par_loop_5 ( update, cells, &
                         & op_arg_dat (p_qold, -1, OP_ID,  OP_READ),  &
                         & op_arg_dat (p_q,    -1, OP_ID,  OP_WRITE), &
                         & op_arg_dat (p_res,  -1, OP_ID,  OP_RW),    &
                         & op_arg_dat (p_adt,  -1, OP_ID,  OP_READ),  &
                         & op_arg_gbl (rms, OP_INC))


    end do ! internal loop

    ncellr = real ( ncell )
    rms = sqrt ( rms / ncellr )

    if (mod(niter,100) .eq. 0) print *, "=====> Iteration result ", rms

  end do ! external loop

! uncomment the following statements to get the result of the airfoil written in a file
! modify the path as convenient
!       call op_get_dat ( p_q )
!        retDebug = openfile ( C_CHAR_"/work/cbertoll/AirfoilFortran/CUDA/FuseTests/q.txt"//C_NULL_CHAR )
!        do debugiter = 1, 4*ncell
!                datad = q(debugiter)
!                retDebug = writeRealToFile ( datad )
!        end do
!        retDebug = closefile ()

end program AIRFOIL


subroutine save_soln_subroutine ( set, qdim, src, dest )

  use OP2_FORTRAN_DECLARATIONS
  use AIRFOIL_SEQ

  implicit none
  type(op_set) :: set
  integer(4) :: qdim
  type(op_dat) :: src, dest

  call op_par_loop_3 ( save_soln_qdim, set, &
                     & op_arg_gbl (qdim, OP_READ), &
                     & op_arg_dat_generic (src,  -1, OP_ID, 4, "real8", OP_READ), &
                     & op_arg_dat_generic (dest, -1, OP_ID, 4, "real8", OP_WRITE))

end subroutine
