//# tSubImage.cc: Test program for class SubImage
//# Copyright (C) 1998,1999,2000,2001,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: tSubImage.cc 20567 2009-04-09 23:12:39Z gervandiepen $

#ifndef IMAGEANALYSIS_IMAGESTATSCALCULATOR_H
#define IMAGEANALYSIS_IMAGESTATSCALCULATOR_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <images/Images/ImageStatistics.h>

#include <casa/namespace.h>

#include <memory>
#include <tr1/memory>

namespace casa {

class ImageStatsCalculator: public ImageTask<Float> {
    // <summary>
    // Top level class used for statistics calculations
    // </summary>

    // <reviewed reviewer="" date="" tests="" demos="">
    // </reviewed>

    // <prerequisite>
    // </prerequisite>

    // <etymology>
    // Image statistics calculator
    // </etymology>

    // <synopsis>
    // Top level class used for statistics calculations
    // </synopsis>

public:

   	ImageStatsCalculator(
   		const SPCIIF image,
    	const Record *const &regionPtr,
    	const String& maskInp, Bool beVerboseDuringConstruction=False
    );

    ~ImageStatsCalculator();

    Record calculate();

    inline String getClass() const {return _class;}

    // I suspect this is not useful any longer
    inline void setPlotStats(const Vector<String>& ps) {_plotStats.assign(ps); }

    // Set range of pixel values to include in the calculation. Should be a two element
    // Vector
    inline void setIncludePix(const Vector<Float>& inc) {_includepix.assign(inc);}

    // Set range of pixel values to exclude from the calculation. Should be a two element
    // Vector
    inline void setExcludePix(const Vector<Float>& exc) {_excludepix.assign(exc);}

    // I doubt this is useful any longer
    inline void setPlotter(const String& p) {_plotter = p;}

    // I doubt this is useful any longer
    inline void setNXNY(const Int x, const Int y) {
    	_nx = x;
    	_ny = y;
    }

    // List stats to logger? If you want no logging you should set this to False in addition to
    // calling setVerbosity()
    inline void setList(const Bool l) {_list = l;}

    inline void setForce(const Bool f) {_force = f;}

    inline void setDisk(const Bool d) {_disk = d;}

    inline void setRobust(const Bool r) {_robust = r;}

    inline void setVerbose(const Bool v) {_verbose = v;}

    inline void setAxes(const Vector<Int>& axes) {_axes.assign(axes);}

    // moved from ImageAnalysis
    // if messageStore != 0, log messages, stripped of time stampe and priority, will also
    // be placed in this parameter and returned to caller for eg logging to file.
    Record statistics(
    	 vector<String> *const &messageStore=0
    );

protected:

   	inline CasacRegionManager::StokesControl _getStokesControl() const {
   		return CasacRegionManager::USE_ALL_STOKES;
   	}

    inline vector<OutputDestinationChecker::OutputStruct> _getOutputStruct() {
    	return vector<OutputDestinationChecker::OutputStruct>(0);
    }

    inline vector<Coordinate::Type> _getNecessaryCoordinates() const {
    	return vector<Coordinate::Type>(0);
    }

    inline Bool _supportsMultipleRegions() const {return True;}

private:
    std::auto_ptr<ImageStatistics<Float> > _statistics;
    std::auto_ptr<ImageRegion> _oldStatsRegion, _oldStatsMask;
    Bool _oldStatsStorageForce;
    Vector<Int> _axes;
    Vector<String> _plotStats;
    Vector<Float> _includepix, _excludepix;
    String _plotter;
    Int _nx, _ny;
    Bool _list, _force, _disk, _robust, _verbose;

    static const String _class;

    // moved from ImageAnalysis
    // See if the combination of the 'region' and 'mask' ImageRegions have changed
    static Bool _haveRegionsChanged (
    	ImageRegion* pNewRegionRegion,
    	ImageRegion* pNewMaskRegion,
    	ImageRegion* pOldRegionRegion,
    	ImageRegion* pOldMaskRegion
    );

    void _reportDetailedStats(
    	const std::tr1::shared_ptr<const ImageInterface<Float> > tempIm,
    	const Record& retval
    );
};
}

#endif
