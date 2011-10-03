PROGRAM Test5
  USE OP2_C
  USE GENERATED_MODULE
  ! Host declarations
  INTEGER(kind=4) :: setCardinality
  INTEGER(kind=4) :: hostGlobal1
  INTEGER(kind=4) :: hostGlobal2
  ! OP2 declarations
  TYPE ( op_set )  :: set
  TYPE ( op_dat )  :: global1
  TYPE ( op_dat )  :: global2
  CHARACTER :: integer4Scalar_integer4Scalar_user*(29) = "integer4Scalar_integer4Scalar"
  setCardinality = 10
  hostGlobal1 = 0
  hostGlobal2 = 10
  CALL op_init()
  CALL op_decl_set(setCardinality,set,"")
  CALL op_decl_gbl(hostGlobal1,global1)
  CALL op_decl_gbl(hostGlobal2,global2)
  CALL s3306241251(integer4Scalar_integer4Scalar_user,set,global1,-1,OP_GBL,OP_INC,global2,-1,OP_GBL,OP_READ)
  PRINT *, hostGlobal1
END PROGRAM Test5

