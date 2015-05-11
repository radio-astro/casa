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
#include <images/Images/ImageInfo.h>
#include <casa/Quanta/MVAngle.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <components/ComponentModels/ComponentShape.h>
#include <components/ComponentModels/GaussianDeconvolver.h>
#include <components/ComponentModels/SkyCompRep.h>
#include <scimath/Mathematics/GaussianBeam.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <display/RegionShapes/RegionShapes.h>
#include <imageanalysis/Annotations/AnnEllipse.h>
#include <display/Fit/RegionBox.h>
#include <QVector>
#include <QDebug>
#include <assert.h>
#include <iostream>
#include <fstream>

namespace casa {

	ComponentListWrapper::ComponentListWrapper():
		RAD("rad"), DEG("deg"), ARC_SEC("arcsec") {

	}

	void ComponentListWrapper::clear() {
		int componentCount = getSize();
		for ( int i = componentCount-1; i>= 0; i-- ) {
			skyList.remove(i);
		}
	}

	int ComponentListWrapper::getSize() const {
		return skyList.nelements();
	}

	bool ComponentListWrapper::fromRecord( String& errorMsg, Record& record ) {
		return skyList.fromRecord( errorMsg, record );
	}

	void ComponentListWrapper::fromComponentList( ComponentList list ) {
		skyList = list;
	}

