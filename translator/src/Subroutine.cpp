#include <Subroutine.h>
#include <OP2CommonDefinitions.h>


SgExpression * 
Subroutine::getFortranKindOfOpDat ( SgType * opDatBaseType )
{
	using SageBuilder::buildIntVal;
	

	SgArrayType * isArrayType = isSgArrayType ( opDatBaseType );

	ROSE_ASSERT ( isArrayType != NULL );

	opDatBaseType = isArrayType->get_base_type ();

	SgExpression * opDatKindSize = opDatBaseType->get_type_kind ();

	/*
	 * ======================================================
	 * if the user hasn't specified a fortran kind, we have
	 * assume standard ones: integer(4) and real(4)
	 * ======================================================
	 */

	if ( opDatKindSize == NULL ) 
	{
		
		if ( isSgTypeInt ( opDatBaseType ) != NULL )
			opDatKindSize = buildIntVal ( FortranVariableDeafultKinds::DEFAULT_KIND_INT );
			
		if ( isSgTypeFloat ( opDatBaseType ) != NULL )
			opDatKindSize = buildIntVal ( FortranVariableDeafultKinds::DEFAULT_KIND_REAL );
			
		// default case
		if ( opDatKindSize == NULL )
			opDatKindSize = buildIntVal ( FortranVariableDeafultKinds::DEFAULT_KIND_REAL );
	}
	
	return opDatKindSize;
}

