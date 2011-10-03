# 1 "/home/abetts/SILOET/OP2_ROSE_Fortran/tests/Fortran/Globals/OP2-PhzI9u.F90"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/home/abetts/SILOET/OP2_ROSE_Fortran/tests/Fortran/Globals/OP2-PhzI9u.F90"
MODULE OP2_C
USE ISO_C_BINDING
INTEGER, PARAMETER :: MAX_NAME_LEN = 100
INTEGER(kind=4) :: OP_READ = 1
INTEGER(kind=4) :: OP_WRITE = 2
INTEGER(kind=4) :: OP_INC = 3
INTEGER(kind=4) :: OP_RW = 4
INTEGER(kind=4) :: OP_MAX = 5
INTEGER(kind=4) :: OP_MIN = 6
TYPE , PUBLIC :: op_set
INTEGER :: id
END TYPE op_set
TYPE , PUBLIC :: op_map
INTEGER :: id
END TYPE op_map
TYPE , PUBLIC :: op_dat
INTEGER :: id
END TYPE op_dat
TYPE ( op_map )  :: OP_ID, OP_GBL
INTERFACE op_decl_dat
MODULE PROCEDURE op_decl_dat_real_8
MODULE PROCEDURE op_decl_dat_integer_4
END INTERFACE 
INTERFACE op_decl_gbl
MODULE PROCEDURE op_decl_gbl_real_8
MODULE PROCEDURE op_decl_gbl_integer_4
MODULE PROCEDURE op_decl_gbl_scalar_integer_4
MODULE PROCEDURE op_decl_gbl_scalar_real_8
END INTERFACE 
INTERFACE op_decl_const
MODULE PROCEDURE op_decl_const_scalar_logical
MODULE PROCEDURE op_decl_const_scalar_real_4
MODULE PROCEDURE op_decl_const_scalar_real_8
MODULE PROCEDURE op_decl_const_scalar_integer_4
MODULE PROCEDURE op_decl_const_logical
MODULE PROCEDURE op_decl_const_real_4
MODULE PROCEDURE op_decl_const_real_8
MODULE PROCEDURE op_decl_const_integer_4
END INTERFACE 
CONTAINS
SUBROUTINE op_init()
END SUBROUTINE op_init

  ! OP_SET declaration
SUBROUTINE op_decl_set(size,set,setname)
INTEGER :: size
TYPE ( op_set )  :: set
CHARACTER(kind=c_char) :: setname*(*)
END SUBROUTINE op_decl_set

  ! OP_MAP declaration
SUBROUTINE op_decl_map(from,to,dim,dat,outmap,mapname)
TYPE ( op_set )  :: from, to
INTEGER :: dim
INTEGER(kind=4) :: dat(:)
TYPE ( op_map )  :: outmap
CHARACTER(kind=c_char) :: mapname*(*)
END SUBROUTINE op_decl_map

  ! OP_DAT declarations
SUBROUTINE op_decl_dat_real_8(set,dim,dat,data,datname)
TYPE ( op_set )  :: set
INTEGER :: dim
REAL(kind=8) :: dat(:)
TYPE ( op_dat )  :: data
CHARACTER(kind=c_char) :: datname*(*)
END SUBROUTINE op_decl_dat_real_8

SUBROUTINE op_decl_dat_integer_4(set,dim,dat,data,datname)
TYPE ( op_set )  :: set
INTEGER :: dim
INTEGER(kind=4) :: dat(:)
TYPE ( op_dat )  :: data
CHARACTER(kind=c_char) :: datname*(*)
END SUBROUTINE op_decl_dat_integer_4

  ! OP_GBL declarations
SUBROUTINE op_decl_gbl_real_8(dat,dim,data)
REAL(kind=8) :: dat(:)
INTEGER :: dim
TYPE ( op_dat )  :: data
END SUBROUTINE op_decl_gbl_real_8

SUBROUTINE op_decl_gbl_integer_4(dat,dim,data)
INTEGER(kind=4) :: dat(:)
INTEGER :: dim
TYPE ( op_dat )  :: data
END SUBROUTINE op_decl_gbl_integer_4

SUBROUTINE op_decl_gbl_scalar_integer_4(dat,data)
INTEGER(kind=4) :: dat
TYPE ( op_dat )  :: data
END SUBROUTINE op_decl_gbl_scalar_integer_4

SUBROUTINE op_decl_gbl_scalar_real_8(dat,data)
REAL(kind=8) :: dat
TYPE ( op_dat )  :: data
END SUBROUTINE op_decl_gbl_scalar_real_8

  ! OP_CONST declarations
SUBROUTINE op_decl_const_scalar_logical(dim,data)
INTEGER(kind=4) :: dim
LOGICAL :: data
END SUBROUTINE op_decl_const_scalar_logical

SUBROUTINE op_decl_const_scalar_real_4(dim,data)
INTEGER(kind=4) :: dim
REAL(kind=4) :: data
END SUBROUTINE op_decl_const_scalar_real_4

SUBROUTINE op_decl_const_scalar_real_8(dim,data)
INTEGER(kind=4) :: dim
REAL(kind=8) :: data
END SUBROUTINE op_decl_const_scalar_real_8

