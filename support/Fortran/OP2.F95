
module OP2_FORTRAN_DECLARATIONS

use, intrinsic :: ISO_C_BINDING

  integer, parameter :: MAX_NAME_LEN = 100

  ! accessing operation codes
  integer(4) :: OP_READ =   1
  integer(4) :: OP_WRITE =  2
  integer(4) :: OP_INC =    3
  integer(4) :: OP_RW =     4
  integer(4) :: OP_MIN =    5
  integer(4) :: OP_MAX =    6

  type op_set
  integer:: id, size
  end type op_set

  type op_map
  type(op_set) :: from, to
  integer:: id, dim, size
  end type op_map

  type op_dat
  type(op_set) :: set
  integer:: id, dim
  end type op_dat

  ! declaration of identity map
  type(op_map) :: OP_ID, OP_GBL

  interface op_decl_dat
    module procedure op_decl_dat_real_8_1, &
                   & op_decl_dat_integer_4_1, &
                   & op_decl_dat_real_8_2, &
                   & op_decl_dat_integer_4_2, &
                   & op_decl_dat_real_8_3, &
                   & op_decl_dat_integer_4_3
  end interface op_decl_dat
 
  interface op_decl_gbl
     module procedure op_decl_gbl_real_8_0, &
                    & op_decl_gbl_real_8_1, &
                    & op_decl_gbl_real_8_2, &
                    & op_decl_gbl_real_8_3, &
                    & op_decl_gbl_integer_4_0, &
                    & op_decl_gbl_integer_4_1, &
                    & op_decl_gbl_integer_4_2, &
                    & op_decl_gbl_integer_4_3, &
                    & op_decl_gbl_logical_0, &
                    & op_decl_gbl_logical_1
  end interface op_decl_gbl     

 interface op_decl_const
     module procedure op_decl_const_scalar_logical, &
                    & op_decl_const_scalar_real_4, &
                    & op_decl_const_scalar_real_8, &
                    & op_decl_const_scalar_integer_4, &
                    & op_decl_const_logical, &
                    & op_decl_const_real_4, &
                    & op_decl_const_real_8, &
                    & op_decl_const_real_8_2, &
                    & op_decl_const_integer_4
  end interface op_decl_const

