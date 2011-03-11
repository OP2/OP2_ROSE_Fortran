


program airfoil

use OP2_C
use constantVars
use airfoil_seq

  implicit none

  intrinsic :: sqrt, real

  !external save_soln, adt_calc, res_calc, bres_calc, update


  integer(4) :: iter, k, i

  integer(4), parameter :: maxnode = 9900
  integer(4), parameter :: maxcell = (9702+1)
  integer(4), parameter :: maxedge = 19502

	integer(4), parameter :: iterationNumber = 1000


  integer(4) :: nnode, ncell, nbedge, nedge, niter
  real(8) :: ncellr

  ! integer references (valid inside the OP2 library) for op_set
  type(op_set) :: nodes, edges, bedges, cells

  ! integer references (valid inside the OP2 library) for pointers between data sets
  type(op_map) :: pedge, pecell, pcell, pbedge, pbecell

  ! integer reference (valid inside the OP2 library) for op_data 
  type(op_dat) :: p_bound, p_x, p_q, p_qold, p_adt, p_res, p_rms
  
  ! arrays used in data
  integer(4), dimension(:), allocatable, target :: ecell, bound, edge, bedge, becell, cell
  real(8), dimension(:), allocatable, target :: x, q, qold, adt, res, rms

!	character(kind=c_char), dimension(5) :: nodesName =	'nodes'
!	character(kind=c_char), dimension(5) :: edgesName =	'edges'
!	character(kind=c_char), dimension(6) :: bedgesName = 'bedges'
!	character(kind=c_char,len=1), dimension(5) :: cellsName =	'cells'

!	character(kind=c_char), dimension(5) :: pedgeName =	'pedge'
!	character(kind=c_char), dimension(6) :: pecellName =	'pecell'
!	character(kind=c_char), dimension(5) :: pcellName =	'pcell'
!	character(kind=c_char), dimension(6) :: pbedgeName =	'pbedge'
!	character(kind=c_char), dimension(7) :: pbecellName =	'pbecell'

!	character(kind=c_char), dimension(5) :: boundName =	'bound'
!	character(kind=c_char), dimension(1) :: xName =	'x'
!	character(kind=c_char), dimension(1) :: qName =	'q'
!	character(kind=c_char), dimension(4) :: qoldName =	'qold'
!	character(kind=c_char), dimension(3) :: adtName =	'adt'
!	character(kind=c_char), dimension(3) :: resName =	'res'

	! read set sizes from input file (input is subdivided in two routines as we cannot allocate arrays in subroutines in
	! fortran 90)
	call getSetSizes ( nnode, ncell, nedge, nbedge )
	
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

	allocate ( rms ( 1 ) )

	! fill up arrays from file
	call getSetInfo ( nnode, ncell, nedge, nbedge, cell, edge, ecell, bedge, becell, bound, x, q, qold, res, adt )

  ! set constants and initialise flow field and residual
	call initialise_flow_field ( ncell, q, res )
	
  do iter = 1, 4*ncell
    res(iter) = 0.0
  end do 

  ! OP initialisation
	call op_init()
	
  ! declare sets, pointers, datasets and global constants (for now, no new partition info)
  call op_decl_set ( nnode, nodes )!, nodesName )
  call op_decl_set ( nedge, edges )!, edgesName )
	call op_decl_set ( nbedge, bedges)!, bedgesName )
  call op_decl_set ( ncell, cells)!, cellsName )

  call op_decl_map ( edges, nodes, 2, edge, pedge)!, pedgeName ) 
  call op_decl_map ( edges, cells, 2, ecell, pecell)!, pecellName )
	call op_decl_map ( bedges, nodes, 2, bedge, pbedge)!, pbedgeName )
	call op_decl_map ( bedges, cells, 1, becell, pbecell)!, pecellName )
  call op_decl_map ( cells, nodes, 4, cell, pcell)!, pcellName )


  call op_decl_dat ( bedges, 1, bound, p_bound)!, boundName )	
  call op_decl_dat ( nodes, 2, x, p_x)!, xName )
  call op_decl_dat ( cells, 4, q, p_q)!, qName )
	call op_decl_dat ( cells, 4, qold, p_qold)!, qoldName )
  call op_decl_dat ( cells, 1, adt, p_adt)!, adtName )
  call op_decl_dat ( cells, 4, res, p_res)!, resName )
	
	call op_decl_gbl ( rms, 1, p_rms )
	
	! main time-marching loop
	
  do niter = 1, iterationNumber

		! save old flow solution

    call op_par_loop_2 ( save_soln, cells, &
                       & p_q,    -1, OP_ID, OP_READ, &
                       & p_qold, -1, OP_ID, OP_WRITE &
                     & )
	
		! predictor/corrector update loop
				
    do k = 1, 2

      ! calculate area/timstep

      call op_par_loop_6 ( adt_calc, cells, &
                         & p_x,   1, pcell, OP_READ, &
                         & p_x,   2, pcell, OP_READ, &
                         & p_x,   3, pcell, OP_READ, &
                         & p_x,   4, pcell, OP_READ, &
                         & p_q,   -1, OP_ID, OP_READ, &
                         & p_adt, -1, OP_ID, OP_WRITE &
                       & )



      ! calculate flux residual

      call op_par_loop_8 ( res_calc, edges, &
                         & p_x,    1, pedge,  OP_READ, &
                         & p_x,    2, pedge,  OP_READ, &
                         & p_q,    1, pecell, OP_READ, &
                         & p_q,    2, pecell, OP_READ, &
                         & p_adt,  1, pecell, OP_READ, &
                         & p_adt,  2, pecell, OP_READ, &
                         & p_res,  1, pecell, OP_INC,  &
                         & p_res,  2, pecell, OP_INC   &
											 & )


      call op_par_loop_6 ( bres_calc, bedges, &
												 & p_x,      1, pbedge,  OP_READ, &
												 & p_x,      2, pbedge,  OP_READ, &
												 & p_q,      1, pbecell, OP_READ, &
												 & p_adt,    1, pbecell, OP_READ, &
											   & p_res,    1, pbecell, OP_INC,  &
                         & p_bound,  -1, OP_ID, OP_READ  &
											 & )

      ! update flow field
      
      rms(1) = 0.0

      call op_par_loop_5 ( update, cells, &
                         & p_qold, -1, OP_ID,  OP_READ,  &
                         & p_q,    -1, OP_ID,  OP_WRITE, &
                         & p_res,  -1, OP_ID,  OP_RW,    &
                         & p_adt,  -1, OP_ID,  OP_READ,  &
                         & p_rms,  -1, OP_GBL, OP_INC    &
                       & )


    end do ! internal loop

    ncellr = real ( ncell )
    rms(1) = sqrt ( rms(1) / ncellr )
	
  end do ! external loop

end program airfoil