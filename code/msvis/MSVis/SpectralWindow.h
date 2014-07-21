/*
 * SpectralWindow.h
 *
 *  Created on: Feb 15, 2013
 *      Author: jjacobs
 */

#ifndef MSVIS_SPECTRALWINDOW_H_
#define MSVIS_SPECTRALWINDOW_H_

#include <casa/aipstype.h>
#include <casa/BasicSL.h>
#include <vector>

namespace casa {

template<typename T>
class ArrayColumn;
class MeasurementSet;

namespace ms {

  //#warning "Needs battery of low-level tests; also needs write functionality"

class SpectralChannel {

public:

    SpectralChannel (Double frequency, Double width,
                     Double effectiveBandwidth, Double resolution)
    : effectiveBandwidth_p (effectiveBandwidth),
      frequency_p (frequency),
      resolution_p (resolution),
      width_p (width)
    {}

    Double effectiveBandwidth () const;
    Double frequency () const;
    Double resolution () const;
    Double width () const;

private:

    Double effectiveBandwidth_p;
    Double frequency_p;
    Double resolution_p;
    Double width_p;

};

class SpectralWindow {

public:

    typedef std::vector<SpectralChannel> Channels;
    typedef Channels::const_iterator const_iterator;

    SpectralWindow (const ROMSSpWindowColumns & columns, Int spectralWindowId);

    const_iterator begin () const;
    bool empty () const;
    const_iterator end () const;
    const SpectralChannel & get (Int i) const;

    Int id () const;
    Bool isFlagged () const;
    Int frequencyGroup () const;
    String frequencyGroupName () const;
    Int frequencyMeasureReference () const;
    Int ifConversionChain () const;
    String name () const;
    int nChannels () const;
    int netSideband () const;
    Double totalBandwidth () const;
    Double referenceFrequency () const;

protected:

    void fillArrays(const ROMSSpWindowColumns & columns);
    Vector<Double> getArray (const ArrayColumn<Double> & arrayColumn);
    void fillScalars (const ROMSSpWindowColumns & columns);
    template<typename T>
    T getScalar (const ScalarColumn<T> & column);

private:

    Channels channels_p;
    Bool flagged_p;
    Int frequencyGroup_p;
    String frequencyGroupName_p;
    Int frequencyMeasureReference_p;
    Int id_p;
    Int ifConversionChain_p;
    String name_p;
    int nChannels_p;
    int netSideband_p;
    Double totalBandwidth_p;
    Double referenceFrequency_p;
};

class SpectralWindows {

public:

    typedef std::vector<SpectralWindow> Windows;
    typedef Windows::const_iterator const_iterator;

    SpectralWindows (const MeasurementSet * ms);

    const_iterator begin () const;
    bool empty () const;
    const_iterator end () const;
    const SpectralWindow & get (Int id) const;
    size_t size () const;

protected:

private:

    Windows windows_p;
};

} // end namespace vi
} // end namespace casa



#endif /* SPECTRALWINDOW_H_ */
