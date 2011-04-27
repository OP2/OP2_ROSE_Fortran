/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models a variable declaration: it includes
 * fields for the sizes of data passed to the device in the
 * specific case of direct loops.
 * These sizes are needed in Fortran to prevent segmentation
 * faults in data accessing.
 *
 * These sizes must be passed in a struct-like data
 * structure as the number of kernel parameters can be become
 * unwieldly. In these cases, the code might not compile
 * with current-generation CUDA compilers due to the number of
 * parameters, so the solution is to instead pack them into
 * a struct.
 *
 * This class extends the generic DeviceDataSizesDeclaration
 * by replacing the implementation of the virtual function
 * addFields with the correct one for direct loops. In fact,
 * we don't need to declare all plan-related variables, but just
 * input application data to the CUDA kernel, and consequently
 * we don't need their sizes.
 */

#ifndef DEVICE_DATA_SIZES_DECLARATION_DIRECT_LOOPS_H
#define DEVICE_DATA_SIZES_DECLARATION_DIRECT_LOOPS_H


#include <DeviceDataSizesDeclaration.h>

class DeviceDataSizesDeclarationDirectLoops: public DeviceDataSizesDeclaration
{

	private:
	
		virtual void
		addFields (ParallelLoop & parallelLoop, SgScopeStatement * moduleScope);

	public:
	
		DeviceDataSizesDeclarationDirectLoops ( ParallelLoop & parallelLoop,
				std::string const & subroutineName, SgScopeStatement * moduleScope ); 

	/*
	 * ======================================================
	 * This function calls the coorect addFields subclass
	 * implementation
	 * ======================================================
	 */
	
	void
	initialise ( ParallelLoop & parallelLoop, SgScopeStatement * moduleScope ) 
	{ 
		addFields ( parallelLoop, moduleScope );
	}
	
};


#endif
