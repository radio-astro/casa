/***
 * Framework independent implementation file for imager...
 *
 * Implement the imager component here.
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE!

 ***/

#include <casa/Containers/Record.h>
#include <casa/Arrays/Vector.h>

#include <synthesis/ImagerObjects/ParallelImagerFactory.h>

#include <parallelimager_cmpt.h>

#include <memory>
#include <algorithm>

using namespace casacore;
using namespace casa;

using namespace casacore;
namespace casac {

parallelimager::parallelimager()
{
}

casac::record *
parallelimager::clean(
	const std::vector<int> &ranks,
	const casac::record& clean_params,
	const casac::record& selection_params,
	const casac::record& image_params,
	const casac::record& grid_params,
	const casac::record& weight_params,
	const casac::record& normalization_params,
	const casac::record& deconvolution_params,
	const casac::record& iteration_params)
{
	// The 'ranks' argument supplies the list of ranks that will be part of the
	// imager. If the rank of this process in MPI_COMM_WORLD is not in the given
	// list, we return an empty Record immediately.
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	if (std::find(ranks.begin(), ranks.end(), world_rank) != ranks.end()) {
		// Starting from MPI_COMM_WORLD, create a communicator solely for the
		// imager tool here.
		casa::parallelimager::MPIGroupWrapper world_group;
		casa::parallelimager::MPIGroupWrapper task_group;
		casa::parallelimager::MPICommWrapper task_comm;
		// The calls using MPI_COMM_WORLD below are safe only when no other
		// threads in the process access that communicator...which is guaranteed
		// by the CASA MPI framework.
		MPI_Comm_group(MPI_COMM_WORLD, &world_group.handle);
		MPI_Group_incl(world_group.handle, ranks.size(),
		               const_cast<int *>(ranks.data()), &task_group.handle);
		MPI_Comm_group_create(MPI_COMM_WORLD, task_group.handle, 0,
		                      &task_comm.handle);
		std::unique_ptr<ParallelImager> imager(
			ParallelImagerFactory::make(
				task_comm.handle,
				*toRecord(clean_params),
				*toRecord(selection_params),
				*toRecord(image_params),
				*toRecord(grid_params),
				*toRecord(weight_params),
				*toRecord(normalization_params),
				*toRecord(deconvolution_params),
				*toRecord(iteration_params)));
		// All MPI group and communicator handles created in this block, as well
		// as the ParallelImager instance (and consequently its own
		// communicators...which are not those created here) should be freed on
		// exit from this block.
		return fromRecord(imager->clean());
	} else {
		Record empty;
		return fromRecord(empty);
	}
}

parallelimager::~parallelimager()
{
}

} // casac namespace
