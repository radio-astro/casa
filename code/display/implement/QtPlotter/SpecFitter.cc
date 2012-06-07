/*
 * SpecFitter.cpp
 *
 *  Created on: May 16, 2012
 *      Author: slovelan
 */

#include "SpecFitter.h"
#include <display/QtPlotter/QtCanvas.qo.h>
#include <display/QtPlotter/SpecFitMonitor.h>
#include <images/Images/ImageInterface.h>
#include <casa/Logging/LogIO.h>

namespace casa {

SpecFitter::SpecFitter() : pixelCanvas( NULL ), specFitMonitor( NULL ), logger( NULL ) {
	// TODO Auto-generated constructor stub

}
void SpecFitter::setCanvas( QtCanvas* pCanvas ){
	pixelCanvas = pCanvas;
}

void SpecFitter::setSpecFitMonitor( SpecFitMonitor* monitor ){
	specFitMonitor = monitor;
}

void SpecFitter::setLogger( LogIO* log ){
	logger = log;
}

QString SpecFitter::getFileName(){
	return specFitMonitor->getFileName();
}

void SpecFitter::logWarning(String msg ){
	*logger << LogIO::WARN << msg << LogIO::POST;
}
void SpecFitter::postStatus( String status ){
	specFitMonitor->postStatus( status );
}
Vector<Float> SpecFitter::getXValues() const {
	return specFitMonitor -> getXValues();
}
Vector<Float> SpecFitter::getYValues() const {
	return specFitMonitor -> getYValues();
}
Vector<Float> SpecFitter::getZValues() const {
	return specFitMonitor -> getZValues();
}

QString SpecFitter::getYUnit() const {
	return specFitMonitor -> getYUnit();
}

QString SpecFitter::getYUnitPrefix() const {
	return specFitMonitor -> getYUnitPrefix();
}
String SpecFitter::getXAxisUnit() const {
	return specFitMonitor -> getXAxisUnit();
}

void SpecFitter::plotMainCurve(){
	specFitMonitor -> plotMainCurve();
}

const ImageInterface<Float>* SpecFitter::getImage() const{
	return specFitMonitor -> getImage();
}

const String SpecFitter::getPixelBox() const {
	return specFitMonitor -> getPixelBox();
}

SpecFitter::~SpecFitter() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
