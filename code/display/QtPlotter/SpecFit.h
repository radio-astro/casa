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
#ifndef SPECFIT_H_
#define SPECFIT_H_

#include <casa/Arrays/Vector.h>
#include <QString>

namespace casa {

	class SpecFit {
	public:
		SpecFit( QString suffix );
		SpecFit( const SpecFit& other );
		QString getSuffix();
		virtual bool isXPixels();
		virtual void evaluate( Vector<Float>& xValues ) = 0;
		Vector<Float> getXValues() const;
		void setXValues( Vector<Float>& xVals );
		Vector<Float> getYValues() const;
		void setCurveName( QString curveName );
		QString getCurveName() const;
		bool isSpecFitFor( int pixelX, int pixelY, int minX, int minY ) const;
		void setFitCenter( int pixelX, int pixelY );
		SpecFit& operator=(const SpecFit& other );
		virtual ~SpecFit();

	protected:
		void initialize( const SpecFit& other );
		Vector<Float> xValues;
		Vector<Float> yValues;

	private:

		QString suffix;
		QString curveName;
		int centerX;
		int centerY;

	};


} /* namespace casa */
#endif /* SPECFIT_H_ */
