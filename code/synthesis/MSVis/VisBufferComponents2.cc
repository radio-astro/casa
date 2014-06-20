/*
 * VisBufferComponents.cc
 *
 *  Created on: Aug 20, 2012
 *      Author: jjacobs
 */

#include <synthesis/MSVis/UtilJ.h>
#include <synthesis/MSVis/VisBufferComponents2.h>
#include <map>

using namespace std;

namespace casa {

namespace vi {

const VisBufferComponents2 VisBufferComponents2::all_p = initializeAll ();

VisBufferComponents2
VisBufferComponents2::operator+ (const VisBufferComponents2 & other) const
{
    VisBufferComponents2 result = * this;

    result.set_p.insert (other.begin(), other.end());

    return result;
}

VisBufferComponents2 &
VisBufferComponents2::operator+= (const VisBufferComponents2 & other)
{
    if (& other != this){

        set_p.insert (other.set_p.begin(), other.set_p.end());
    }

    return * this;
}

VisBufferComponents2 &
VisBufferComponents2::operator+= (VisBufferComponent2 componentToAdd)
{
    set_p.insert (componentToAdd);

    return * this;
}


VisBufferComponents2
VisBufferComponents2::all ()
{
    return all_p;
}

VisBufferComponents2::const_iterator
VisBufferComponents2::begin () const
{
    return set_p.begin();
}

Bool
VisBufferComponents2::contains (VisBufferComponent2 component) const
{
    return utilj::containsKey (component, set_p);
}

Bool
VisBufferComponents2::empty () const
{
    return set_p.empty();
}

VisBufferComponents2::const_iterator
VisBufferComponents2::end () const
{
    return set_p.end();
}

VisBufferComponents2
VisBufferComponents2::exceptThese (VisBufferComponent2 component, ...)
{
    va_list vaList;

    va_start (vaList, component);

    VisBufferComponent2 c = component;
    VisBufferComponents2 dirtyComponents = all();

    while (c != Unknown){

        //ThrowIf (! all().contains (c), "Not a writable VB component: " + String::toString (c));

        dirtyComponents.set_p.erase (c);
        c = (VisBufferComponent2) va_arg (vaList, Int);
    }

    va_end (vaList);

    return dirtyComponents;

}

VisBufferComponents2
VisBufferComponents2::initializeAll ()
{

    VisBufferComponents2 all;

//    VisBufferComponent2
//    writableComponents [] = {Corrected,
//                             CorrectedCube,
//                             Flag,
//                             FlagCube,
//                             FlagRow,
//                             Model,
//                             ModelCube,
//                             Observed,
//                             ObservedCube,
//                             Sigma,
//                             SigmaMat,
//                             Weight,
//                             WeightMat,
//                             Unknown};

    for (Int i = 0; i < N_VisBufferComponents2; i++){

        all.set_p.insert (static_cast<VisBufferComponent2> (i));
    }

    return all;
}


String
VisBufferComponents2::name (Int id)
{
    assert (id >= 0 && id < N_VisBufferComponents2);

    // This method is called only occasionally for debugging so at this time a
    // brute-force implmentation is acceptable.

    map<Int,String> names;

    names [AllBeamOffsetsZero] = "AllBeamOffsetsZero";
    names [AntennaMounts] = "AntennaMounts";
    names [Antenna1] = "Antenna1";
    names [Antenna2] = "Antenna2";
    names [ArrayId] = "ArrayId";
    names [BeamOffsets] = "BeamOffsets";
    names [CorrType] = "CorrType";
    names [DataDescriptionIds] = "DataDescriptionIds";
    names [Direction1] = "Direction1";
    names [Direction2] = "Direction2";
    names [Exposure] = "Exposure";
    names [Feed1] = "Feed1";
    names [FeedPa1] = "FeedPa1";
    names [Feed2] = "Feed2";
    names [FeedPa2] = "FeedPa2";
    names [FieldId] = "FieldId";
    //names [Flag] = "Flag";
    names [FlagCategory] = "FlagCategory";
    names [FlagCube] = "FlagCube";
    names [FlagRow] = "FlagRow";
    names [FloatData] = "FloatData";
    names [ImagingWeight] = "ImagingWeight";
    names [JonesC] = "JonesC";
    names [NAntennas] = "NAntennas";
    names [NChannels] = "NChannels";
    names [NCorrelations] = "NCorrelations";
    names [NRows] = "NRows";
    names [ObservationId] = "ObservationId";
    names [PhaseCenter] = "PhaseCenter";
    names [PolFrame] = "PolFrame";
    names [PolarizationId] = "PolarizationId";
    names [ProcessorId] = "ProcessorId";
    names [ReceptorAngles] = "ReceptorAngles";
    names [RowIds] = "RowIds";
    names [Scan] = "Scan";
    names [Sigma] = "Sigma";
    names [SpectralWindows] = "SpectralWindows";
    names [StateId] = "StateId";
    names [Time] = "Time";
    names [TimeCentroid] = "TimeCentroid";
    names [TimeInterval] = "TimeInterval";
    names [Weight] = "Weight";
    names [WeightScaled] = "WeightScaled";
    names [WeightSpectrum] = "WeightSpectrum";
    names [SigmaSpectrum] = "SigmaSpectrum";
    names [WeightSpectrumScaled] = "WeightSpectrumScaled";
    names [Uvw] = "Uvw";
    names [VisibilityCorrected] = "VisibilityCorrected";
    names [VisibilityModel] = "VisibilityModel";
    names [VisibilityObserved] = "VisibilityObserved";
    names [VisibilityCubeCorrected] = "VisibilityCubeCorrected";
    names [VisibilityCubeModel] = "VisibilityCubeModel";
    names [VisibilityCubeObserved] = "VisibilityCubeObserved";

    assert (names.size() == N_VisBufferComponents2);

    return names [id];
}

VisBufferComponents2
VisBufferComponents2::none ()
{
    return VisBufferComponents2 ();
}

VisBufferComponents2
VisBufferComponents2::singleton (VisBufferComponent2 component)
{
    VisBufferComponents2 result;
    result.set_p.insert (component);

    return result;
}

VisBufferComponents2
VisBufferComponents2::these (VisBufferComponent2 component, ...)
{
    va_list vaList;

    va_start (vaList, component);

    VisBufferComponent2 c = component;
    VisBufferComponents2 dirtyComponents;

    while (c != Unknown){

        //ThrowIf (! all().contains (c), "Not a writable VB component: " + String::toString (c));

        dirtyComponents.set_p.insert (c);
        c = (VisBufferComponent2 ) va_arg (vaList, Int);
    }

    va_end (vaList);

    return dirtyComponents;
}

String
VisBufferComponents2::toString () const
{
    String result = "{";

    for (const_iterator i = begin(); i != end(); i++){
        result += name (*i) + ",";
    }

    * result.rbegin() = '}'; // replace last "," with "}"

    return result;
}


} // end namespace vi

} // end namespace casa
