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
#include <display/QtPlotter/Util.h>
#include <images/Images/ImageInterface.h>
#include <images/Regions/ImageRegion.h>
#include <images/Regions/RegionManager.h>
#include <images/Regions/WCEllipsoid.h>
#include <images/Regions/WCExtension.h>
#include <coordinates/Coordinates/TabularCoordinate.h>
#include <display/Display/DisplayCoordinateSystem.h>
#include <display/QtPlotter/QtProfile.qo.h>
#include <imageanalysis/ImageAnalysis/PixelValueManipulator.h>
#include <imageanalysis/Annotations/AnnEllipse.h>
#include <imageanalysis/Annotations/AnnPolygon.h>
#include <assert.h>
#include <cmath>
#include <iostream>
#include <QMessageBox>
#include <QWidget>
#include <QMap>
#include <QDir>
#include <QDebug>
#include <QtCore/qmath.h>

namespace casa {

	const QString Util::ORGANIZATION = "NRAO/CASA";
	const QString Util::APPLICATION = "Spectral Profiler";

	Util::Util() {
		// TODO Auto-generated constructor stub
	}

	Util::~Util() {
		// TODO Auto-generated destructor stub
	}

	const double Util::PI = std::atan(1.0) * 4.0;
	const double Util::TIME_CONV = 60.0;
	const double Util::RAD_DEGREE_CONVERSION = 572.95779513082;

	double Util::degMinSecToRadians( int degrees, int mins, float secs ) {
		assert( -90 <= degrees && degrees <=90 );
		assert( 0 <= mins && mins < TIME_CONV );
		assert( 0 <= secs && secs < TIME_CONV );
		double decimalDegrees = toDecimalDegrees( degrees, mins, secs );
		return toRadians( decimalDegrees );
	}

	double Util::hrMinSecToRadians( int hours, int mins, float secs ) {
		assert( 0 <= hours && hours <= 24 );
		assert( 0 <= mins && mins < TIME_CONV );
		assert( 0 <= secs && secs < TIME_CONV );

		double decimalHours = toDecimalDegrees( hours, mins, secs );
		double degrees = decimalHours * 15;
		return toRadians( degrees );
	}

	QString Util::toHTML( const QString& baseStr ) {
		QString htmlStr( "<font color='black'>");
		htmlStr.append( baseStr );
		htmlStr.append( "</font>");
		return htmlStr;
	}

	double Util::toRadians( double degrees ) {
		double rads = degrees *  PI / 180;
		return rads;
	}

	double Util::toDecimalDegrees( int degrees, int mins, float seconds ) {
		double deg = degrees + mins / TIME_CONV + seconds / (TIME_CONV * TIME_CONV );
		return deg;
	}

	void Util::showUserMessage( QString& msg, QWidget* parent ) {
		QMessageBox msgBox( parent );
		msgBox.setText( msg );
		msgBox.exec();
	}

	void Util::minMax( double& min, double& max, const Vector<Double>& values ) {
		if ( values.size() > 0 ) {
			min = values[0];
			max = values[0];
			for ( int i = 1; i < static_cast<int>(values.size()); i++ ) {
				if ( values[i] < min ) {
					min = values[i];
				} else if ( values[i] > max ) {
					max = values[i];
				}
			}
		}
	}

	int Util::getCenter( const Vector<Double>& values, Double& mean ) {
		if ( values.size() == 1 ) {
			mean =  values[0];
		} else if ( values.size() == 2 ) {
			mean =  0.5*(values[0]+values[1]);
		} else {
			Double minval;
			Double maxval;
			minMax(minval, maxval, values);
			mean =  0.5*(minval + maxval);
		}
		int pos = static_cast<int>(floor(mean+0.5));
		return pos;
	}

	void Util::getRa(double radians, int& raHour, int& raMin, double& raSec) {
		double ras = radians * 24 * RAD_DEGREE_CONVERSION;
		if (ras > 86400) ras = 0;
		double rah = ras/3600;
		raHour = (int)floor(rah);
		double ram = (rah - raHour) * 60;
		raMin = (int)floor(ram);
		double raSecond = (ram - raMin) * 60;
		raSec = floor(1000 * raSecond) / 1000.;
	}

