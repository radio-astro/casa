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

#ifndef CONVERTERFREQUENCY_H_
#define CONVERTERFREQUENCY_H_

#include <display/QtPlotter/conversion/Converter.h>

namespace casa {

	class ConverterFrequency : public Converter {
	public:
		ConverterFrequency(const QString& oldUnits, const QString& newUnits);
		static void convertFrequency( Vector<double> &resultValues,
		                              QString& frequencySourceUnits, QString& frequencyDestUnits,
		                              SpectralCoordinate& spectralCoordinate );
		virtual double toPixel( double value, SpectralCoordinate spectralCoordinate);
		virtual Vector<double> convert( const Vector<double>& oldValues, SpectralCoordinate spectralCoordinate);
		virtual ~ConverterFrequency();
	};

} /* namespace casa */
#endif /* CONVERTERFREQUENCY_H_ */
