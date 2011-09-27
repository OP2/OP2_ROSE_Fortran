SUBROUTINE writeOutputToFile(res, resSize)
INTEGER(kind=4) :: resSize
REAL(kind=8), DIMENSION(resSize) :: res
INTEGER(kind=4) :: OUT_FILE_ID = 20
INTEGER(kind=4) :: i
OPEN (UNIT=OUT_FILE_ID, FILE='result.txt') 

DO i = 1, resSize
  WRITE (OUT_FILE_ID, FMT=*) res(i)
END DO

CLOSE (UNIT=OUT_FILE_ID) 
END SUBROUTINE writeOutputToFile

SUBROUTINE writeRealDataToFile(dataIn, dataSize, filename)
INTEGER(kind=4) :: dataSize
REAL(kind=8), DIMENSION(dataSize) :: dataIn
character(len=20) :: filename
INTEGER(kind=4) :: OUT_FILE_ID = 30
INTEGER(kind=4) :: i
OPEN (UNIT=OUT_FILE_ID, FILE=filename) 

DO i = 1, dataSize
WRITE (OUT_FILE_ID, FMT=*) dataIn(i)
END DO

CLOSE (UNIT=OUT_FILE_ID) 
END SUBROUTINE writeRealDataToFile

SUBROUTINE writeIntDataToFile(dataIn, dataSize, filename)
INTEGER(kind=4) :: dataSize
INTEGER(kind=4), DIMENSION(dataSize) :: dataIn
character(len=20) :: filename
INTEGER(kind=4) :: OUT_FILE_ID = 30
INTEGER(kind=4) :: i
OPEN (UNIT=OUT_FILE_ID, FILE=filename) 

DO i = 1, dataSize
WRITE (OUT_FILE_ID, FMT=*) dataIn(i)
END DO

CLOSE (UNIT=OUT_FILE_ID) 
END SUBROUTINE writeIntDataToFile

