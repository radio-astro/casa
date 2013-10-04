/*
 * Client.cc
 *
 *  Created on: Jul 30, 2013
 *      Author: slovelan
 */
#include <plotms/Client/Client.h>

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
}


