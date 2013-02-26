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

#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <casa/namespace.h>

#include <memory>

namespace casa {

class ImageStatsCalculator: public ImageTask {
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
   		ImageAnalysis *const &ia,
    	const String& region, const Record *const &regionPtr,
    	const String& box, const String& chanInp,
    	const String& stokes, const String& maskInp, Bool beVerboseDuringConstruction
    );

    ~ImageStatsCalculator();

    Record calculate();

    inline String getClass() const {return _class;}

    inline void setPlotStats(const Vector<String>& ps) {_plotStats = ps; }

    inline void setIncludePix(const Vector<Float>& inc) {_includepix = inc;}

    inline void setExcludePix(const Vector<Float>& exc) {_excludepix = exc;}

    inline void setPlotter(const String& p) {_plotter = p;}

    inline void setNXNY(const Int x, const Int y) {
    	_nx = x;
    	_ny = y;
    }

    inline void setList(const Bool l) {_list = l;}

    inline void setForce(const Bool f) {_force = f;}

    inline void setDisk(const Bool d) {_disk = d;}

    inline void setRobust(const Bool r) {_robust = r;}

    inline void setVerbose(const Bool v) {_verbose = v;}

    inline void setAxes(Vector<Int>& axes) {_axes = axes;}

protected:

   	inline CasacRegionManager::StokesControl _getStokesControl() const {
   		return CasacRegionManager::USE_ALL_STOKES;
   	}

    inline vector<ImageInputProcessor::OutputStruct> _getOutputStruct() {
    	return vector<ImageInputProcessor::OutputStruct>(0);
    }

    inline vector<Coordinate::Type> _getNecessaryCoordinates() const {
    	return vector<Coordinate::Type>(0);
    }

    inline Bool _supportsMultipleRegions() {return True;}

private:
    ImageAnalysis *const _ia;
    Vector<Int> _axes;
    Vector<String> _plotStats;
    Vector<Float> _includepix, _excludepix;
    String _plotter;
    Int _nx, _ny;
    Bool _list, _force, _disk, _robust, _verbose;

    static const String _class;

};
}

#endif
