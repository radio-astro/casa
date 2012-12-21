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
#include <QVector>
#include <QDebug>
#include <assert.h>

namespace casa {

ComponentListWrapper::ComponentListWrapper(){

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

void ComponentListWrapper::remove( const QVector<int>& indices ){
	int totalRemoveCount = indices.size();
	Vector<int> removeIndices( totalRemoveCount );
	for ( int i = 0; i < totalRemoveCount; i++ ){
		removeIndices[i] = indices[i];
	}
	skyList.remove( removeIndices );
}

double ComponentListWrapper::getRAValue( int i ) const {
	assert( i >= 0 && i < getSize() );
	MDirection mDirection = skyList.getRefDirection( i );
	Unit radUnit( "rad");
	Quantum<Vector<Double> > radQuantum = mDirection.getAngle( radUnit );
	Vector<Double> radVector = radQuantum.getValue();
	return radVector[0];
}

string ComponentListWrapper::getRA( int i ) const {
	double raValue = getRAValue( i );
	MVAngle raAngle( raValue );
	String raStr = raAngle.string( MVAngle::TIME, 10 );
	return raStr;
}

double ComponentListWrapper::getDECValue( int i ) const {
	assert( i >= 0 && i < getSize() );
	MDirection mDirection = skyList.getRefDirection( i );
	Unit radUnit( "rad");
	Quantum<Vector<Double> > radQuantum = mDirection.getAngle( radUnit );
	Vector<Double> radVector = radQuantum.getValue();
	return radVector[1];
}

string ComponentListWrapper::getDEC( int i ) const {
	double decValue = getDECValue( i );
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

Vector<double> ComponentListWrapper::getLatLong( int i ) const {
	assert( i >= 0 && i < getSize() );
	MDirection mDirection = skyList.getRefDirection( i );
	const String DEGREE_STR( "deg");
	Unit degreeUnit( DEGREE_STR);
	Quantum<Vector<Double> > angleQuantum = mDirection.getAngle( degreeUnit );
	Vector<Double> angleVector = angleQuantum.getValue();
	return angleVector;
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

			const ComponentShape *compShape = skyList.getShape(index);

			Vector<Double> shapeParams =compShape->parameters();
			int parameterCount =compShape->nParameters();
			const QString POINT_WIDTH( "1");
			const QString ARC_SEC( "arcsec");
			const QString DEG_STR( "deg");
			QString majorAxis = POINT_WIDTH + ARC_SEC;
			QString minorAxis = POINT_WIDTH + ARC_SEC;
			QString posAngle = "0" + DEG_STR;
			if (parameterCount > 2){
				double majorAxisValue = shapeParams(0)/C::pi*180.0*3600.0;
				double minorAxisValue = shapeParams(1)/C::pi*180.0*3600.0;
				majorAxis = QString::number(majorAxisValue) + ARC_SEC;
				minorAxis = QString::number(minorAxisValue) + ARC_SEC;
				double posValue = shapeParams(2)*180.0/C::pi;
				posAngle = QString::number(posValue) + DEG_STR;


				//Pixel centers
				int worldAxisCount = coordSystem.nWorldAxes();
				if ( worldAxisCount >= 2 ){
					Vector<double> worldCoordinates( worldAxisCount );
					worldCoordinates[0] = getRAValue( index );
					worldCoordinates[1] = getDECValue( index );
					Vector<double> pixelCoordinates( worldAxisCount );
					coordSystem.toPixel( pixelCoordinates, worldCoordinates );
					QString xCenter = QString::number(static_cast<int>(pixelCoordinates[0]));
					QString yCenter = QString::number(static_cast<int>(pixelCoordinates[1]));

					// get the integrated flux value
					Quantity integratedFlux = getFlux( index);
					Unit imUnit=image->units();
					ImageInfo imageInformation = image->imageInfo();

					// get the peak flux from the integrated flux
					Quantity peakFluxQuantity=SkyCompRep::integralToPeakFlux(directionCoordinate,
								ComponentType::GAUSSIAN, integratedFlux,
								imUnit, Quantity(shapeParams(0),"rad"), Quantity(shapeParams(1),"rad"),
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
				else {
					errorMsg = errorMsg + "\n Error finding center for source "+QString::number((index+1));
				}
			}
			else {
				errorMsg = errorMsg + "\n Error finding major/minor axis source "+QString::number((index+1));
			}
		}
		if ( writeCount < lineCount ){
			successfulWrite = false;
		}
	}
	return successfulWrite;
}

ComponentListWrapper::~ComponentListWrapper() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
