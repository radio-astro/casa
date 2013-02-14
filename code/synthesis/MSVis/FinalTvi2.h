//# VisibilityIterator.h: Step through the MeasurementEquation by visibility
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the Implied warranty of MERCHANTABILITY or
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
//# $Id: VisibilityIterator2.h,v 19.14 2006/02/28 04:48:58 mvoronko Exp $

#if ! defined (MSVIS_WritingVi2_H_121115_0950)
#define MSVIS_WritingVi2_H_121115_0950

#include <casa/aips.h>
#include <ms/MeasurementSets.h>
#include <synthesis/MSVis/TransformingVi2.h>
#include <synthesis/MSVis/ViColumns2.h>

#include <boost/noncopyable.hpp>
#include <map>
#include <vector>

namespace casa { //# NAMESPACE CASA - BEGIN

template <typename T> class Vector;

namespace vi {

//# forward decl

class VisBuffer2;

class ChannelSelector;
class ChannelSelectorCache;
typedef Vector<Vector <Slice> > ChannelSlicer;
class SpectralWindowChannelsCache;
class SpectralWindowChannels;
class SubtableColumns;


// <summary>
// VisibilityIterator2 iterates through one or more readonly MeasurementSets
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="MSIter">MSIter</linkto>
//   <li> <linkto class="MeasurementSet">MeasurementSet</linkto>
//   <li> <linkto class="VisSet">VisSet</linkto>
// </prerequisite>
//
// <etymology>
// The VisibilityIterator2 is a readonly iterator returning visibilities
// </etymology>
//
// <synopsis>
// VisibilityIterator2 provides iteration with various sort orders
// for one or more MSs. It has member functions to retrieve the fields
// commonly needed in synthesis calibration and imaging.
//
// One should use <linkto class="VisBuffer">VisBuffer</linkto>
// to access chunks of data.
// </synopsis>
//
// <example>
// <code>
// //
// </code>
// </example>
//
// <motivation>
// For imaging and calibration you need to access an MS in some consistent
// order (by field, spectralwindow, time interval etc.). This class provides
// that access.
// </motivation>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="1997/05/30">
//   <li> cleanup the currently dual interface for visibilities and flags
//   <li> sort out what to do with weights when interpolating
// </todo>

class FinalTvi2 : public TransformingVi2 {

public:

    FinalTvi2 (ViImplementation2 * inputVi,
               VisibilityIterator2 * vi,
               MeasurementSet & finalMs,
               Bool isWritable);

    // Destructor

    virtual ~FinalTvi2 ();

    //   +==================================+
    //   |                                  |
    //   | Iteration Control and Monitoring |
    //   |                                  |
    //   +==================================+


    virtual void origin ();
    virtual void next ();

    //   +=========================+
    //   |                         |
    //   | Subchunk Data Accessors |
    //   |                         |
    //   +=========================+


    //   +------------------------+
    //   |                        |
    //   | Angular Data Providers |
    //   |                        |
    //   +------------------------+


    //   +=========================+
    //   |                         |
    //   | Chunk and MS Level Data |
    //   |                         |
    //   +=========================+



    //   +-------------------+
    //   |                   |
    //   | Writeback Methods |
    //   |                   |
    //   +-------------------+

    // This method writes back any changed (dirty) components of the provided
    // VisBuffer and is the preferred method for writing data out.

    virtual void writeBackChanges (VisBuffer2 * vb);

    // Write/modify the flags in the data.
    // This will flag all channels in the original data that contributed to
    // the output channel in the case of channel averaging.
    // All polarizations have the same flag value.
    virtual void writeFlag (const Matrix<Bool> & flag);

    // Write/modify the flags in the data.
    // This writes the flags as found in the MS, Cube (npol,nchan,nrow),
    // where nrow is the number of rows in the current iteration (given by
    // nRow ()).
    virtual void writeFlag (const Cube<Bool> & flag);

    // Write/modify the flag row column; dimension Vector (nrow)
    virtual void writeFlagRow (const Vector<Bool> & rowflags);

    virtual void writeFlagCategory(const Array<Bool>& fc);

    // Write/modify the visibilities.
    // This is possibly only for a 'reference' MS which has a new DATA column.
    // The first axis of the matrix should equal the selected number of channels
    // in the original MS.
    // If the MS does not contain all polarizations, only the parallel
    // hand polarizations are used.
//    virtual void writeVisCorrected (const Matrix<CStokesVector> & visibilityStokes);
//    virtual void writeVisModel (const Matrix<CStokesVector> & visibilityStokes);
//    virtual void writeVisObserved (const Matrix<CStokesVector> & visibilityStokes);

    // Write/modify the visibilities
    // This writes the data as found in the MS, Cube (npol,nchan,nrow).
    virtual void writeVisCorrected (const Cube<Complex> & vis);
    virtual void writeVisModel (const Cube<Complex> & vis);
    virtual void writeVisObserved (const Cube<Complex> & vis);

    // Write/modify the weights
    virtual void writeWeight (const Vector<Float> & wt);

    // Write/modify the weightMat
    virtual void writeWeightMat (const Matrix<Float> & wtmat);

    // Write/modify the weightSpectrum
    virtual void writeWeightSpectrum (const Cube<Float> & wtsp);

    // Write/modify the Sigma
    virtual void writeSigma (const Vector<Float> & sig);

    // Write/modify the ncorr x nrow SigmaMat.
    virtual void writeSigmaMat (const Matrix<Float> & sigmat);

    // Write the information needed to generate on-the-fly model visibilities.

    virtual void writeModel(const RecordInterface& rec, Bool iscomponentlist=True,
                            Bool incremental=False);

protected:

    void configureNewSubchunk ();

    void writeDataValues (MeasurementSet & ms, const RefRows & rows);
    void writeKeyValues (MeasurementSet & ms, const RefRows & rows);
    void writeMiscellaneousValues (MeasurementSet & ms, const RefRows & rows);

private:

    ViColumns2     columns_p;
    Bool           columnsAttached_p;
    MeasurementSet ms_p;

};

} // end namespace vi

} //# NAMESPACE CASA - END

#endif // ! defined (MSVIS_WritingVi2_H_121115_0950)