	void Util::getDec(double radians, int& decDeg, int& decMin, double& decSec) {

		int sign = (radians > 0) ? 1 : -1;
		double decs = sign * radians * 360 * RAD_DEGREE_CONVERSION;

		if (decs > 1296000) decs = 0;

		double decd = decs / 3600;
		decDeg = (int)floor(decd);
		double decm = (decd - decDeg) * 60;
		decMin = (int)floor(decm);
		double decSeconds = (decm - decMin) * 60;
		decSec = floor(1000 * decSeconds) / 1000.;

	}

	QString Util::stripBrackets( QString unitStr ) {
		int startIndex = unitStr.indexOf( "[");
		int endIndex = unitStr.indexOf( "]");
		QString noBrackets = unitStr;
		if ( startIndex >= 0 && endIndex > 0 ) {
			noBrackets = unitStr.mid(startIndex+1, endIndex - startIndex-1);
		}
		return noBrackets;
	}

	QString Util::stripFont( QString unitStr ) {
		int openingBracketEnd = unitStr.indexOf( ">");
		int endBracketStart = unitStr.indexOf( "<", openingBracketEnd);
		QString strippedUnits = unitStr;
		if ( openingBracketEnd > 0 && endBracketStart > 0 ) {
			strippedUnits = unitStr.mid(openingBracketEnd + 1, endBracketStart - openingBracketEnd -1 );
		}
		return strippedUnits;
	}

	QList<QString> Util::getTitleCaseVariations( QString source ) {
		QString baseString = source.trimmed().toLower();
		QStringList wordList = baseString.split( " ", QString::SkipEmptyParts );
		QMap<int,QList<QString> > wordMap;
		int wordCount = wordList.size();
		for ( int i = 0; i < wordCount; i++ ) {
			QString wordLookup = wordList[i];
			QList<QString> titleCaseList;
			titleCaseList.append( wordLookup );
			titleCaseList.append( toTitleCase( wordLookup ));
			wordMap.insert( i, titleCaseList );
		}

		//Combine the strings
		int count = static_cast<int>(qPow( 2, wordCount ));
		QList<QString> variationList;
		int i = 0;
		while ( i < count ) {
			QString variation;
			for ( int j = 0; j < wordCount; j++ ) {
				int lookup = static_cast<int>((i / qPow( 2, wordCount - j - 1))) % 2;
				QList<QString> lookupList = wordMap[j];
				variation = variation + lookupList[lookup];
				if ( j != wordCount - 1 ) {
					variation = variation + " ";
				}
			}
			variationList.append(variation);
			i++;
		}
		return variationList;
	}

	QString Util::toTitleCase( QString word ) {
		QString firstLetter = word.mid( 0, 1 );
		QString firstLetterUpperCase = firstLetter.toUpper();
		QString result = word.replace(0,1,firstLetterUpperCase );
		return result;
	}

	QString Util::toDegreeString( int hrs, int mins, double secs ) {
		return QString::number(hrs) + ":"+QString::number(mins)+":"+QString::number(secs);
	}

	QString Util::toDecString( int hrs, int mins, double secs ) {
		return QString::number(hrs) + "."+QString::number(mins)+"."+QString::number(secs);
	}

	void Util::appendSign( double rad, QString& degreeString ) {
		if ( rad < 0 ) {
			degreeString = "-"+degreeString;
		} else {
			degreeString = "+"+degreeString;
		}
	}

	int Util::getTabularFrequencyAxisIndex(SHARED_PTR<const ImageInterface<Float> > img){
		int tabIndex = -1;
		const DisplayCoordinateSystem cSys = img->coordinates();
		Int tabularCoordinateIndex = cSys.findCoordinate( Coordinate::TABULAR);
		if ( tabularCoordinateIndex >= 0 ){
			//Check to see if the units are frequency
			TabularCoordinate tabCoordinate = cSys.tabularCoordinate( tabularCoordinateIndex );
			Vector<String> tabUnits = tabCoordinate.worldAxisUnits();
			int tabCount = tabUnits.size();
			for ( int i = 0; i < tabCount; i++ ){
				QString tabUnit( tabUnits[i].c_str() );
				if ( tabUnit.indexOf( "Hz") != -1 ){
					tabIndex = tabularCoordinateIndex;
					break;
				}
			}

		}
		return tabIndex;
	}

