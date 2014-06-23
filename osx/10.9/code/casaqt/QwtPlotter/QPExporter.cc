//# Copyright (C) 2008
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

#include "QPExporter.h"
#include <graphics/GenericPlotter/PlotCanvas.h>
#include <casaqt/QwtPlotter/QPPlotter.qo.h>
#include <casaqt/QwtPlotter/QPExportCanvas.h>
#include <casaqt/QwtPlotter/QPOptions.h>
#include <qwt_text.h>

namespace casa {

const String QPExporter::EXPORT_NAME = "export";
const String QPExporter::CLASS_NAME = "QPExporter";

QPExporter::QPExporter() {
	// TODO Auto-generated constructor stub

}

void QPExporter::findGridProperties( QPExportCanvas* grabCanvas, QPPlotter* grabPlotter,
		Int& width, Int& height, Int& gridRows, Int& gridCols ){
	//Figure out the grid size
	width = 0;
	if ( grabPlotter != NULL ){
		width = grabPlotter->width();
	}
	else if( grabCanvas != NULL ){
	    width = grabCanvas->canvasWidth();
	}

	height = 0;
	if ( grabPlotter != NULL ){
	   height = grabPlotter->height();
	}
	if (grabCanvas != NULL ){
	   height = grabCanvas->canvasHeight();
	}
	gridRows = 1;
	gridCols = 1;
	if ( grabPlotter != NULL ){
		gridRows = grabPlotter->getRowCount();
	    gridCols = grabPlotter->getColCount();
	}
}

bool QPExporter::exportPostscript(
        const PlotExportFormat& format,
        vector<QPExportCanvas*> &qcanvases,
        QPExportCanvas* grabCanvas, QPPlotter* grabPlotter){

    // Set resolution.
    QPrinter::PrinterMode mode = QPrinter::ScreenResolution;
    if(format.resolution == PlotExportFormat::HIGH)
        mode = QPrinter::HighResolution;

    // Set file.
    QPrinter printer(mode);
    switch (format.type)   {
        case PlotExportFormat::PDF:
                printer.setOutputFormat(QPrinter::PdfFormat);
                break;
        case PlotExportFormat::PS:
                printer.setOutputFormat(QPrinter::PostScriptFormat);
                break;
        default: {}
    }

    printer.setOutputFileName(format.location.c_str());


    // Set output settings.
    if(format.dpi > 0) printer.setResolution(format.dpi);
        printer.setColorMode(QPrinter::Color);

    Int width = 0;
    Int height = 0;
    Int gridRows = 1;
    Int gridCols = 1;
    findGridProperties( grabCanvas, grabPlotter, width, height, gridRows, gridCols );

    Bool wasCanceled = false;

    // High resolution, or format size larger than widget.
    QImage image=produceHighResImage(format, qcanvases, width, height, gridRows,
        		  gridCols, wasCanceled);

    QPainter painter(&printer);
    painter.drawImage(QPoint(0,0), image );
    painter.end();

    return !wasCanceled;
}



/* Formerly named exportHelper()  */
bool QPExporter::exportCanvases  ( vector<QPExportCanvas*>& canvases,
        const PlotExportFormat& format, PlotCanvas* grabCanvas, QPPlotter* grabPlotter){
    if (format.location.empty() || ((format.type == PlotExportFormat::JPG ||
       format.type == PlotExportFormat::PNG) && (grabCanvas == NULL &&
       grabPlotter == NULL)))
        return false;

    QPExportCanvas* exportCanvas = NULL;
    if ( grabCanvas != NULL ){
    	exportCanvas = dynamic_cast<QPExportCanvas*>(grabCanvas);
    }

    // Compile vector of unique, non-null QPExportCanvases.
    vector<QPExportCanvas*> qcanvases;
    for(unsigned int i = 0; i < canvases.size(); i++) {
        if(canvases[i] == NULL){
        	continue;
        }

        QPExportCanvas* canv = canvases[i];
        bool found = false;
        for(unsigned int j = 0; !found && j < qcanvases.size(); j++){
            if(qcanvases[j] == canv){
            	found = true;
            }
        }
        if(!found){
        	qcanvases.push_back(canv);
        }
    }
    if(qcanvases.size() == 0) return false;

    // Compile vector of unique, non-null loggers for export event.
    vector<PlotLoggerPtr> loggers;
    PlotLoggerPtr logger;
    for(unsigned int i = 0; i < qcanvases.size(); i++) {
        logger = qcanvases[i]->logger();
        if(logger.null()) continue;

        bool found = false;
        for(unsigned int j = 0; !found && j < loggers.size(); j++)
            if(loggers[j] == logger) found = true;
        if(!found) loggers.push_back(logger);
    }


    // Start logging.
    for(unsigned int i = 0; i < loggers.size(); i++){
        loggers[i]->markMeasurement(CLASS_NAME, EXPORT_NAME,
                                    PlotLogger::EXPORT_TOTAL);
    }

    bool ret = false;
    switch (format.type){
        case PlotExportFormat::JPG:
        case PlotExportFormat::PNG:
                ret=exportToImageFile(format, qcanvases, exportCanvas, grabPlotter);
                break;
        case PlotExportFormat::PS:
        case PlotExportFormat::PDF:
                ret=exportPostscript(format, qcanvases, exportCanvas, grabPlotter );
                break;
        case PlotExportFormat::TEXT:
        case PlotExportFormat::NUM_FMTS:
        		qDebug() << "Exporting canvas in format: "<< format.type<<" is not supported.";
        		break;

    }


    // End logging.
    for(unsigned int i = 0; i < loggers.size(); i++)
        loggers[i]->releaseMeasurement();

    return ret;
}



bool QPExporter::exportToImageFile(
        const PlotExportFormat& format,
        vector<QPExportCanvas*> &qcanvases,
        QPExportCanvas* grabCanvas,
        QPPlotter* grabPlotter){

    QImage image;

    Int width = 0;
    Int height = 0;
    Int gridRows = 1;
    Int gridCols = 1;
    findGridProperties( grabCanvas, grabPlotter, width, height, gridRows, gridCols );

    // Remember the current background color, used for on-screen GUI
    // We want to temporarily change this to white for making the image file.
    // Later we will restore this color.
    PlotAreaFillPtr normal_background;
    if(grabCanvas != NULL)
        normal_background = grabCanvas->background();


    // Just grab the widget if: 1) screen resolution, or 2) high resolution
    // but size is <= widget size or not set.
    bool wasCanceled = false;
    if(format.resolution == PlotExportFormat::SCREEN)    {
        image=grabCanvas->grabImageFromCanvas(format);
    }
    else {
        // High resolution, or format size larger than widget.
        image=produceHighResImage(format, qcanvases, width, height, gridRows, gridCols, wasCanceled);
    }

    // Set DPI.
    if(!wasCanceled && format.dpi > 0) {
        // convert dpi to dpm
        int dpm = QPOptions::round((format.dpi / 2.54) * 100);
        image.setDotsPerMeterX(dpm);
        image.setDotsPerMeterY(dpm);
    }


    // Set output quality.
    bool hires = (format.resolution == PlotExportFormat::HIGH);
    int quality;    // see QImage.save official documentation for its meaning
    switch (format.type)
    {
        case PlotExportFormat::JPG:
                // JPEG quality ranges from 0 (crude 8x8 blocks) to 100 (best)
                quality= (hires)? 99: 95;   // experimental; need user feedback
                break;

        case PlotExportFormat::PNG:
                // Compression is lossless.  "quality" is number of deflations.
                // First one does great compression.  More buy only a small %.
                // Set to -1 for no compression.
                quality=1;
                break;

        default:
                quality=-1;  // no compression of undefined/unknown formats
    }

    // Save to file.
    bool save_ok;
    save_ok= image.save(format.location.c_str(),
          PlotExportFormat::exportFormat(format.type).c_str(),
          quality);

    // Restore background color
    if(grabCanvas != NULL)
        grabCanvas->setBackground(normal_background);

    return !wasCanceled && !image.isNull() && save_ok;
}

/* static */

int QPExporter::findAxisWidth( vector<QPExportCanvas*> &qcanvases ){
	int canvasCount = qcanvases.size();
	int width = 0;
	for ( int i = 0; i < canvasCount; i++ ){
		if ( qcanvases[i]->isAxis()){
			if ( qcanvases[i]->isVertical()){
				width = qcanvases[i]->canvasWidth();
				break;
			}
		}
	}
	return width;
}

int QPExporter::findAxisHeight( vector<QPExportCanvas*> &qcanvases ){
	int canvasCount = qcanvases.size();
	int height = 0;
	for ( int i = 0; i < canvasCount; i++ ){
		if ( qcanvases[i]->isAxis()){
			if ( !qcanvases[i]->isVertical()){
				height = qcanvases[i]->canvasHeight();
				break;
			}
		}
	}
	return height;
}

int QPExporter::getCanvasCount( vector<QPExportCanvas*> &qcanvases ){
	int canvasCount = 0;
	int canvasSize = qcanvases.size();
	for ( int i = 0; i < canvasSize; i++ ){
		if ( !qcanvases[i]->isAxis() ){
			canvasCount++;
		}
	}
	return canvasCount;
}

QImage QPExporter::produceHighResImage(
        const PlotExportFormat& format,
        vector<QPExportCanvas*> &qcanvases,
        int width, int height,
        int rowCount, int colCount,
        bool &wasCanceled ){

    // make sure both width and height are set
    if (format.width > 0){
    	width  = format.width;
    }
    if (format.height > 0) {
    	height = format.height;
    }

    int widgetWidth  = width/colCount;
    int widgetHeight = height/rowCount;
    int externalAxisHeight = 0;
    int externalAxisWidth = 0;
    int canvasSize = qcanvases.size();
    bool topAxis = false;
    bool leftAxis = false;
    if ( rowCount * colCount < canvasSize){
    	//We are using external axes. Subtract off the size of the external axes
    	//before computing the widget width;
    	externalAxisWidth = findAxisWidth( qcanvases );
    	widgetWidth = (width - externalAxisWidth )/ colCount;
    	externalAxisHeight = findAxisHeight( qcanvases );
    	widgetHeight = (height - externalAxisHeight)/rowCount;
    	if ( qcanvases[0]->isAxis()){
    		if ( !qcanvases[0]->isVertical()){
    			topAxis = true;
    			//We could still have a left axis after we have cycled through
    			//all the top axes.
    			for ( int j = 0; j < canvasSize; j++ ){
    				if ( qcanvases[j]->isAxis() ){
    					if ( qcanvases[j]->isVertical()){
    						leftAxis = true;
    						break;
    					}
    				}
    				else {
    					//We didn't hit a left axis before proceeding to the plots.
    					break;
    				}
    			}
    		}
    		else {
    			leftAxis = true;
    		}
    	}
    }

    QImage image = QImage(width, height, QImage::Format_ARGB32);

    // Fill with background color.
    QPExportCanvas* canv = qcanvases[0];
    image.fill(canv->palette().color(canv->backgroundRole()).rgba());
    image.fill((uint)(-1));

    // Change canvas' background color to white for a nice bright clean image file
    PlotFactory* f = canv->implementationFactory();
    PlotAreaFillPtr paf = f->areaFill(String("#ff77ff"));
    delete f;
    paf->setColor("#FFFFFF");

    // Print each canvas.
    QPainter painter(&image);

    QRect imageRect(0, 0, width, height);
    int canvasCount = getCanvasCount( qcanvases );
    int rowDivisor = canvasCount / rowCount;
    int colDivisor = colCount;
    if ( leftAxis ){
    	colDivisor = colCount+1;
    }

    int canvasIndex = 0;
    for (int i = 0; i < canvasSize; i++) {

    	int rowIndex = canvasIndex / rowDivisor;
    	int axisWidth = externalAxisWidth;
    	int axisHeight = externalAxisHeight;

    	int colIndex = i % colDivisor;

    	//Plot
    	if ( !qcanvases[i]->isAxis()){
    	    canvasIndex++;
    	    if ( !topAxis ){
    	    	axisHeight = 0;
    	    }

    	    if ( !leftAxis ){
    	    	axisWidth = 0;
    	    }

    	    if ( topAxis && leftAxis ){
    	    	colIndex = colIndex + 1;
    	    }
    	}
    	//Axis
    	else {
    		if ( !topAxis ){
    			if ( qcanvases[i]->isVertical()){
    				axisHeight = 0;
    			}
    		}
    		if ( !leftAxis ){
    			if ( !qcanvases[i]->isVertical()){
    				axisWidth = 0;

    			}
    			else {
    				//External right axis goes in last column.
    				colIndex = colCount;
    				rowIndex = rowIndex - 1;
    			}
    		}
    		else {
    			//Left axes must go in column 0
    			if ( qcanvases[i]->isVertical()){
    				colIndex = 0;
    			}
    		}
    	}

        wasCanceled = qcanvases[i]->print(  &painter, paf, widgetWidth, widgetHeight, axisWidth, axisHeight, rowIndex, colIndex, imageRect );
        if ( wasCanceled ){
        	break;
        }
    }

    return image;
}
bool QPExporter::exportCanvas(PlotCanvas* canvas, const PlotExportFormat& format) {
	vector<QPExportCanvas*> canvases;
	if ( canvas != NULL ){
		QPExportCanvas* exportCanvas = dynamic_cast<QPExportCanvas*>(canvas);
		canvases.push_back( exportCanvas );
	}
    return exportCanvases(canvases, format, canvas, NULL);
}

bool QPExporter::exportPlotter(QPPlotter* plotter, const PlotExportFormat& fmt) {
    vector<QPExportCanvas*> canvases = plotter->getGridComponents();
    return exportCanvases(canvases, fmt, NULL, plotter);
}

QPExporter::~QPExporter(){
}

} /* namespace casa */
