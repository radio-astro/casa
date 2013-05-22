
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/MSVis/ViImplementation2.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>
#include <casa/Quanta/MVTime.h>
#include <ms/MeasurementSets/MSDerivedValues.h>
#include <casa/Arrays.h>

#include <algorithm>
using namespace std;

namespace casa {

namespace vi {

void
ViImplementation2::azel0Calculate (Double time, MSDerivedValues & msd,
        MDirection & azel0, const MEpoch & mEpoch0)
{
    // Refactored into a static method to allow VisBufferAsync to use

    MEpoch mEpoch = mEpoch0;

    mEpoch.set (MVEpoch (Quantity (time, "s")));

    msd.setEpoch (mEpoch);

    msd.setAntenna (-1);

    azel0 = msd.azel ();

    if (aips_debug) {
        cout << "At time: " << MVTime (mEpoch.getValue ()) <<
             " AzEl = " << azel0.getAngle ("deg") << endl;
    }
}

void
ViImplementation2::azelCalculate (Double time, MSDerivedValues & msd, Vector<MDirection> & azel,
                                        Int nAnt, const MEpoch & mEpoch0)
{
    // Refactored into a static method to allow VisBufferAsync to use

    MEpoch mEpoch = mEpoch0;

    mEpoch.set (MVEpoch (Quantity (time, "s")));

    msd.setEpoch (mEpoch);

    // Calculate az/el for all antennas.

    azel.resize (nAnt);

    for (Int iant = 0; iant < nAnt; iant++) {

        msd.setAntenna (iant);
        azel (iant) = msd.azel ();

        if (aips_debug) {

            if (iant == 0){
                cout << "Antenna " << iant << " at time: " << MVTime (mEpoch.getValue ()) <<
                     " has AzEl = " << azel (iant).getAngle ("deg") << endl;
            }
        }
    }
}


Vector<Float>
ViImplementation2::feed_paCalculate (Double time, MSDerivedValues & msd,
                                           Int nAntennas, const MEpoch & mEpoch0,
                                            const Vector<Float> & receptor0Angle)
{
    MEpoch mEpoch = mEpoch0;

    mEpoch.set (MVEpoch (Quantity (time, "s")));

    msd.setEpoch (mEpoch);

    // Calculate pa for all antennas.

    Vector<Float> feedpa (nAntennas);

    for (Int iant = 0; iant < nAntennas; iant++) {

        msd.setAntenna (iant);
        feedpa (iant) = msd.parAngle ();

        // add angle for receptor 0

        feedpa (iant) += receptor0Angle (iant);

        if (aips_debug && iant == 0) {

            cout << "Antenna " << iant << " at time: " << MVTime (mEpoch.getValue ()) <<
                    " has PA = " << feedpa (iant) * 57.28 << endl;
        }
    }

    return feedpa;
}

Double
ViImplementation2::hourangCalculate (Double time, MSDerivedValues & msd, const MEpoch & mEpoch0)
{
    MEpoch mEpoch = mEpoch0;

    mEpoch.set (MVEpoch (Quantity (time, "s")));

    msd.setEpoch (mEpoch);

    msd.setAntenna (-1);

    Double hourang = msd.hourAngle ();

    return hourang;
}

Float
ViImplementation2::parang0Calculate (Double time, MSDerivedValues & msd, const MEpoch & mEpoch0)
{
    MEpoch mEpoch = mEpoch0;

    mEpoch.set (MVEpoch (Quantity (time, "s")));
    msd.setEpoch (mEpoch);

    // Calculate pa for all antennas.
    msd.setAntenna (-1);
    Float parang0 = msd.parAngle ();

    if (aips_debug)
        cout << "At time: " << MVTime (mEpoch.getValue ()) <<
             " PA = " << parang0 * 57.28 << " deg" << endl;

    return parang0;
}

Vector<Float>
ViImplementation2::parangCalculate (Double time, MSDerivedValues & msd, int nAntennas, const MEpoch mEpoch0)
{
    MEpoch mEpoch = mEpoch0;
    mEpoch.set (MVEpoch (Quantity (time, "s")));

    msd.setEpoch (mEpoch);

    // Calculate pa for all antennas.

    Vector<Float> parang (nAntennas);

    for (Int iant = 0; iant < nAntennas; iant++) {

        msd.setAntenna (iant);
        parang (iant) = msd.parAngle ();

        if (aips_debug && iant == 0) {
            cout << "Antenna " << iant << " at time: " << MVTime (mEpoch.getValue ()) <<
                 " has PA = " << parang (iant) * 57.28 << endl;
        }
    }

    return parang;
}

void
ViImplementation2::doWeightScaling (Bool hasWeightScaling,
                                    WeightScaling * scaling,
                                    const Array<Float>& unscaled,
                                    Array<Float>& scaled)
{
    if (hasWeightScaling){

        // Apply the scaling function to each of the weights.

        scaled.resize (unscaled.shape());

        std::transform <Array<Float>::const_contiter,
                        Array<Float>::contiter,
                        WeightScaling &>
            (unscaled.cbegin(), unscaled.cend(), scaled.cbegin(), * scaling);

    }
    else{

        // No scaling function so simply copy raw weights to scaled.

        scaled.assign (unscaled);
    }
}


} // end namespace vi

} // end namespace casa