	String Util::mainImageName( String path ){
		String coreName = path;
		QChar sep = QDir::separator();
		int slashIndex = path.find_last_of( sep.toAscii(), 0 );
		if ( slashIndex >= 0 ){
			int count = path.length();
			coreName = path.substr(slashIndex, count );
		}
		int dotIndex = path.find(".image");
		if ( dotIndex >= 0){
			coreName = path.substr(0, dotIndex);
		}
		return coreName;
	}

	Record Util::make3DRegion( const DisplayCoordinateSystem& cSys, ImageRegion* shape3D,
			int channelMin, int channelMax, int spectralAxisIndex ){
		Record regionRecord;
		if ( shape3D != NULL ){
			//Make an extension box in the spectral direction.
			bool spectralAxis = cSys.hasSpectralAxis();
			if ( spectralAxis ) {
				uInt nExtendAxes = 1;
				IPosition pixelAxes(nExtendAxes);
				if (spectralAxisIndex > 0) {
					pixelAxes[0] = spectralAxisIndex;
					int pixelAxesCount = pixelAxes.size();
					Vector<Int> absRel(pixelAxesCount, RegionType::Abs);
					Vector<Quantity> minRange(pixelAxesCount);
					Vector<Quantity> maxRange(pixelAxesCount);
					minRange[0]=Quantity( channelMin, "pix" );
					maxRange[0] = Quantity( channelMax, "pix");
					WCBox wbox(minRange, maxRange, pixelAxes, cSys, absRel);
					WCExtension extension( *shape3D, wbox);
					ImageRegion extendedRegion (extension);
					regionRecord = extendedRegion.toRecord("");
				}
				else {
					qDebug() << "No elliptical 3D region - missing spectral axis.";
				}
			}
			else {
				qDebug() << "Could not make ellipsoid.";
			}
		}
		return regionRecord;
	}

	Record Util::getEllipticalRegion3D( const DisplayCoordinateSystem& cSys,
				const Vector<Double>& x, const Vector<Double>& y,
				int channelMin, int channelMax, int spectralAxisNumber ){

		ImageRegion* ellipsoid = getEllipsoid( cSys, x, y);
		Record regionRecord = make3DRegion( cSys, ellipsoid, channelMin, channelMax, spectralAxisNumber );
		return regionRecord;
	}

	Record Util::getPolygonalRegion3D( const DisplayCoordinateSystem& cSys,
				const Vector<Double>& x, const Vector<Double>& y,
				int channelMin, int channelMax, int spectralAxisNumber){
		ImageRegion* polygon = getPolygon( cSys,x,y);
		Record regionRecord = make3DRegion( cSys, polygon, channelMin, channelMax, spectralAxisNumber );
		return regionRecord;
	}

	ImageRegion* Util::getPolygon(const DisplayCoordinateSystem& cSys,
			const Vector<Double>& x, const Vector<Double>& y){
		ImageRegion* polygon = NULL;
		const String radUnits( "rad");
		RegionManager regMan;
		int n = x.size();
		Vector<Quantity> xvertex(n);
		Vector<Quantity> yvertex(n);
		for (Int k = 0; k < n; ++k) {
			xvertex[k] = Quantity(x[k], radUnits);
			yvertex[k] = Quantity(y[k], radUnits);
		}
		int directionIndex = cSys.findCoordinate( Coordinate::DIRECTION );
		if ( directionIndex >= 0 ){
			Vector<Int> dirPixelAxis = cSys.pixelAxes(directionIndex);
			Vector<Int> pixax(2);
			pixax(0) = dirPixelAxis[0];
			pixax(1) = dirPixelAxis[1];
			polygon = regMan.wpolygon(xvertex, yvertex, pixax, cSys, "abs");
		}
		return polygon;
	}

