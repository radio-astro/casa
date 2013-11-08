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

#ifndef CONVERTER_H_
#define CONVERTER_H_

#include <QList>
#include <QString>
#include <casa/Arrays/Vector.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
namespace casa {

	class Converter {
	public:
		Converter( const QString& oldUnits, const QString& newUnits);

		//Factory for producing the appropriate converter.
		//Note:  user is responsible for deleting the converter.
		static Converter* getConverter( const QString& oldUnits,const QString& newUnits );
		static void setSpectralCoordinate( SpectralCoordinate coordinate );
		static void setRestFrequency( double restFrequency );
		static void convert( Vector<double> &resultValues, int sourceIndex, int destIndex);

		bool setVelocityUnits( const QString& units );
		bool setWavelengthUnits( const QString& units );
		QString getNewUnits() const;



		//Abstract methods to be implemented by subclasses.
		//virtual float convert( float oldValue ) const = 0;
		virtual double toPixel( double value ) = 0;
		virtual Vector<double> convert( const Vector<double>& oldValues ) = 0;
		virtual double convert ( double oldValue );
		virtual ~Converter();

	protected:
		static const QList<QString> FREQUENCY_UNITS;
		static const QList<QString> WAVELENGTH_UNITS;
		static const QList<QString> VELOCITY_UNITS;

		QString oldUnits;
		QString newUnits;
		static SpectralCoordinate spectralCoordinate;
		typedef enum {FREQUENCY_UNIT, VELOCITY_UNIT, WAVELENGTH_UNIT, CHANNEL_UNIT, UNRECOGNIZED } UnitType;

	private:
		static UnitType getUnitType( const QString& unit );

	};

} /* namespace casa */
#endif /* CONVERTER_H_ */
