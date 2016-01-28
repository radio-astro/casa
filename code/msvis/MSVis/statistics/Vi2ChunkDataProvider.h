// -*- mode: c++ -*-
//# Copyright (C) 1996,1997,1998,1999,2000,2002,2003,2015
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//
// Base class of data providers based on StatsDataProvider, backed by a
// VisibilityIterator2 instances.
//
#ifndef MSVIS_STATISTICS_VI2_CHUNK_DATA_PROVIDER_H_
#define MSVIS_STATISTICS_VI2_CHUNK_DATA_PROVIDER_H_

#include <casacore/casa/aips.h>
#include <casacore/casa/Arrays/Array.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisBufferComponents2.h>
#include <msvis/MSVis/statistics/Vi2StatsFlagsIterator.h>
#include <msvis/MSVis/statistics/Vi2StatsWeightsIterator.h>
#include <msvis/MSVis/statistics/Vi2StatsSigmasIterator.h>
#include <msvis/MSVis/statistics/Vi2ChunkStatisticsIteratee.h>
#include <casacore/scimath/Mathematics/StatisticsAlgorithm.h>
#include <casacore/scimath/Mathematics/StatsDataProvider.h>
#include <memory>
#include <vector>

namespace casa {

//
// Data provider template class backed by VisibilityIterator2 instances. These
// data providers operate on a single MS column over all vi2 sub-chunks in the
// chunk selected when the data provider is instantiated. In other words, the
// data sample for statistics generated with a Vi2ChunkDataProvider instance is
// the data from a single column in a single vi2 chunk. It is intended that the
// user set up the VisibilityIterator2 appropriately to select the desired data
// sample for computing statistics. The user may then iterate through the
// VisibilityIterator2 chunks, calling reset() on the Vi2ChunkDataProvider
// instance before supplying the instance to the Statistics::setDataProvider()
// call to compute statistics for that chunk's data. In outline:
//
// Vi2ChunkDataProvider *dp; // given
// StatisticsAlgorithm statistics; // given
// for (dp->vi2->originChunks(); dp->vi2->moreChunks(); dp->vi2->nextChunk()) {
//  // Prepare the data provider
//  dp->vi2->origin();
//  dp->reset();
//  // Compute statistics for this chunk
//  statistics.setDataProvider(dp);
//  doStuff(statistics);  // do something with the statistics;
//                        // maybe call statistics.getStatistics()
// }
//
// The above pattern is encapsulated by the Vi2ChunkDataProvider::foreachChunk()
// method and Vi2ChunkStatisticsIteratee. The above can be implemented as
// follows (but with template parameters where needed):
//
// Vi2ChunkDataProvider *dp; // given
// StatisticsAlgorithm statistics; // given
// class DoStuff : public Vi2ChunkStatisticsIteratee {
//   ... // constructor, probably needs some sort of accumulator
//   void nextChunk(StatisticsAlgorithm stats, const * VisBuffer2 vb) {
//     stats.getStatistics()...;
//   }
// }
// DoStuff doStuff;
// dp->foreachChunk(statistics, doStuff);
//
// Note that the AccumType template parameter value of StatsDataProvider is
// derived from the DataIterator parameter value of this template, imposing a
// constraint on the classes that can be used for DataIterator.
//
template <class DataIterator, class MaskIterator, class WeightsIterator>
class Vi2ChunkDataProvider
	: public StatsDataProvider<typename DataIterator::AccumType,
	                           DataIterator,
	                           MaskIterator,
	                           WeightsIterator> {

public:
	// Define typedefs for some template parameters. This is primarily to
	// support instantiating "...Statistics" instances of the proper type given
	// only an instance of this type. [AccumType is an exception, in that it's
	// also needed by the (macro) definition of DataRanges.]
	typedef typename DataIterator::AccumType AccumType;
	typedef DataIterator DataIteratorType;
	typedef MaskIterator MaskIteratorType;
	typedef WeightsIterator WeightsIteratorType;

	Vi2ChunkDataProvider(
		vi::VisibilityIterator2 *vi2,
		vi::VisBufferComponent2 component,
		Bool omit_flagged_data,
		Bool use_data_weights)
		: vi2(vi2)
		, component(component)
		, use_data_weights(use_data_weights)
		, omit_flagged_data(omit_flagged_data) {

		// Attach the MS columns to vi2 by calling originChunks(). Not the most
		// direct method, but attaching the columns is required in many cases to
		// pass existsColumn() test.
		vi2->originChunks();
		if (!vi2->existsColumn(component))
			throw AipsError("Column is not present");
	}

	Vi2ChunkDataProvider(Vi2ChunkDataProvider&& other)
		: vi2(other.vi2)
		, component(other.component)
		, use_data_weights(other.use_data_weights)
		, omit_flagged_data(other.omit_flagged_data)
		, data_iterator(other.data_iterator)
		, weights_iterator(other.weights_iterator)
		, mask_iterator(other.mask_iterator) {
		other.vi2 = nullptr;
	}

	Vi2ChunkDataProvider& operator=(Vi2ChunkDataProvider&& other) {
		vi2 = other.vi2;
		component = other.component;
		use_data_weights = other.use_data_weights;
		omit_flagged_data = other.omit_flagged_data;
		data_iterator = other.data_iterator;
		weights_iterator = other.weights_iterator;
		mask_iterator = other.mask_iterator;
		other.vi2 = nullptr;
		return *this;
	}

	// Increment the data provider to the next sub-chunk.
	void operator++() {
		reset_iterators();
		vi2->next();
	}

	// Are there any sub-chunks left to provide?
	Bool atEnd() const {
		return !vi2->more();
	}

	// Take any actions necessary to finalize the provider. This will be called
	// when atEnd() returns True.
	void finalize() {};

	uInt64 getCount() {
		return getData().getCount();
	}

	// Get the current sub-chunk
	DataIterator getData() {
		if (!data_iterator)
			data_iterator =
				std::unique_ptr<DataIterator>(new DataIterator(dataArray()));
		return *data_iterator;
	}

	uInt getStride() {
		return 1;
	};

	Bool hasMask() const {
		return omit_flagged_data;
	};

	// Get the associated mask of the current sub-chunk. Only called if
	// hasMask() returns True.
	MaskIterator getMask() {
		if (!mask_iterator)
			mask_iterator = std::unique_ptr<MaskIterator>(
				new MaskIterator(vi2->getVisBuffer()));
		return *mask_iterator;
	};

	// Get the stride for the current mask (only called if hasMask() returns
	// True). Will always return 1 in this implementation.
	uInt getMaskStride() {
		return 1;
	};

	Bool hasWeights() const {
		return use_data_weights;
	};

	// Get the associated weights of the current sub-chunk. Only called if
	// hasWeights() returns True;
	WeightsIterator getWeights() {
		if (!weights_iterator)
			weights_iterator = std::unique_ptr<WeightsIterator>(
				new WeightsIterator(vi2->getVisBuffer()));
		return *weights_iterator;
	};

	Bool hasRanges() const {
		return false; // TODO: is this correct in all cases?
	};

	// Get the associated range(s) of the current sub-chunk. Only called if
	// hasRanges() returns True, which will never be the case in this
	// implementation.
	DataRanges
	getRanges() {
		DataRanges result;
		return result;
	};

	// If the associated data set has ranges, are these include (return True) or
	// exclude (return False) ranges?
	Bool isInclude() const {
		return false;
	};

	// Reset the provider to point to the first sub-chunk.
	void reset() {
		reset_iterators();
		vi2->origin();
	}

	std::unique_ptr<vi::VisibilityIterator2> vi2;

	void foreachChunk(
		StatisticsAlgorithm<AccumType,DataIteratorType,MaskIteratorType,WeightsIteratorType>& statistics,
		Vi2ChunkStatisticsIteratee<DataIterator,WeightsIterator,MaskIterator>& iteratee) {

		for (vi2->originChunks(); vi2->moreChunks(); vi2->nextChunk()) {
			reset();
			statistics.setDataProvider(this);
			iteratee.nextChunk(statistics, vi2->getVisBuffer());
		}
	}

protected:

	vi::VisBufferComponent2 component;

	std::unique_ptr<const DataIterator> data_iterator;

	const Bool use_data_weights;

	std::unique_ptr<const WeightsIterator> weights_iterator;

	const Bool omit_flagged_data;

	std::unique_ptr<const MaskIterator> mask_iterator;

private:

	void reset_iterators() {
		data_iterator.reset();
		weights_iterator.reset();
		mask_iterator.reset();
	}

	virtual const Array<typename DataIterator::DataType>& dataArray() = 0;
};

// Data provider template for row-based MS columns (i.e, not visibilities) using
// the 'weights' column for data weights. In most instances, you would not
// weight the data in these columns, but the Vi2ChunkDataProvider template
// requires that a WeightsIterator be provided.
template<class DataIterator>
using Vi2ChunkWeightsRowDataProvider =
	Vi2ChunkDataProvider<
	DataIterator,Vi2StatsFlagsRowIterator,Vi2StatsWeightsRowIterator>;

// Data provider template for row-based MS columns (i.e, not visibilities) using
// the 'sigma' column for data weights (appropriately transformed). In most
// instances, you would not weight the data in these columns, but the
// Vi2ChunkDataProvider template requires that a WeightsIterator be provided.
template<class DataIterator>
using Vi2ChunkSigmasRowDataProvider =
	Vi2ChunkDataProvider<
	DataIterator,Vi2StatsFlagsRowIterator,Vi2StatsSigmasRowIterator>;

// Data provider template for cube-based MS columns (i.e, the visibilities)
// using the 'weights' column for data weights.
template<class DataIterator>
using Vi2ChunkWeightsCubeDataProvider =
	Vi2ChunkDataProvider<
	DataIterator,Vi2StatsFlagsCubeIterator,Vi2StatsWeightsCubeIterator>;

// Data provider template for cube-based MS columns (i.e, the visibilities)
// using the 'sigma' column for data weights (appropriately transformed).
template<class DataIterator>
using Vi2ChunkSigmasCubeDataProvider =
	Vi2ChunkDataProvider<
	DataIterator,Vi2StatsFlagsCubeIterator,Vi2StatsSigmasCubeIterator>;

} // end namespace casa

#endif // MSVIS_STATISTICS_VI2_CHUNK_DATA_PROVIDER_H_