contains

  subroutine op_init (deb)
      integer(4) :: deb
  end subroutine op_init

  subroutine op_decl_set ( size, set, setname )
    
    integer :: size
    type(op_set) :: set
		character(kind=c_char,len=*) :: setname

  end subroutine op_decl_set


  subroutine op_decl_map ( from , to, dim, dat, outmap, mapname )
    type(op_set) :: from, to
    integer :: dim
    integer(4), dimension(*) :: dat
    type(op_map) :: outmap
    character(kind=c_char,len=*) :: mapname
  end subroutine op_decl_map

  subroutine op_decl_dat_real_8_1(set, dim, dat, data, datname)
    type(op_set) :: set
    integer :: dim
    real(8), dimension(:) :: dat
    type(op_dat) :: data
		character(kind=c_char,len=*) :: datname
  end subroutine op_decl_dat_real_8_1
  
  subroutine op_decl_dat_integer_4_1(set, dim, dat, data, datname)
    type(op_set) :: set
    integer :: dim
    integer(4), dimension(:) :: dat
    type(op_dat) :: data  
		character(kind=c_char,len=*) :: datname
  end subroutine op_decl_dat_integer_4_1

  subroutine op_decl_dat_real_8_2(set, dim, dat, data, datname)
    type(op_set) :: set
    integer :: dim
    real(8), dimension(:,:) :: dat
    type(op_dat) :: data
		character(kind=c_char,len=*) :: datname
  end subroutine op_decl_dat_real_8_2
  
  subroutine op_decl_dat_integer_4_2(set, dim, dat, data, datname)
    type(op_set) :: set
    integer :: dim
    integer(4), dimension(:,:) :: dat
    type(op_dat) :: data  
		character(kind=c_char,len=*) :: datname
  end subroutine op_decl_dat_integer_4_2

  subroutine op_decl_dat_real_8_3(set, dim, dat, data, datname)
    type(op_set) :: set
    integer :: dim
    real(8), dimension(:,:,:) :: dat
    type(op_dat) :: data
		character(kind=c_char,len=*) :: datname
  end subroutine op_decl_dat_real_8_3
  
  subroutine op_decl_dat_integer_4_3(set, dim, dat, data, datname)
    type(op_set) :: set
    integer :: dim
    integer(4), dimension(:,:,:) :: dat
    type(op_dat) :: data  
		character(kind=c_char,len=*) :: datname
  end subroutine op_decl_dat_integer_4_3

  subroutine op_decl_gbl_real_8_0 ( dat, data)
    real(8) :: dat   
    type(op_dat) :: data
  end subroutine op_decl_gbl_real_8_0
  
  subroutine op_decl_gbl_real_8_1 ( dat, data, dim)
    real(8), dimension(:) :: dat
    type(op_dat) :: data
    integer :: dim
  end subroutine op_decl_gbl_real_8_1

  subroutine op_decl_gbl_real_8_2 ( dat, data, dim)
    real(8), dimension(:,:) :: dat
    type(op_dat) :: data
    integer :: dim
  end subroutine op_decl_gbl_real_8_2

  subroutine op_decl_gbl_integer_4_2 ( dat, data, dim)
    integer(4), dimension(:,:) :: dat    
    type(op_dat) :: data
    integer :: dim
  end subroutine op_decl_gbl_integer_4_2

  subroutine op_decl_gbl_real_8_3 ( dat, data, dim)
    real(8), dimension(:,:,:) :: dat
    type(op_dat) :: data
    integer :: dim
  end subroutine op_decl_gbl_real_8_3

  subroutine op_decl_gbl_integer_4_0 ( dat, data)
    integer(4) :: dat
    type(op_dat) :: data    
  end subroutine op_decl_gbl_integer_4_0

  subroutine op_decl_gbl_integer_4_1 ( dat, data, dim)
    integer(4), dimension(:) :: dat
    type(op_dat) :: data
    integer :: dim
  end subroutine op_decl_gbl_integer_4_1

  subroutine op_decl_gbl_integer_4_3 ( dat, data, dim)
    integer(4), dimension(:,:,:) :: dat
    type(op_dat) :: data
    integer :: dim
  end subroutine op_decl_gbl_integer_4_3

    subroutine op_decl_gbl_logical_0 ( dat, data)
        
        logical :: dat
        type(op_dat) :: data
        
    end subroutine op_decl_gbl_logical_0

    subroutine op_decl_gbl_logical_1 ( dat, data, gbldim)
        
        logical, dimension(*):: dat        
        type(op_dat) :: data
        integer :: gbldim
       
    end subroutine op_decl_gbl_logical_1

  subroutine op_decl_const_scalar_logical ( data, dim )
    
    integer(4) :: dim
    logical :: data
  
  end subroutine op_decl_const_scalar_logical

  subroutine op_decl_const_scalar_real_4 ( data, dim )
    
    integer(4) :: dim
    real(4) :: data
  
  end subroutine op_decl_const_scalar_real_4

  subroutine op_decl_const_scalar_real_8 ( data, dim )
    
    integer(4) :: dim
    real(8) :: data
  
  end subroutine op_decl_const_scalar_real_8

  subroutine op_decl_const_scalar_integer_4 ( data, dim )
    
    integer(4) :: dim
    integer(4) :: data
  
  end subroutine op_decl_const_scalar_integer_4



  subroutine op_decl_const_logical ( data, dim )
    
    integer(4) :: dim
    logical, dimension(:) :: data
  
  end subroutine op_decl_const_logical

  subroutine op_decl_const_real_4 ( data, dim )
    
    integer(4) :: dim
    real(4), dimension(:) :: data
  
  end subroutine op_decl_const_real_4

  subroutine op_decl_const_real_8 ( data, dim )
    
    integer(4) :: dim
    real(8), dimension(:) :: data
  
  end subroutine op_decl_const_real_8

  subroutine op_decl_const_integer_4 ( data, dim )
    
    integer(4) :: dim
    integer(4), dimension(:) :: data
  
  end subroutine op_decl_const_integer_4

  subroutine op_decl_const_real_8_2 ( data, dim1, dim2 )
    
    integer(4) :: dim1, dim2
    real(8), dimension(:,:) :: data
  
  end subroutine op_decl_const_real_8_2

end module OP2_FORTRAN_DECLARATIONS
