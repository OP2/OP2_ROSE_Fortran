PROGRAM Test1
  USE OP2_C
  USE GENERATED_MODULE
  INTEGER(kind=4) :: i
  TYPE ( op_set )  :: iterationSet
  INTEGER(kind=4) :: setSize
  REAL(kind=8), DIMENSION(:), ALLOCATABLE :: A,C
  INTEGER(kind=4), DIMENSION(:), ALLOCATABLE :: B
  TYPE ( op_dat )  :: opDatA,opDatB,opDatC
  character(len=35) :: real8Array_integer4Array_real8Array_user = "real8Array_integer4Array_real8Array"
  setSize = 20
  allocate( A(setSize * 4) )
  allocate( B(setSize * 7) )
  allocate( C(setSize * 11) )

  DO i = 1, setSize * 4
    A(i) = mod(i - 1,  4)
  END DO

  DO i = 1, setSize * 7
    B(i) = mod(i - 1,  7)
  END DO

  CALL op_decl_set(setSize,  iterationSet,  "")
  CALL op_decl_dat(iterationSet,  4,  A,  opDatA,  "")
  CALL op_decl_dat(iterationSet,  7,  B,  opDatB,  "")
  CALL op_decl_dat(iterationSet,  11,  C,  opDatC,  "")
  CALL op_init()
  CALL s3168439322(real8Array_integer4Array_real8Array_user,  iterationSet,  opDatA,  -1,  OP_ID,  OP_READ,  opDatB,  -&
  &1,  OP_ID,  OP_READ,  opDatC,  -1,  OP_ID,  OP_WRITE)

  DO i = 1, 11 * setSize
    PRINT *, C(i)
  END DO

END PROGRAM Test1

