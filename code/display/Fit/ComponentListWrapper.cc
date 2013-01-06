//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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

#include "ComponentListWrapper.h"
#include <images/Images/ImageInterface.h>
#include <casa/Quanta/MVAngle.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <components/ComponentModels/ComponentShape.h>
#include <components/ComponentModels/SkyCompRep.h>
#include <display/RegionShapes/RegionShapes.h>
#include <QVector>
#include <QDebug>
#include <assert.h>

namespace casa {

ComponentListWrapper::ComponentListWrapper():
	RAD("rad"), DEG("deg"), ARC_SEC("arcsec"){

}

void ComponentListWrapper::clear(){
	int componentCount = getSize();
	for ( int i = componentCount-1; i>= 0; i-- ){
		skyList.remove(i);
	}
}

int ComponentListWrapper::getSize() const {
	return skyList.nelements();
}

bool ComponentListWrapper::fromRecord( String& errorMsg, Record& record ){
	return skyList.fromRecord( errorMsg, record );
}

void ComponentListWrapper::fromComponentList( ComponentList list ){
	skyList = list;
}

void ComponentListWrapper::remove( const QVector<int>& indices ){
	int totalRemoveCount = indices.size();
	Vector<int> removeIndices( totalRemoveCount );
	for ( int i = 0; i < totalRemoveCount; i++ ){
		removeIndices[i] = indices[i];
	}
	skyList.remove( removeIndices );
}

double ComponentListWrapper::getRAValue( int i, const String& unit ) const {
	assert( i >= 0 && i < getSize() );
	MDirection mDirection = skyList.getRefDirection( i );
	Quantum<Vector<Double> > radQuantum = mDirection.getAngle( unit );
	Vector<Double> radVector = radQuantum.getValue();
	return radVector[0];
}



string ComponentListWrapper::getRA( int i ) const {
	double raValue = getRAValue( i, RAD );
	MVAngle raAngle( raValue );
	String raStr = raAngle.string( MVAngle::TIME, 10 );
	return raStr;
}

double ComponentListWrapper::getDECValue( int i, const String& unit ) const {
	assert( i >= 0 && i < getSize() );
	MDirection mDirection = skyList.getRefDirection( i );
	Quantum<Vector<Double> > radQuantum = mDirection.getAngle( unit );
	Vector<Double> radVector = radQuantum.getValue();
	return radVector[1];
}

string ComponentListWrapper::getDEC( int i ) const {
	double decValue = getDECValue( i, RAD );
	MVAngle decAngle( decValue );
	String decStr = decAngle.string( MVAngle::ANGLE, 10 );
	return decStr;
}

string ComponentListWrapper::getType( int i ) const {
	assert( i >= 0 && i < getSize() );
	MDirection mDirection = skyList.getRefDirection( i );
	String refString = mDirection.getRefString();
	return refString;
}

Quantum< Vector<double> > ComponentListWrapper::getLatLong( int i ) const {
	assert( i >= 0 && i < getSize() );
	MDirection mDirection = skyList.getRefDirection( i );
	Unit degreeUnit( DEG );
	Quantum<Vector<Double> > angleQuantum = mDirection.getAngle( degreeUnit );
	return angleQuantum;
}

Quantity ComponentListWrapper::getFlux( int i ) const {
	assert( i >= 0 && i < getSize() );
	Vector< Quantum<double> > fluxVector;
	skyList.getFlux( fluxVector, i );
	Quantity quantity;
	if ( fluxVector.size() > 0 ){
		quantity = fluxVector[0];
	}
	return quantity;
}

Quantity ComponentListWrapper::getAxis( int listIndex, int shapeIndex, bool toArcSecs ) const {
	const ComponentShape* compShape = skyList.getShape( listIndex );
	Vector<Double> shapeParams =compShape->parameters();
	int parameterCount =compShape->nParameters();
	double axisValue = -1;
	if (parameterCount > shapeIndex){
		axisValue = shapeParams(shapeIndex);
	}
	axisValue = radiansToDegrees( axisValue );
	String unitStr = DEG;
	if ( toArcSecs ){
		axisValue = degreesToArcSecs( axisValue );
		unitStr = ARC_SEC;
	}
	Quantity axisQuantity( axisValue, unitStr );
	return axisQuantity;
}

Quantity ComponentListWrapper::getMajorAxis( int i ) const {
	return getAxis( i, 0, true );
}

Quantity ComponentListWrapper::getMinorAxis( int i ) const {
	return getAxis( i, 1, true );
}

Quantity ComponentListWrapper::getAngle( int i ) const {
	return getAxis( i, 2, false );
}

double ComponentListWrapper::radiansToDegrees( double value ) const {
	return value/C::pi*180.0;
}

double ComponentListWrapper::degreesToArcSecs( double value ) const {
	return value * 3600.0;
}



bool ComponentListWrapper::toEstimateFile( QTextStream& stream,
		ImageInterface<Float>* image, QString& errorMsg ) const {
	//The format of each line is
	//peak intensity, peak x-pixel value, peak y-pixel value, major axis, minor axis, position angle, fixed
	CoordinateSystem coordSystem = image->coordinates();
	bool successfulWrite = true;
	bool directionCoordinate = coordSystem.hasDirectionCoordinate();
	if ( !directionCoordinate ){
		successfulWrite = false;
		errorMsg = "Image does not have a direction coordinate.";
	}
	else {
		DirectionCoordinate directionCoordinate = coordSystem.directionCoordinate(0);
		int lineCount = getSize();
		int writeCount = 0;
		for (int index=0; index<lineCount; index++){

			SkyComponent skyComponent = skyList.component( index );
			String summaryStr = skyComponent.summarize( &coordSystem );

			//Get the major & minor axis and the position angle.
			const QString POINT_WIDTH( "1");
			QString arcSecStr( ARC_SEC.c_str());
			QString degStr( DEG.c_str() );
			QString majorAxis = POINT_WIDTH + arcSecStr;
			QString minorAxis = POINT_WIDTH + arcSecStr;
			QString posAngle = "0" + degStr;
			Quantity majorAxisQuantity = getMajorAxis( index );
			if ( majorAxisQuantity.getValue() > 0 ){
				majorAxis = QString::number(majorAxisQuantity.getValue()) + arcSecStr;
			}
			Quantity minorAxisQuantity = getMinorAxis( index );
			if ( minorAxisQuantity.getValue() > 0 ){
				minorAxis = QString::number(minorAxisQuantity.getValue()) + arcSecStr;
			}

			Quantity angleQuantity =  getAngle( index );
			posAngle = QString::number(angleQuantity.getValue()) + degStr;

			//Pixel centers
			int worldAxisCount = coordSystem.nWorldAxes();
			if ( worldAxisCount >= 2 ){
				Vector<double> worldCoordinates( worldAxisCount );
				worldCoordinates[0] = getRAValue( index, RAD );
				worldCoordinates[1] = getDECValue( index, RAD );
				Vector<double> pixelCoordinates( worldAxisCount );
				coordSystem.toPixel( pixelCoordinates, worldCoordinates );
				QString xCenter = QString::number(static_cast<int>(pixelCoordinates[0]));
				QString yCenter = QString::number(static_cast<int>(pixelCoordinates[1]));

				// get the integrated flux value
				Quantity integratedFlux = getFlux( index);
				Unit imUnit=image->units();
				ImageInfo imageInformation = image->imageInfo();
				const ComponentShape* compShape = skyList.getShape( index );
				Vector<Double> shapeParams =compShape->parameters();
				int parameterCount =compShape->nParameters();
				if ( parameterCount >= 2 ){
					// get the peak flux from the integrated flux
					Quantity peakFluxQuantity=SkyCompRep::integralToPeakFlux(directionCoordinate,
								ComponentType::GAUSSIAN, integratedFlux,
								imUnit, Quantity(shapeParams(0),RAD), Quantity(shapeParams(1),RAD),
								imageInformation.restoringBeam());
					double peakFluxValue = peakFluxQuantity.getValue();
					QString peakFlux = QString::number( peakFluxValue );

					//Write a line
					const QString COMMA_STR( ", ");
					stream << peakFlux << COMMA_STR;
					stream << xCenter << COMMA_STR;
					stream << yCenter << COMMA_STR;
					stream << majorAxis << COMMA_STR;
					stream << minorAxis << COMMA_STR;
					stream << posAngle;
					stream << "\n";
					writeCount++;
				}
			}
			else {
				errorMsg = errorMsg + "\n Error finding center for source "+QString::number((index+1));
			}
		}
		if ( writeCount < lineCount ){
			successfulWrite = false;
		}
	}
	return successfulWrite;
}

void ComponentListWrapper::toRecord( Record& record, const Quantity& quantity ) const {
	String recordError;
	if ( !QuantumHolder( quantity ).toRecord( recordError, record )){
		qDebug() << "Could not write quantity to record: "<<recordError.c_str();
	}
}

QList<RegionShape*> ComponentListWrapper::toDrawingDisplay(ImageInterface<Float>* image, const QString& colorName) const {
	int sourceCount = getSize();
	QList<RegionShape*> fitList;
	CoordinateSystem coordSystem = image->coordinates();
	bool directionCoordinate = coordSystem.hasDirectionCoordinate();
	if ( directionCoordinate ){
		DirectionCoordinate directionCoordinate = coordSystem.directionCoordinate(0);
		for (int index=0; index < sourceCount; index++){

			SkyComponent skyComponent = skyList.component( index );
			String summaryStr = skyComponent.summarize( &coordSystem );


			//Pixel centers
			int worldAxisCount = coordSystem.nWorldAxes();
			if ( worldAxisCount >= 2 ){
				Vector<double> worldCoordinates( worldAxisCount );
				worldCoordinates[0] = getRAValue( index, RAD );
				worldCoordinates[1] = getDECValue( index, RAD );
				Vector<double> pixelCoordinates( worldAxisCount );
				coordSystem.toPixel( pixelCoordinates, worldCoordinates );

				Quantity majorAxisValue = getMajorAxis( index );
				Quantity minorAxisValue = getMinorAxis( index );
				Quantity posValue = getAngle( index );
				if ( majorAxisValue.getValue() > 0 && minorAxisValue.getValue() > 0 ){
					RSEllipse* ellipse = new RSEllipse( pixelCoordinates[0], pixelCoordinates[1],
							majorAxisValue.getValue(), minorAxisValue.getValue(), posValue.getValue() );
					ellipse->setLineColor( colorName.toStdString() );
					fitList.append( ellipse );
				}
			}
		}
	}
	return fitList;
}

ComponentListWrapper::~ComponentListWrapper() {
}

} /* namespace casa */
