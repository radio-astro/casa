/*
 * AveragingVi2Factory.cc
 *
 *  Created on: Feb 25, 2013
 *      Author: jjacobs
 */


#include <synthesis/MSVis/AveragingVi2Factory.h>
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/MSVis/VisibilityIteratorImpl2.h>
#include <synthesis/MSVis/AveragingTvi2.h>
#include <cstdarg>
#include <synthesis/MSVis/UtilJ.h>

using namespace std;

namespace casa {
namespace vi {

AveragingParameters::AveragingParameters ()
: averagingInterval_p (0),
  chunkInterval_p (0),
  sortColumns_p (SortColumns ()),
  weightScaling_p (0)
{}

AveragingParameters::AveragingParameters (Double averagingInterval,
                                          Double chunkInterval,
                                          const SortColumns & sortColumns,
                                          WeightScaling * weightScalingForAveraging)
: averagingInterval_p (averagingInterval),
  chunkInterval_p (chunkInterval),
  sortColumns_p (sortColumns),
  weightScaling_p (weightScalingForAveraging)
{
    Assert (averagingInterval > 0);
    Assert (chunkInterval > 0);
    Assert (chunkInterval >= averagingInterval);
}

Double
AveragingParameters::getChunkInterval () const
{
    Assert (chunkInterval_p > 0);

    return chunkInterval_p;
}

Double
AveragingParameters::getAveragingInterval () const
{
    Assert (averagingInterval_p > 0);

    return averagingInterval_p;
}

const SortColumns &
AveragingParameters::getSortColumns () const
{
    return sortColumns_p;
}

WeightScaling *
AveragingParameters::getWeightScaling () const
{
    return weightScaling_p;
}

AveragingVi2Factory::AveragingVi2Factory (const AveragingParameters & parameters,
                                          MeasurementSet * ms)
{
    Block <const MeasurementSet *> mss (1, ms);

    initialize (parameters, mss);
}

AveragingVi2Factory::AveragingVi2Factory  (const AveragingParameters & parameters,
                                           MeasurementSet * ms1,
                                           MeasurementSet * ms2,
                                           ...)
{
    // Capture the first argument directly into the stl vector

    vector<MeasurementSet *> mssV;
    mssV.push_back (ms1);

    va_list args;

    va_start (args, ms2);

    MeasurementSet * ms = va_arg (args, MeasurementSet *);

    while (ms != 0){

        mssV.push_back (ms);

        ms = va_arg (args, MeasurementSet *);
    }

    // Convert the stl vector to the casa Vector.

    Block <const MeasurementSet *> mss (mssV.size());
    for (uInt i = 0; i < mssV.size(); i++){
        mss [i] = mssV [i];
    }

    // Now have the other overload do the actual work.

    initialize (parameters, mss);
}


AveragingVi2Factory::AveragingVi2Factory (const AveragingParameters & parameters,
                                          const Block<const MeasurementSet *> & mss)
{
    initialize (parameters, mss);
}

AveragingVi2Factory::~AveragingVi2Factory ()
{
}


void
AveragingVi2Factory::initialize (const AveragingParameters & parameters,
                                 const Block<const MeasurementSet *> & mss)
{
    parameters_p = parameters;
    mss_p = mss;
}


ViImplementation2 *
AveragingVi2Factory::createVi (VisibilityIterator2 * vi2) const
{

    // Make the chunk interval compatible with the averaging interval up rounding it up to
    // the nearest multiple of the averaging interval (e.g., chunkInterval 12 with averaging
    // interval of 5 is rounded to 15.

    Double chunkInterval = parameters_p.getChunkInterval ();
    Double chunkRatio = ceil (chunkInterval / parameters_p.getAveragingInterval ());
    chunkInterval = parameters_p.getAveragingInterval () * chunkRatio;

    // Create a simple VI implementation to perform the reading.

    VisibilityIteratorImpl2 * vii2 = new VisibilityIteratorImpl2 (vi2,
                                                                  mss_p,
                                                                  parameters_p.getSortColumns (),
                                                                  chunkInterval,
                                                                  VbPlain,
                                                                  False);

    vii2->setWeightScaling (parameters_p.getWeightScaling());

    AveragingTvi2 * averagingTvi2 = new AveragingTvi2 (vi2, vii2, parameters_p.getAveragingInterval ());

    return averagingTvi2;
}

} // end namesapce vi
} // end namespace casa
