//# Copyright (C) 2009
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
//# $Id: $
#include <plotms/Client/Client.h>
#include <QDebug>

namespace casa {
void Client::holdDrawing(){
	 vector<PlotCanvasPtr> canvases = currentCanvases();
	 for(unsigned int i = 0; i < canvases.size(); i++) {
	      canvases[i]->holdDrawing();
	 }
}

void Client::releaseDrawing() {
    vector<PlotCanvasPtr> canvases = currentCanvases();
    for(unsigned int i = 0; i < canvases.size(); i++) {
        canvases[i]->releaseDrawing();
    }
}

bool Client::allDrawingHeld() const {
    if(itsPlotter_.null() || itsPlotter_->canvasLayout().null()) {
    	return false;
    }
    vector<PlotCanvasPtr> canvases= itsPlotter_->canvasLayout()->allCanvases();
    for(unsigned int i = 0; i < canvases.size(); i++) {
        if(!canvases[i].null() && !canvases[i]->drawingIsHeld()) {
        	return false;
        }
    }
    return true;
}

// See showGUI() and execLoop().
 int Client::showAndExec(bool show ) {
     showGUI(show);
     return execLoop();
 }

PlotFactoryPtr Client::getPlotFactory() {
	return itsFactory_;
}



PlotSymbolPtr Client::createSymbol(const String& descriptor, Int size,
		const String& color, const String& fillPattern, bool outline ){
	return itsFactory_->createSymbol(descriptor, size, color, fillPattern, outline);
}

PlotSymbolPtr Client::createSymbol( const PlotSymbolPtr& copy ){
	return itsFactory_->symbol( copy );
}

bool Client::isVisible(PlotCanvasPtr& canvas ){
	return itsPlotter_->isVisible( canvas );
}

PlotLoggerPtr Client::getLogger(){
	return itsPlotter_->logger();
}

void Client::setOperationCompleted( bool /*completed*/ ){

}

void Client::setCanvasCachedAxesStackImageSize(int width, int height ){
	itsPlotter_->setCanvasCachedAxesStackImageSize( width, height );
}

PlotterPtr Client::getPlotter() {
	return itsPlotter_;
}

vector<PlotCanvasPtr> Client::currentCanvases() {
	return itsPlotter_->canvasLayout()->allCanvases();
}

bool Client::exportToFormat(const PlotExportFormat& format){
	bool success = itsPlotter_->exportPlot( format );
	return success;
}

void Client::setCommonAxes( bool commonX, bool commonY ){
	itsPlotter_->setCommonAxisX( commonX );
	itsPlotter_->setCommonAxisY( commonY );
}

bool Client::isCommonAxisX() const {
	return itsPlotter_->isCommonAxisX();
}

bool Client::isCommonAxisY() const {
	return itsPlotter_->isCommonAxisY();
}

void Client::setAxisLocation( PlotAxis locationX, PlotAxis locationY ){
	itsPlotter_->setAxisLocation( locationX, locationY );
}

PlotAxis Client::getAxisLocationX() const {
	return itsPlotter_->getAxisLocationX();
}

PlotAxis Client::getAxisLocationY() const {
	return itsPlotter_->getAxisLocationY();
}

}


