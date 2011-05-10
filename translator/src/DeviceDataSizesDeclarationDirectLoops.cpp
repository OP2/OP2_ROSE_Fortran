#include <boost/lexical_cast.hpp>
#include <DeviceDataSizesDeclarationDirectLoops.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <CommonNamespaces.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */
void
DeviceDataSizesDeclarationDirectLoops::addFields ( ParallelLoop & parallelLoop,
																									 SgScopeStatement * moduleScope )
{
  using boost::lexical_cast;
  using SageBuilder::buildVariableDeclaration;
  using std::map;
  using std::string;
  using std::vector;
	
  for (unsigned int i = 1; i
			 <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      string const variableName = OP_DAT_ArgumentNames::OP_DAT_NamePrefix + lexical_cast <string> (i)
			+ OP_DAT_ArgumentNames::OP_DAT_SizeNameSuffix;
			
      SgVariableDeclaration * fieldDeclaration = buildVariableDeclaration (
																																					 variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
																																					 moduleScope);
			
      fieldDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
			
      deviceDatatypeStatement->get_definition ()->append_member (
																																 fieldDeclaration);
			
      OP_DAT_Sizes[i] = fieldDeclaration;
    }
  }
	
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

DeviceDataSizesDeclarationDirectLoops::DeviceDataSizesDeclarationDirectLoops ( ParallelLoop & parallelLoop, 
  std::string const & subroutineName, SgScopeStatement * moduleScope
	): DeviceDataSizesDeclaration ( parallelLoop, subroutineName, moduleScope )
{
	
}
