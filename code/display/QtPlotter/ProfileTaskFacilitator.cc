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
#include "ProfileTaskFacilitator.h"
#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/QtPlotter/ProfileTaskMonitor.h>
#include <images/Images/ImageInterface.h>
#include <casa/Logging/LogIO.h>
#include <QDoubleValidator>
#include <QDebug>

namespace casa {

	ProfileTaskFacilitator::ProfileTaskFacilitator() :
		pixelCanvas( NULL ), taskMonitor( NULL ), logger( NULL ),
		validator( NULL ) {

		validator = new QDoubleValidator(-1.0e+32, 1.0e+32,10, NULL);
	}

	void ProfileTaskFacilitator::clear() {

	}

	void ProfileTaskFacilitator::pixelsChanged( int /*pixX*/, int /*pixY*/ ) {

	}
	void ProfileTaskFacilitator::setCanvas( QtCanvas* pCanvas ) {
		pixelCanvas = pCanvas;
	}

	void ProfileTaskFacilitator::setTaskMonitor( ProfileTaskMonitor* monitor ) {
		taskMonitor = monitor;
	}

	void ProfileTaskFacilitator::setLogger( LogIO* log ) {
		logger = log;
	}

	QString ProfileTaskFacilitator::getFileName() {
		return taskMonitor->getFileName();
	}

	void ProfileTaskFacilitator::logWarning(String msg, bool opticalSource ) {
		//Only log if the source matches the mode we are in.
		bool currentlyOptical = isOptical();
		if ( currentlyOptical == opticalSource ){
			*logger << LogIO::WARN << msg << LogIO::POST;
		}
	}
	void ProfileTaskFacilitator::postStatus( String status, bool opticalSource ) {
		bool currentlyOptical = isOptical();
		if ( currentlyOptical == opticalSource ){
			taskMonitor->postStatus( status );
		}
	}
	Vector<Float> ProfileTaskFacilitator::getXValues() const {
		return taskMonitor -> getXValues();
	}
	Vector<Float> ProfileTaskFacilitator::getYValues() const {
		return taskMonitor -> getYValues();
	}
	Vector<Float> ProfileTaskFacilitator::getZValues() const {
		return taskMonitor -> getZValues();
	}

	QString ProfileTaskFacilitator::getYUnit() const {
		return taskMonitor -> getYUnit();
	}

	QString ProfileTaskFacilitator::getYUnitPrefix() const {
		return taskMonitor -> getYUnitPrefix();
	}
	String ProfileTaskFacilitator::getXAxisUnit() const {
		return taskMonitor -> getXAxisUnit();
	}

	void ProfileTaskFacilitator::plotMainCurve() {
		taskMonitor -> plotMainCurve();
	}

	const std::tr1::shared_ptr<const ImageInterface<Float> > ProfileTaskFacilitator::getImage( const QString& imageName) const {
		return taskMonitor -> getImage( imageName );
	}

	const String ProfileTaskFacilitator::getPixelBox() const {
		Vector<double> xPixels;
		Vector<double> yPixels;
		taskMonitor->getPixelBounds(xPixels, yPixels);
		String box = "";
		const String commaStr = ",";
		if ( xPixels.size() == 2 && yPixels.size() == 2 ) {

			box = String::toString(xPixels[0]) + commaStr;
			box.append( String::toString( yPixels[0] )+ commaStr);
			box.append( String::toString( xPixels[1] ) + commaStr );
			box.append( String::toString( yPixels[1] ) );
		} else if ( xPixels.size() == 1 && yPixels.size() == 1 ) {
			box = String::toString(xPixels[0]) + commaStr;
			box.append( String::toString( yPixels[0] )+ commaStr);
			box.append( String::toString( xPixels[0] ) + commaStr );
			box.append( String::toString( yPixels[0] ) );
		} else {
			qDebug() << "Unrecognized region pixel size is "<<xPixels.size();
		}
		return box;
	}

	bool ProfileTaskFacilitator::isOptical() {
		return optical;
	}

	void ProfileTaskFacilitator::setOptical( bool opt ) {
		optical = opt;
	}

	bool ProfileTaskFacilitator::isValidChannelRangeValue( QString str, const QString& endStr ) {
		bool valid = !str.isEmpty();
		if ( !valid ) {
			String msg("No "+ endStr.toStdString() +" value specified!");
			logWarning(msg);
			postStatus(msg);
		} else {
			//These checks are necessary in cases the values are set
			//by the code rather than the user.
			int pos=0;
			if ( validator->validate(str, pos) != QValidator::Acceptable) {
				String startString(str.toStdString());
				String msg = String( endStr.toStdString() +" value not correct: ") + startString;
				logWarning( msg );
				postStatus(msg);
				valid = false;
			}
		}
		return valid;
	}