SUBROUTINE op_decl_const_scalar_integer_4(dim,data)
INTEGER(kind=4) :: dim
INTEGER(kind=4) :: data
END SUBROUTINE op_decl_const_scalar_integer_4

SUBROUTINE op_decl_const_logical(dim,data)
INTEGER(kind=4) :: dim
LOGICAL :: data(:)
END SUBROUTINE op_decl_const_logical

SUBROUTINE op_decl_const_real_4(dim,data)
INTEGER(kind=4) :: dim
REAL(kind=4) :: data(:)
END SUBROUTINE op_decl_const_real_4

SUBROUTINE op_decl_const_real_8(dim,data)
INTEGER(kind=4) :: dim
REAL(kind=8) :: data(:)
END SUBROUTINE op_decl_const_real_8

SUBROUTINE op_decl_const_integer_4(dim,data)
INTEGER(kind=4) :: dim
INTEGER(kind=4) :: data(:)
END SUBROUTINE op_decl_const_integer_4

  ! OP_PAR_LOOPs
SUBROUTINE op_par_loop_1(subroutineName,set,data0,itemSel0,map0,access0)
external :: subroutineName
TYPE ( op_set )  :: set
TYPE ( op_dat )  :: data0
INTEGER :: itemSel0, access0
TYPE ( op_map )  :: map0
END SUBROUTINE op_par_loop_1

SUBROUTINE op_par_loop_2(subroutineName,set,data0,itemSel0,map0,access0,data1,itemSel1,map1,access1)
external :: subroutineName
TYPE ( op_set )  :: set
TYPE ( op_dat )  :: data0, data1
INTEGER :: itemSel0, itemSel1, access0, access1
TYPE ( op_map )  :: map0, map1
END SUBROUTINE op_par_loop_2

SUBROUTINE op_par_loop_3(subroutineName,set,data0,itemSel0,map0,access0,data1,itemSel1,map1,access1,data2,itemSel2,map2,access2)
external :: subroutineName
TYPE ( op_set )  :: set
TYPE ( op_dat )  :: data0, data1, data2
INTEGER :: itemSel0, itemSel1, itemSel2, access0, access1, access2
TYPE ( op_map )  :: map0, map1, map2
END SUBROUTINE op_par_loop_3

SUBROUTINE op_par_loop_4(subroutineName,set,data0,itemSel0,map0,access0,data1,itemSel1,map1,access1,data2,itemSel2,map2,access2,data3,itemSel3,map3,access3)
external :: subroutineName
TYPE ( op_set )  :: set
TYPE ( op_dat )  :: data0, data1, data2, data3
INTEGER :: itemSel0, itemSel1, itemSel2, itemSel3, access0, access1, access2, access3
TYPE ( op_map )  :: map0, map1, map2, map3
END SUBROUTINE op_par_loop_4

SUBROUTINE op_par_loop_5(subroutineName,set,data0,itemSel0,map0,access0,data1,itemSel1,map1,access1,data2,itemSel2,map2,access2,data3,itemSel3,map3,access3,data4,itemSel4,map4,access4)
external :: subroutineName
TYPE ( op_set )  :: set
TYPE ( op_dat )  :: data0, data1, data2, data3, data4
INTEGER :: itemSel0, itemSel1, itemSel2, itemSel3, itemSel4
INTEGER :: access0, access1, access2, access3, access4
TYPE ( op_map )  :: map0, map1, map2, map3, map4
END SUBROUTINE op_par_loop_5

SUBROUTINE op_par_loop_6(subroutineName,set,data0,itemSel0,map0,access0,data1,itemSel1,map1,access1,data2,itemSel2,map2,access2,data3,itemSel3,map3,access3,data4,itemSel4,map4,access4,data5,itemSel5,map5,access5)
external :: subroutineName
TYPE ( op_set )  :: set
TYPE ( op_dat )  :: data0, data1, data2, data3, data4, data5
INTEGER :: itemSel0, itemSel1, itemSel2, itemSel3, itemSel4, itemSel5
INTEGER :: access0, access1, access2, access3, access4, access5
TYPE ( op_map )  :: map0, map1, map2, map3, map4, map5
END SUBROUTINE op_par_loop_6

SUBROUTINE op_par_loop_8(subroutineName,set,data0,itemSel0,map0,access0,data1,itemSel1,map1,access1,data2,itemSel2,map2,access2,data3,itemSel3,map3,access3,data4,itemSel4,map4,access4,data5,itemSel5,map5,access5,data6,itemSel6,map6,access6,data7,itemSel7,map7,access7)
external :: subroutineName
TYPE ( op_set )  :: set
TYPE ( op_dat )  :: data0, data1, data2, data3, data4, data5, data6, data7
INTEGER :: itemSel0, itemSel1, itemSel2, itemSel3, itemSel4, itemSel5, itemSel6, itemSel7
INTEGER :: access0, access1, access2, access3, access4, access5, access6, access7
TYPE ( op_map )  :: map0, map1, map2, map3, map4, map5, map6, map7
END SUBROUTINE op_par_loop_8

END MODULE OP2_C

