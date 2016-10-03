//# Copyright (C) 2005
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
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
/*
*  This abstract class is an interface that specifies the basic functionality that
*  spectral line moments/fitting should implement.  It is expected that it will be subclassed
*  by implementation classes to provide specialized behavior.
*
*/

#ifndef PROFILE_TASK_FACILITATOR_H_
#define PROFILE_TASK_FACILITATOR_H_

#include <QString>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <casa/Containers/Record.h>

class QDoubleValidator;

namespace casacore{

	class LogIO;
	template <class T> class ImageInterface;
}

namespace casa {

	class QtCanvas;
	class ProfileTaskMonitor;


	class ProfileTaskFacilitator {
	public:
		ProfileTaskFacilitator();
		virtual ~ProfileTaskFacilitator();

		virtual void setUnits( QString units ) = 0;
		virtual void setRange(double start, double end )=0;
		virtual void reset() = 0;

		virtual void setCanvas( QtCanvas* pixelCanvas );
		virtual void setTaskMonitor( ProfileTaskMonitor* monitor );
		virtual void setCurveName( const QString& curveName );
		virtual void addCurveName( const QString& curveName );
		virtual void setDisplayYUnits( const QString& units );
		virtual void setImageYUnits( const QString& units );
		virtual void setLogger( casacore::LogIO* log );
		virtual void plotMainCurve();

		virtual QString getFileName();
		virtual void logWarning(casacore::String msg, bool opticalSource = false );
		virtual void postStatus( casacore::String status, bool opticalSource = false );
		virtual casacore::Vector<casacore::Float> getXValues() const;
		virtual casacore::Vector<casacore::Float> getYValues() const;
		virtual casacore::Vector<casacore::Float> getZValues() const;
		virtual casacore::Vector<casacore::Double> getRegionXValues() const;
		virtual casacore::Vector<casacore::Double> getRegionYValues() const;
		virtual QString getYUnit() const;
		virtual QString getYUnitPrefix() const;
		virtual casacore::String getXAxisUnit() const;
		virtual const SHARED_PTR<const casacore::ImageInterface<casacore::Float> > getImage( const QString& name="") const;
		virtual const casacore::String getPixelBox() const;
		virtual const casacore::String getRegionShape() const;

		bool isOptical();
		void setOptical( bool optical );

		virtual void clear();
		void setCollapseVals(const casacore::Vector<casacore::Float> &spcVals);

		virtual void pixelsChanged(int pixX, int pixY );

	protected:
		bool isValidChannelRangeValue( QString str, const QString& endStr );
		void findChannelRange( float startVal, float endVal,
		                       const casacore::Vector<casacore::Float>& specValues, casacore::Int& channelStartIndex, casacore::Int& channelEndIndex );
		QtCanvas* pixelCanvas;
		ProfileTaskMonitor* taskMonitor;
		casacore::LogIO* logger;

	private:
		bool optical;
		QDoubleValidator *validator;
	};

} // end namespace casa

#endif /* SPECFITTER_H_ */