	void ComponentListWrapper::remove( QVector<int> indices ) {

		//Remove the indices from the sky list
		int totalRemoveCount = indices.size();
		Vector<int> removeIndices( totalRemoveCount );
		for ( int i = 0; i < totalRemoveCount; i++ ) {
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

	const ComponentShape* ComponentListWrapper::getShape( int i ) const {
		const ComponentShape* shape = NULL;
		if ( i >= 0 && i < getSize() ){
			shape = skyList.getShape(i);
		}
		return shape;
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
		if ( fluxVector.size() > 0 ) {
			quantity = fluxVector[0];
		}
		return quantity;
	}

	void ComponentListWrapper::deconvolve(const ImageInterface<float>* image, int channel,
	                                      Quantity& majorAxis, Quantity& minorAxis, Quantity& positionAngle) const {
		ImageInfo imageInformation = image->imageInfo();
		bool hasBeam = imageInformation.hasBeam();
		if ( hasBeam ) {
			DisplayCoordinateSystem coordSystem = image->coordinates();
			IPosition imageShape = image->shape();
			GaussianBeam beam = imageInformation.restoringBeam( channel );
            typedef GaussianBeam Angular2DGaussian;
            Angular2DGaussian originalBeam( majorAxis, minorAxis, positionAngle );
			Angular2DGaussian resultBeam;
            GaussianDeconvolver::deconvolve( resultBeam, originalBeam, beam );
			majorAxis.setValue( resultBeam.getMajor().getValue());
			minorAxis.setValue( resultBeam.getMinor().getValue());
			positionAngle.setValue( resultBeam.getPA().getValue());
		}
	}

	Quantity ComponentListWrapper::getAxis( int listIndex, int shapeIndex, bool toArcSecs ) const {
		const ComponentShape* compShape = skyList.getShape( listIndex );
		Vector<Double> shapeParams =compShape->parameters();
		int parameterCount =compShape->nParameters();
		double axisValue = -1;
		if (parameterCount > shapeIndex) {
			axisValue = shapeParams(shapeIndex);
		}
		axisValue = radiansToDegrees( axisValue );
		String unitStr = DEG;
		if ( toArcSecs ) {
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

	double ComponentListWrapper::rotateAngle( double value ) const {
		double rotatedValue = value + 90;
		while ( rotatedValue  < 0.0 ) {
			rotatedValue += 180.0;
		}
		while (rotatedValue >= 180.0) {
			rotatedValue -= 180.0;
		}
		return rotatedValue;
	}

	bool ComponentListWrapper::toEstimateFile( QTextStream& stream,
	        const ImageInterface<Float>*const image, QString& errorMsg,
	        bool screenEstimates, RegionBox* screenBox ) const {
		//The format of each line is
		//peak intensity, peak x-pixel value, peak y-pixel value, major axis, minor axis, position angle, fixed
		DisplayCoordinateSystem coordSystem = image->coordinates();
		bool successfulWrite = true;
		bool directionCoordinate = coordSystem.hasDirectionCoordinate();
		if ( !directionCoordinate ) {
			successfulWrite = false;
			errorMsg = "Image does not have a direction coordinate.";
		} else {
			DirectionCoordinate directionCoordinate = coordSystem.directionCoordinate(0);
			int lineCount = getSize();
			int writeCount = 0;
			for (int index=0; index<lineCount; index++) {

				SkyComponent skyComponent = skyList.component( index );
				//String summaryStr = skyComponent.summarize( &coordSystem );
				//qDebug() << "summaryStr="<<summaryStr.c_str();

				//Get the major & minor axis and the position angle.
				const QString POINT_WIDTH( "1");
				QString arcSecStr( ARC_SEC.c_str());
				QString degStr( DEG.c_str() );
				QString majorAxis = POINT_WIDTH + arcSecStr;
				QString minorAxis = POINT_WIDTH + arcSecStr;
				QString posAngle = "0" + degStr;
				Quantity majorAxisQuantity = getMajorAxis( index );
				Quantity minorAxisQuantity = getMinorAxis( index );
				double majorAxisValue = majorAxisQuantity.getValue();
				double minorAxisValue = minorAxisQuantity.getValue();
				if ( majorAxisValue < minorAxisValue ) {
					double tmp = majorAxisValue;
					majorAxisValue = minorAxisValue;
					minorAxisValue = tmp;
				}
				if ( majorAxisValue > 0 ) {
					majorAxis = QString::number(majorAxisValue) + arcSecStr;
				}

				if ( minorAxisValue > 0 ) {
					minorAxis = QString::number(minorAxisValue) + arcSecStr;
				}

				Quantity angleQuantity =  getAngle( index );
				double angleValue = angleQuantity.getValue();
				posAngle = QString::number(angleValue) + degStr;

				//Pixel centers
				int worldAxisCount = coordSystem.nWorldAxes();
				if ( worldAxisCount >= 2 ) {
					Vector<double> worldCoordinates( worldAxisCount );
					worldCoordinates[0] = getRAValue( index, RAD );
					worldCoordinates[1] = getDECValue( index, RAD );
					Vector<double> pixelCoordinates( worldAxisCount );
					coordSystem.toPixel( pixelCoordinates, worldCoordinates );
					QString xCenter = QString::number(static_cast<int>(pixelCoordinates[0]));
					QString yCenter = QString::number(static_cast<int>(pixelCoordinates[1]));
					bool estimateInRange = true;
					if ( screenEstimates && screenBox != NULL ) {
						if ( ! screenBox->isInBox(pixelCoordinates[0], pixelCoordinates[1])) {
							estimateInRange = false;
						}
					}
					if ( estimateInRange ) {
						// get the integrated flux value
						Quantity integratedFlux = getFlux( index);
						Unit imUnit=image->units();
						ImageInfo imageInformation = image->imageInfo();
						const ComponentShape* compShape = skyList.getShape( index );
						QString peakFlux = QString::number( integratedFlux.getValue() );
						Vector<Double> shapeParams =compShape->parameters();
						int parameterCount =compShape->nParameters();
						if ( parameterCount >= 2 ) {
							// get the peak flux from the integrated flux
							Quantity peakFluxQuantity=SkyCompRep::integralToPeakFlux(directionCoordinate,
													  ComponentType::GAUSSIAN, integratedFlux,
													  imUnit, Quantity(shapeParams(0),RAD), Quantity(shapeParams(1),RAD),
													  imageInformation.restoringBeam());
							double peakFluxValue = peakFluxQuantity.getValue();
							peakFlux = QString::number( peakFluxValue );
						}

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

				} else {
					errorMsg = errorMsg + "\n Error finding center for source "+QString::number((index+1));
				}
			}
			if ( !screenEstimates ) {
				if ( writeCount < lineCount ) {
					successfulWrite = false;
					errorMsg = errorMsg + "\n There was an error writing the sources";
				}
			} else {
				//If we are screening estimates, as long as we have written one,
				//we are happy.
				if ( writeCount == 0 ) {
					errorMsg = "Please check that the region contains at least one source estimate.";
					successfulWrite = false;
				}
			}
		}
		return successfulWrite;
	}

	void ComponentListWrapper::toRecord( Record& record, const Quantity& quantity ) const {
		String recordError;
		if ( !QuantumHolder( quantity ).toRecord( recordError, record )) {
			qDebug() << "Could not write quantity to record: "<<recordError.c_str();
		}
	}

	QList<RegionShape*> ComponentListWrapper::toDrawingDisplay(const ImageInterface<Float>* image,
	        const QString& colorName) const {
		int sourceCount = getSize();
		QList<RegionShape*> fitList;
		DisplayCoordinateSystem coordSystem = image->coordinates();

		for (int index=0; index < sourceCount; index++) {
			SkyComponent skyComponent = skyList.component( index );

			//Pixel centers
			int worldAxisCount = coordSystem.nWorldAxes();
			if ( worldAxisCount >= 2 ) {
				Vector<double> worldCoordinates( worldAxisCount );
				worldCoordinates[0] = getRAValue( index, RAD );
				worldCoordinates[1] = getDECValue( index, RAD );
				Vector<double> pixelCoordinates( worldAxisCount );
				coordSystem.toPixel( pixelCoordinates, worldCoordinates );

				const ComponentShape* shape = getShape( index );
				if ( shape != NULL ){
					//The convolved fit must be graphed because it is scaled
					//to match the image.
					if ( coordSystem.hasDirectionCoordinate()){
						int dirInd = coordSystem.findCoordinate(Coordinate::DIRECTION);
						const DirectionCoordinate& dirCoord = coordSystem.directionCoordinate(dirInd);
						Vector<double> axes = shape->toPixel( dirCoord );
						Quantity posValue = getAngle( index );
						double angleValue = rotateAngle( posValue.getValue());
						Vector<Int> dirAxes = coordSystem.directionAxesNumbers();
						int axesCount = axes.size();
						if( axesCount >= 4 ){
							double majorAxisValue = axes[2] / 2;
							double minorAxisValue = axes[3] / 2;
							if ( majorAxisValue > 0 && minorAxisValue > 0 ){
								RSEllipse* ellipse = new RSEllipse( pixelCoordinates[0], pixelCoordinates[1],
									majorAxisValue, minorAxisValue, angleValue );
								ellipse->setLineColor( colorName.toStdString() );
								fitList.append( ellipse );
							}
						}
					}
				}
			}
		}
		return fitList;
	}

	bool ComponentListWrapper::toRegionFile(const ImageInterface<float>* image,
	        int channelIndex, const QString& filePath ) const {
		bool success = false;
		int sourceCount = getSize();
		DisplayCoordinateSystem coordSystem = image->coordinates();
		IPosition imageShape = image->shape();
		Vector<Stokes::StokesTypes> stokes;
		CoordinateUtil::findStokesAxis(stokes, coordSystem);
		ofstream fileStream;
		fileStream.open(filePath.toStdString().c_str());

		if ( fileStream.is_open()) {
			fileStream << "#CRTF"<<"\n";
			for (int index=0; index < sourceCount; index++) {
				SkyComponent skyComponent = skyList.component( index );
				//Pixel centers
				int worldAxisCount = coordSystem.nWorldAxes();
				if ( worldAxisCount >= 2 ) {
					double xValue = getRAValue( index, RAD );
					Quantity xCenter( xValue, RAD );
					double yValue = getDECValue( index, RAD );
					Quantity yCenter( yValue, RAD );

					Quantity majorAxisValue = getMajorAxis( index );
					Quantity minorAxisValue = getMinorAxis( index );
					Quantity posValue = getAngle( index );

					//The deconvolved fit should be reported in the region file.
					deconvolve( image, channelIndex, majorAxisValue, minorAxisValue, posValue );
					AnnEllipse ellipse(xCenter, yCenter, majorAxisValue/2,
					                   minorAxisValue/2, posValue,
					                   coordSystem, imageShape, stokes);
					ellipse.print(fileStream);
					fileStream << "\n";
				}
			}
			success = true;
			fileStream.close( );
		}
		return success;
	}

	ComponentListWrapper::~ComponentListWrapper() {
	}

} /* namespace casa */
