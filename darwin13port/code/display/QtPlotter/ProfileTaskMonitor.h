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
#ifndef PROFILETASKMONITOR_H_
#define PROFILETASKMONITOR_H_

#include <QString>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <measures/Measures/MFrequency.h>

#include <tr1/memory.hpp>

namespace casa {

	template <class T> class ImageInterface;

	class ProfileTaskMonitor {
	public:
		ProfileTaskMonitor();

		enum PURPOSE {
		    SPECTROSCOPY,
		    MOMENTS_COLLAPSE
		};
		virtual QString getFileName() const = 0;
		virtual QString getImagePath() const =0;
		virtual Vector<Float> getXValues() const = 0;
		virtual Vector<Float> getYValues() const = 0;
		virtual Vector<Float>getZValues() const = 0;
		virtual QString getYUnit() const = 0;
		virtual QString getYUnitPrefix() const = 0;
		virtual String getXAxisUnit() const = 0;
		virtual void postStatus( String str ) = 0;
		virtual void plotMainCurve() = 0;
		virtual std::tr1::shared_ptr<const ImageInterface<Float> > getImage( const QString& imageName = "") const = 0;
		virtual const void getPixelBounds( Vector<double>& xPixels, Vector<double>& yPixels) const = 0;
		virtual MFrequency::Types getReferenceFrame() const = 0;
		//Specific to Moments
		virtual void persist( const QString& key, const QString& value ) = 0;
		virtual QString read( const QString & key ) const = 0;
		virtual void imageCollapsed(String path, String dataType, String displayType,
				Bool autoRegister, Bool tmpData,
				std::tr1::shared_ptr<ImageInterface<Float> > img = std::tr1::shared_ptr<ImageInterface<Float> >())=0;
		virtual void setPurpose( ProfileTaskMonitor::PURPOSE purpose ) = 0;

		//Specific to Spectrum Position setting.
		virtual void setPosition( const QList<double> &world_x, const QList<double> &world_y ) =0;


		virtual ~ProfileTaskMonitor();
	};

} /* namespace casa */
#endif /* PROFILETASKMONITOR_H_ */
