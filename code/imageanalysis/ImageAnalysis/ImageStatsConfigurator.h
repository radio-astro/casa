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

#ifndef IMAGEANALYSIS_IMAGESTATSCONFIGURATOR_H
#define IMAGEANALYSIS_IMAGESTATSCONFIGURATOR_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <images/Images/ImageStatistics.h>
#include <casa/namespace.h>

#include <memory>

namespace casacore{

template <class T> class CountedPtr;
}

namespace casa {

class C11Timer;

class ImageStatsConfigurator: public ImageTask<casacore::Float> {
    // <summary>
    // This adds configuration methods for statistics classes.
    // </summary>

    // <reviewed reviewer="" date="" tests="" demos="">
    // </reviewed>

    // <prerequisite>
    // </prerequisite>

    // <etymology>
    // Image statistics configuration
    // </etymology>

    // <synopsis>
    // This adds configuration methods for statistics classes.
    // </synopsis>

public:

    enum PreferredClassicalAlgorithm {
        // old algorithm
        TILED_APPLY,
        // new algorithm
        STATS_FRAMEWORK,
        // decide based on size and number of steps needed for
        // stats
        AUTO
    };

    ImageStatsConfigurator() = delete;

    ~ImageStatsConfigurator();

    void configureChauvenet(casacore::Double zscore, casacore::Int maxIterations);

    void configureClassical(PreferredClassicalAlgorithm p);

    // configure fit to half algorithm
    void configureFitToHalf(
        casacore::FitToHalfStatisticsData::CENTER centerType,
        casacore::FitToHalfStatisticsData::USE_DATA useData,
        casacore::Double centerValue
    );

    // configure hinges-fences algorithm
    void configureHingesFences(casacore::Double f);

protected:

    ImageStatsConfigurator(
        const SPCIIF image,
        const casacore::Record *const &regionPtr,
        const casacore::String& maskInp
    );

    casacore::String _configureAlgorithm();

    std::unique_ptr<casacore::ImageStatistics<casacore::Float> >& _getImageStats() {
        return _statistics;
    }

    casacore::LatticeStatistics<casacore::Float>::AlgConf _getAlgConf() const {
        return _algConf;
    }

    void _resetStats(ImageStatistics<Float>* stat=nullptr) { _statistics.reset(stat); }

private:
    std::unique_ptr<casacore::ImageStatistics<casacore::Float> > _statistics;
    casacore::LatticeStatistics<casacore::Float>::AlgConf _algConf;
    PreferredClassicalAlgorithm _prefClassStatsAlg;

};
}

#endif
