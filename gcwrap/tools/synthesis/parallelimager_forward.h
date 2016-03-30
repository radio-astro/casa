// Include file for forward declarations

#include <stdcasa/StdCasa/CasacSupport.h>
#include <synthesis/ImagerObjects/MPIGlue.h>

namespace casa
{
	class ParallelImager;

	namespace parallelimager {
		// Wrapper for automatically freeing an MPI group handle upon
		// destruction of the wrapper.
		struct MPIGroupWrapper {
			MPI_Group handle;
			MPIGroupWrapper()
				: handle(MPI_GROUP_NULL) {}
			MPIGroupWrapper(MPI_Group g)
				: handle(g) {}
			~MPIGroupWrapper() {
				if (handle != MPI_GROUP_NULL)
					MPI_Group_free(&handle);
			}
		};

		// Wrapper for automatically freeing an MPI communicator handle upon
		// destruction of the wrapper.
		struct MPICommWrapper {
			MPI_Comm handle;
			MPICommWrapper()
				: handle(MPI_COMM_NULL) {}
			MPICommWrapper(MPI_Comm c)
				: handle(c) {}
			~MPICommWrapper() {
				if (handle != MPI_COMM_NULL
				    && handle != MPI_COMM_SELF
				    && handle != MPI_COMM_WORLD)
					MPI_Comm_free(&handle);
			}
		};
	};
};