	void ProfileTaskFacilitator::findChannelRange( float startVal, float endVal,
	        const Vector<Float>& specValues, Int& channelStartIndex, Int& channelEndIndex ) {
		if (specValues.size() < 1) {
			String msg = String("No spectral values provided!");
			logWarning( msg );
			return;
		}

		Bool ascending=True;
		if (specValues(specValues.size()-1)<specValues(0)) {
			ascending=False;
		}
		int startIndex = 0;
		int endIndex = 0;
		String startValueStr( "Start value: " );
		String endValueStr( "End value: ");
		String smallerStr( " is smaller than all spectral values!");
		String largerStr( " is larger than all spectral values!");
		if (ascending) {
			if (endVal < specValues(0)) {
				String msg = startValueStr + String::toString(endVal) + smallerStr;
				logWarning( msg );
				return;
			}

			if (startVal > specValues(specValues.size()-1)) {
				String msg = endValueStr + String::toString(startVal) + largerStr;
				logWarning( msg );
				return;
			}

			startIndex=0;
			while (specValues(startIndex)<startVal) {
				startIndex++;
			}

			endIndex=specValues.size()-1;
			while (specValues(endIndex)>endVal) {
				endIndex--;
			}
		}
		//Descending case
		else {
			if (endVal < specValues(specValues.size()-1)) {
				String msg = startValueStr + String::toString(endVal) + smallerStr;
				logWarning( msg );
				return;
			}
			if (startVal > specValues(0)) {
				String msg = endValueStr + String::toString(startVal) + largerStr;
				logWarning( msg );
				return;
			}

			startIndex=0;
			while (specValues(startIndex)>endVal) {
				startIndex++;
			}
			endIndex=specValues.size()-1;
			while (specValues(endIndex)<startVal) {
				endIndex--;
			}
		}

		channelStartIndex = startIndex;
		channelEndIndex = endIndex;
	}

	void ProfileTaskFacilitator::setCollapseVals(const Vector<Float> &spcVals) {

		*logger << LogOrigin("QtProfile", "setCollapseVals");

		if ( spcVals.size() < 1 ) {
			String message = "No spectral values! Can not set collapse values!";
			logWarning( message );
			return;
		}

		// grab the start and end value
		Float valueStart=spcVals(0);
		Float valueEnd  =spcVals(spcVals.size()-1);

		Bool ascending(True);
		if (valueStart > valueEnd) {
			ascending=False;
		}

		// convert to QString
		QString startQStr =  QString((String::toString(valueStart)).c_str());
		QString endQStr   =  QString((String::toString(valueEnd)).c_str());

		// make sure the values are valid
		int pos=0;
		QDoubleValidator validator( -1.0e+32, 1.0e+32, 10, NULL );
		if (validator.validate(startQStr, pos) != QValidator::Acceptable) {
			String msg = String("Spectral value not correct: ") + String::toString(valueStart);
			logWarning( msg );
			return;
		}
		if (validator.validate(endQStr, pos) != QValidator::Acceptable) {
			String msg = String("Spectral value not correct: ") + String::toString(valueEnd);
			logWarning( msg );
			return;
		}

		//Set the values into the fields
		String msg;
		if (ascending) {
			setRange( valueStart, valueEnd );
			msg = String::toString(valueStart) + " and " + String::toString(valueEnd);
		} else {
			setRange( valueEnd, valueStart);
			msg = String::toString(valueEnd) + " and " + String::toString(valueStart);
		}

		// give feedback
		msg = String("Initial collapse values set: ") + msg;
		*logger << LogIO::NORMAL << msg << LogIO::POST;
	}

	void ProfileTaskFacilitator::setCurveName( const QString& /*curveName*/ ) {
		//Implemented to do nothing.  Subclasses should override if they need the
		//names of the curves being drawn on the canvas.
	}

	void ProfileTaskFacilitator::addCurveName( const QString& /*curveName*/ ) {
		//Implemented to do nothing.  Subclasses should override if they need the
		//names of the curves being added to the canvas.
	}

	void ProfileTaskFacilitator::setDisplayYUnits( const QString& /*units*/ ) {
		//Implemented to do nothing.  Subclasses should override if they need the
		//y axis units being displayed on the canvas.
	}

	void ProfileTaskFacilitator::setImageYUnits( const QString& /*units*/ ) {
		//Implemented to do nothing.  Subclasses should override if they need the
		//y axis units being displayed in the image.
	}

	ProfileTaskFacilitator::~ProfileTaskFacilitator() {
		delete validator;
	}

} /* namespace casa */
