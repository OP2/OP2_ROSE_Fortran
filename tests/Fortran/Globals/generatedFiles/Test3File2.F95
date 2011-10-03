PROGRAM Test3
  USE OP2_C
  USE GENERATED_MODULE
  ! Host declarations
  INTEGER(kind=4) :: setCardinality
  INTEGER(kind=4) :: hostGlobal
  REAL(kind=8), ALLOCATABLE :: hostData(:)
  INTEGER(kind=4) :: i
  ! OP2 declarations
  TYPE ( op_set )  :: set
  TYPE ( op_dat )  :: deviceData
  TYPE ( op_dat )  :: global
  CHARACTER :: real8Array_integer4Scalar_user*(25) = "real8Array_integer4Scalar"
  setCardinality = 10
  hostGlobal = 10
  allocate( hostData(4 * setCardinality) )

  DO i = 1, 4 * setCardinality
    hostData(i) = i
  END DO

  CALL op_init()
  CALL op_decl_set(setCardinality,set,"")
  CALL op_decl_dat(set,4,hostData,deviceData,"")
  CALL op_decl_gbl(hostGlobal,global)
  CALL s1010876094(real8Array_integer4Scalar_user,set,deviceData,-1,OP_ID,OP_WRITE,global,-1,OP_GBL,OP_READ)

  DO i = 1, 4 * setCardinality
    PRINT *, hostData(i)
  END DO

END PROGRAM Test3

