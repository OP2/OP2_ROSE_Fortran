PROGRAM Test4
  USE OP2_C
  USE GENERATED_MODULE
  ! Host declarations
  INTEGER(kind=4) :: setCardinality
  INTEGER(kind=4), DIMENSION(:), ALLOCATABLE :: hostGlobal
  REAL(kind=8), DIMENSION(:), ALLOCATABLE :: hostData
  INTEGER(kind=4) :: i
  ! OP2 declarations
  TYPE ( op_set )  :: set
  TYPE ( op_dat )  :: deviceData
  TYPE ( op_dat )  :: global
  character(len=24) :: real8Array_integer4Array_user = "real8Array_integer4Array"
  setCardinality = 10
  allocate( hostGlobal(2) )
  allocate( hostData(4 * setCardinality) )

  DO i = 1, 2
    hostGlobal(i) = i
  END DO

  DO i = 1, 4 * setCardinality
    hostData(i) = i
  END DO

  CALL op_init()
  CALL op_decl_set(setCardinality,  set,  "")
  CALL op_decl_dat(set,  4,  hostData,  deviceData,  "")
  CALL op_decl_gbl(hostGlobal,  2,  global)
  CALL s2168928953(real8Array_integer4Array_user,  set,  deviceData,  -1,  OP_ID,  OP_WRITE,  global,  -1,  OP_GBL,  OP&
  &_READ)

  DO i = 1, 4 * setCardinality
    PRINT *, hostData(i)
  END DO

END PROGRAM Test4

