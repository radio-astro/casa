/*
 * AveragingViFactory.h
 *
 *  Created on: Feb 25, 2013
 *      Author: jjacobs
 */

#ifndef MSVIS_AVERAGINGVIFACTORY_H_
#define MSVIS_AVERAGINGVIFACTORY_H_

#include <synthesis/MSVis/VisibilityIterator2.h>

namespace casa {

class MeasurementSet2;

namespace vi {

class AveragingParameters {

public:

    AveragingParameters ();

    AveragingParameters (Double averagingInterval,
                         Double chunkInterval,
                         const SortColumns & sortColumns = SortColumns (),
                         WeightScaling * weightScalingForAveraging = 0);

    Double getChunkInterval () const;
    Double getAveragingInterval () const;
    const SortColumns & getSortColumns () const;
    WeightScaling * getWeightScaling () const;

private:

    Double averagingInterval_p;
    Double chunkInterval_p;
    SortColumns sortColumns_p;
    WeightScaling * weightScaling_p;
};

// The AveragingVi2Factory is used to initialize a VisibilityIterator2 so that
// it performs time averaging.  The parameters are fairly straightforward:
//
// averagingInterval - the time period to average over
// chunkInterval - the time interval of an averaged chunk
// sortColumns - the usual set of sort columns.  The sort columns also determine
//               the boundaries of a chunk.  If a column is present in the sort
//               columns, then there will be no averaging across values with different
//               values of that column (e.g., scan_number, array_id, etc.).  This is
//               because a chunk is defined to be the set of records having the same
//               values for all of the sort columns except for time.  The time column
//               is used to select rows having the same sort-column values but within
//               a specified interval of time.

class AveragingVi2Factory : public ViFactory {

public:

    AveragingVi2Factory (const AveragingParameters & parameters,
                         MeasurementSet * ms);

    AveragingVi2Factory (const AveragingParameters & parameters,
                         MeasurementSet * ms1,
                         MeasurementSet * ms2,
                         ...);
    // use 0 to mark end of MSs

    AveragingVi2Factory (const AveragingParameters & parameters,
                         const Block<const MeasurementSet *> & mss);

    ~AveragingVi2Factory ();

protected:

    virtual ViImplementation2 * createVi (VisibilityIterator2 *) const;

private:

    void initialize (const AveragingParameters & parameters,
                     const Block<const MeasurementSet *> & mss);

    Block<const MeasurementSet *> mss_p;
    AveragingParameters parameters_p;
};

} // end namesapce vi
} // end namespace casa


#endif /* MSVIS_AVERAGINGVIFACTORY_H_ */
