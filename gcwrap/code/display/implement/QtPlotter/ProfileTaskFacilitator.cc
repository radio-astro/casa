/*
 * ProfileTaskFacilitator.cc
 *
 *  Created on: May 16, 2012
 *      Author: slovelan
 */

#include "ProfileTaskFacilitator.h"
#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/QtPlotter/ProfileTaskMonitor.h>
#include <images/Images/ImageInterface.h>
#include <casa/Logging/LogIO.h>
#include <QDoubleValidator>

namespace casa {

ProfileTaskFacilitator::ProfileTaskFacilitator() :
		pixelCanvas( NULL ), taskMonitor( NULL ), logger( NULL ),
		validator( NULL ){

	validator = new QDoubleValidator(-1.0e+32, 1.0e+32,10, NULL);
}

void ProfileTaskFacilitator::clear(){

}
void ProfileTaskFacilitator::setCanvas( QtCanvas* pCanvas ){
	pixelCanvas = pCanvas;
}

void ProfileTaskFacilitator::setTaskMonitor( ProfileTaskMonitor* monitor ){
	taskMonitor = monitor;
}

void ProfileTaskFacilitator::setLogger( LogIO* log ){
	logger = log;
}

QString ProfileTaskFacilitator::getFileName(){
	return taskMonitor->getFileName();
}

void ProfileTaskFacilitator::logWarning(String msg ){
	*logger << LogIO::WARN << msg << LogIO::POST;
}
void ProfileTaskFacilitator::postStatus( String status ){
	taskMonitor->postStatus( status );
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

void ProfileTaskFacilitator::plotMainCurve(){
	taskMonitor -> plotMainCurve();
}

const ImageInterface<Float>* ProfileTaskFacilitator::getImage() const{
	return taskMonitor -> getImage();
}

const String ProfileTaskFacilitator::getPixelBox() const {
	Vector<double> xPixels;
	Vector<double> yPixels;
	taskMonitor->getPixelBounds(xPixels, yPixels);
	String box = "";
	if ( xPixels.size() == 2 && yPixels.size() == 2 ){
		const String commaStr = ",";
		box = String::toString(xPixels[0]) + commaStr;
		box.append( String::toString( yPixels[0] )+ commaStr);
		box.append( String::toString( xPixels[1] ) + commaStr );
		box.append( String::toString( yPixels[1] ) );
	}
	return box;
}

bool ProfileTaskFacilitator::isOptical(){
	return optical;
}

void ProfileTaskFacilitator::setOptical( bool opt ){
	optical = opt;
}

bool ProfileTaskFacilitator::isValidChannelRangeValue( QString str, const QString& endStr ) {
	bool valid = !str.isEmpty();
	if ( !valid ){
		String msg("No "+ endStr.toStdString() +" value specified!");
		logWarning(msg);
		postStatus(msg);
	}
	else {
		//These checks are necessary in cases the values are set
		//by the code rather than the user.
		int pos=0;
		if ( validator->validate(str, pos) != QValidator::Acceptable){
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
	if (specValues.size() < 1){
		String msg = String("No spectral values provided!");
		logWarning( msg );
		return;
	}

	Bool ascending=True;
	if (specValues(specValues.size()-1)<specValues(0)){
		ascending=False;
	}
	int startIndex = 0;
	int endIndex = 0;
	String startValueStr( "Start value: " );
	String endValueStr( "End value: ");
	String smallerStr( " is smaller than all spectral values!");
	String largerStr( " is larger than all spectral values!");
	if (ascending){
		if (endVal < specValues(0)){
			String msg = startValueStr + String::toString(endVal) + smallerStr;
			logWarning( msg );
			return;
		}

		if (startVal > specValues(specValues.size()-1)){
			String msg = endValueStr + String::toString(startVal) + largerStr;
			logWarning( msg );
			return;
		}

		startIndex=0;
		while (specValues(startIndex)<startVal){
			startIndex++;
		}

		endIndex=specValues.size()-1;
		while (specValues(endIndex)>endVal){
			endIndex--;
		}
	}
	//Descending case
	else {
		if (endVal < specValues(specValues.size()-1)){
			String msg = startValueStr + String::toString(endVal) + smallerStr;
			logWarning( msg );
			return;
		}
		if (startVal > specValues(0)){
			String msg = endValueStr + String::toString(startVal) + largerStr;
			logWarning( msg );
			return;
		}

		startIndex=0;
		while (specValues(startIndex)>endVal){
			startIndex++;
		}
		endIndex=specValues.size()-1;
		while (specValues(endIndex)<startVal){
			endIndex--;
		}
	}

	channelStartIndex = startIndex;
	channelEndIndex = endIndex;
}

void ProfileTaskFacilitator::setCollapseVals(const Vector<Float> &spcVals){

	*logger << LogOrigin("QtProfile", "setCollapseVals");

	if ( spcVals.size() < 1 ){
		String message = "No spectral values! Can not set collapse values!";
		logWarning( message );
		return;
	}

	// grab the start and end value
	Float valueStart=spcVals(0);
	Float valueEnd  =spcVals(spcVals.size()-1);

	Bool ascending(True);
	if (valueStart > valueEnd){
		ascending=False;
	}

	// convert to QString
	QString startQStr =  QString((String::toString(valueStart)).c_str());
	QString endQStr   =  QString((String::toString(valueEnd)).c_str());

	// make sure the values are valid
	int pos=0;
	QDoubleValidator validator( -1.0e+32, 1.0e+32, 10, NULL );
	if (validator.validate(startQStr, pos) != QValidator::Acceptable){
		String msg = String("Spectral value not correct: ") + String::toString(valueStart);
		logWarning( msg );
		return;
	}
	if (validator.validate(endQStr, pos) != QValidator::Acceptable){
		String msg = String("Spectral value not correct: ") + String::toString(valueEnd);
		logWarning( msg );
		return;
	}


	// set the values into the fields
	String msg;
	if (ascending){
		setRange( valueStart, valueEnd );
		//startValue->setText(startQStr);
		//endValue->setText(endQStr);
		msg = String::toString(valueStart) + " and " + String::toString(valueEnd);
	}
	else{
		setRange( valueEnd, valueStart);
		//startValue->setText(endQStr);
		//endValue->setText(startQStr);
		msg = String::toString(valueEnd) + " and " + String::toString(valueStart);
	}

	// give feedback
	msg = String("Initial collapse values set: ") + msg;
	*logger << LogIO::NORMAL << msg << LogIO::POST;
}



ProfileTaskFacilitator::~ProfileTaskFacilitator() {
	delete validator;
}

} /* namespace casa */