	ImageRegion* Util::getEllipsoid(const DisplayCoordinateSystem& cSys,
			const Vector<Double>& x, const Vector<Double>& y){
		Vector<Quantity> center(2);
		Vector<Quantity> radius(2);
		ImageRegion* imageRegion = NULL;
		if ( x.size() == 2 && y.size() == 2 ){
			const String radUnits( "rad");
			center[0] = Quantity( (x[0]+x[1])/2, radUnits );
			center[1] = Quantity( (y[0]+y[1])/2, radUnits );

			MDirection::Types type = MDirection::N_Types;
			int directionIndex = cSys.findCoordinate( Coordinate::DIRECTION );
			if ( directionIndex >= 0 ){
				uInt dirIndex = static_cast<uInt>(directionIndex);
				type = cSys.directionCoordinate(dirIndex).directionType(true);

				Vector<Double> qCenter(2);
				qCenter[0] = center[0].getValue();
				qCenter[1] = center[1].getValue();
				MDirection mdcenter( Quantum<Vector<Double> >(qCenter,radUnits), type );

				Vector<Double> blc_rad_x(2);
				blc_rad_x[0] = x[0];
				blc_rad_x[1] = center[1].getValue();
				MDirection mdblc_x( Quantum<Vector<Double> >(blc_rad_x,radUnits),type );

				Vector<Double> blc_rad_y(2);
				blc_rad_y[0] = center[0].getValue();
				blc_rad_y[1] = y[0];
				MDirection mdblc_y( Quantum<Vector<Double> >(blc_rad_y,radUnits),type );

				double xdistance = mdcenter.getValue( ).separation(mdblc_x.getValue( ));
				double ydistance = mdcenter.getValue( ).separation(mdblc_y.getValue( ));
				const float ERR = 0.000001;
				if ( xdistance >= ERR && ydistance >= ERR ){

					radius[0] = Quantity(xdistance, radUnits );
					radius[1] = Quantity(ydistance, radUnits );

					Vector<Int> pixax(2);
					Vector<Int> dirPixelAxis = cSys.pixelAxes(directionIndex);
					pixax(0) = dirPixelAxis[0];
					pixax(1) = dirPixelAxis[1];
					WCEllipsoid ellipsoid( center, radius, IPosition(dirPixelAxis), cSys);
					imageRegion = new ImageRegion( ellipsoid );
				}
			}
		}
		else {
			qDebug() << "Invalid size (2) for an ellipse: "<<x.size()<<" and "<<y.size();
		}
		return imageRegion;
	}



	Record Util::getRegionRecord( String shape, const DisplayCoordinateSystem& cSys,
			const Vector<Double>& x, const Vector<Double>& y){
		const String radUnits( "rad");
		const String absStr( "abs");
		Record regionRecord;
		Int directionIndex = cSys.findCoordinate(Coordinate::DIRECTION);
		if ( directionIndex >= 0 ){
			Vector<Int> dirPixelAxis = cSys.pixelAxes(directionIndex);
			RegionManager regMan;
			if ( shape == QtProfile::SHAPE_RECTANGLE ){
				Vector<Quantity> blc(2);
				Vector<Quantity> trc(2);
				blc(0) = Quantity(x[0], radUnits);
				blc(1) = Quantity(y[0], radUnits);
				trc(0) = Quantity(x[1], radUnits);
				trc(1) = Quantity(y[1], radUnits);
				Vector<Int> pixax(2);
				pixax(0) = dirPixelAxis[0];
				pixax(1) = dirPixelAxis[1];

				Record* imagregRecord = regMan.wbox(blc, trc, pixax, cSys, absStr, radUnits);
				regionRecord = *imagregRecord;
				delete imagregRecord;
			}
			else if ( shape == QtProfile::SHAPE_ELLIPSE ){
				ImageRegion* ellipsoid = getEllipsoid( cSys, x, y );
				if ( ellipsoid != NULL ){
					regionRecord = ellipsoid->toRecord("");
					delete ellipsoid;
				}
			}
			else if ( shape == QtProfile::SHAPE_POLY ){
				ImageRegion* polygon = getPolygon( cSys, x, y );
				if ( polygon != NULL ){
					regionRecord = polygon->toRecord(String(""));
					delete polygon;
				}
			}
			else if ( shape == QtProfile::SHAPE_POINT ){
				//Try a rectangle with blc=trc;
				Vector<Quantity> blc(2);
				Vector<Quantity> trc(2);
				blc(0) = Quantity(x[0], radUnits);
				blc(1) = Quantity(y[0], radUnits);
				trc(0) = Quantity(x[0], radUnits);
				trc(1) = Quantity(y[0], radUnits);
				Vector<Int> pixax(2);
				pixax(0) = dirPixelAxis[0];
				pixax(1) = dirPixelAxis[1];

				Record* imagregRecord = regMan.wbox(blc, trc, pixax, cSys, absStr, radUnits);
				regionRecord=*imagregRecord;
				delete imagregRecord;
			}
			else {
				qDebug() <<"Util::getRegionRecord unrecognized shape: "<<shape.c_str();
			}
		}

		return regionRecord;
	}

