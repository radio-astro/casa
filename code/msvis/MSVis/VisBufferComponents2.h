#if ! defined (MSVIS_VisBufferComponents2_H)
#define MSVIS_VisBufferComponents2_H

#include <casa/BasicSL/String.h>
#include <set>

namespace casa {

namespace vi {

typedef enum {

    Unknown=-1,

    AllBeamOffsetsZero,  // VisBufferAsync use only
    AntennaMounts,       // VisBufferAsync use only
    Antenna1,
    Antenna2,
    ArrayId,
    BeamOffsets,         // VisBufferAsync use only
    CorrType,
    DataDescriptionIds,
    Direction1,
    Direction2,
    Exposure,
    Feed1,
    FeedPa1,
    Feed2,
    FeedPa2,
    FieldId,
    FlagCategory,
    FlagCube,
    FlagRow,
    FloatData,
    ImagingWeight,
    JonesC,
    NAntennas,
    NChannels,
    NCorrelations,
    NRows,
    ObservationId,
    PhaseCenter,
    PolFrame,
    PolarizationId,
    ProcessorId,
    ReceptorAngles,       // VisBufferAsync use only
    RowIds,
    Scan,
    Sigma,
    SpectralWindows,
    StateId,
    Time,
    TimeCentroid,
    TimeInterval,
    Weight,
    WeightScaled,
    WeightSpectrum,
    SigmaSpectrum,
    WeightSpectrumScaled,
    Uvw,
    VisibilityCorrected,
    VisibilityModel,
    VisibilityObserved,
    VisibilityCubeCorrected,
    VisibilityCubeModel,
    VisibilityCubeObserved,
    VisibilityCubeFloat,

    N_VisBufferComponents2 // must be last

} VisBufferComponent2;


// <summary>
//
// This class encapsulates an enum with values corresponding to the various
// data components accessible via a VisBuffer.  Its current usage is to
// specify the VisBuffer components to be prefetched when asynchronous I/O
// is in use or to specify the VisBuffer components that have been modified
// when a Visibility Processing Framework bucket-brigade is operating on
// a VisBuffer.  This implementation should be replaced by a C++11 when
// the standard matures a bit.
//
// <prerequisite>
//   <li> <linkto class="VisBuffer">VisBuffer</linkto>
// </prerequisite>
//
// </summary>
//
// <synopsis>
//
// </synopsis>
// <example>
//
// <code>
//
// </code>
// </example>
//
class VisBufferComponents2 {

public:


    typedef std::set<VisBufferComponent2> Set;
    typedef Set::const_iterator const_iterator;

    VisBufferComponents2 operator+ (const VisBufferComponents2 & other) const;
    VisBufferComponents2 & operator+= (const VisBufferComponents2 & other);
    VisBufferComponents2 & operator+= (VisBufferComponent2 componentToAdd);

    const_iterator begin () const;
    Bool contains (VisBufferComponent2 component) const;
    Bool empty () const;
    const_iterator end () const;
    String toString () const;

    static VisBufferComponents2 all ();
    static VisBufferComponents2 exceptThese (VisBufferComponent2 component, ...);
    static String name (Int id);
    static VisBufferComponents2 none ();
    static VisBufferComponents2 singleton (VisBufferComponent2 component);
    static VisBufferComponents2 these (VisBufferComponent2 component, ...);


protected:

private:

    Set set_p;

    static const VisBufferComponents2 all_p;

    static VisBufferComponents2 initializeAll ();

};

} // end namespace vi


} // end namespace casa
#endif // ! defined (MSVIS_VisBufferComponents2_H)