	std::pair<Vector<Float>,Vector<Float> > Util::getProfile(SHARED_PTR<const casa::ImageInterface<Float> >& imagePtr,
					const Vector<Double>& x, const Vector<Double>& y, const String& shape,
					int tabularAxis, ImageCollapserData::AggregateType function, String unit,
					const String& coordinateType, const Quantity *const restFreq, const String& frame){

		DisplayCoordinateSystem cSys = imagePtr->coordinates();
		uInt spectralAxis = 0;
		if ( cSys.hasSpectralAxis()){
			spectralAxis = cSys.spectralAxisNumber();
		}
		else if ( tabularAxis >= 0 ){
			spectralAxis = (uInt)(tabularAxis);
		}

		Record regionRecord = getRegionRecord( shape, cSys, x, y);
		QString pixelSpectralType(coordinateType.c_str());
		if ( pixelSpectralType == QtProfile::FREQUENCY ){
			pixelSpectralType = "default";
		}
		else if ( pixelSpectralType == QtProfile::CHANNEL ){
			pixelSpectralType = "default";
			unit = "pixel";
		}

		PixelValueManipulatorData::SpectralType specType
			= PixelValueManipulatorData::spectralType( pixelSpectralType.toStdString().c_str() );
		Vector<Float> jyValues;
		Vector<Float> xValues;
		try {
			PixelValueManipulator<Float> pvm(imagePtr, &regionRecord, "");

			Record result = pvm.getProfile( spectralAxis, function, unit, specType,
				restFreq, frame );
			const String VALUE_KEY( "values");
			if ( result.isDefined( VALUE_KEY )){
				result.get( VALUE_KEY, jyValues );

			}

			const String COORD_KEY( "coords" );
			if ( result.isDefined( COORD_KEY ) ){
				Vector<Double> coords;
				result.get( COORD_KEY, coords );
				int coordCount = coords.size();
				xValues.resize( coordCount );
				for ( int i = 0; i < coordCount; i++ ){
					xValues[i] = static_cast<Float>( coords[i]);
				}
			}

			const String UNIT_KEY( "xUnit" );
			if ( result.isDefined( UNIT_KEY ) ){
				unit = result.asString( UNIT_KEY );
			}
		}
		catch( AipsError& error ){
			qDebug() << "Could not generate profile: "<<error.getMesg().c_str();
		}
		std::pair<Vector<Float>,Vector<Float> > resultVectors( xValues, jyValues );
		return resultVectors;
	}

	bool Util::arrayEquals( const Vector<Double>& a, const Vector<Double>& b ){
		bool equalArrays = false;
		if ( a.nelements() == b.nelements() ){
			int elementCount = a.size();
			bool equalElements = true;
			for ( int i = 0; i < elementCount; i++ ){
				if ( a[i] != b[i] ){
					equalElements = false;
					break;
				}
			}
			if ( equalElements ){
				equalArrays = true;
			}
		}
		return equalArrays;
	}
}
