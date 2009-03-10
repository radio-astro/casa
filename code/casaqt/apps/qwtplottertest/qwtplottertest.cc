//# qwtplottertest.cc: Application to show/test qwt plotter features.
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
//# $Id: $
#ifdef AIPS_HAS_QWT

#include <casaqt/PlotterImplementations/PlotterImplementations.h>

#include <stdlib.h>
#include <time.h>

#include <casa/namespace.h>

namespace casa {

// Set up a simple event handler to:
// 1) on a double click, add a point at that location
//    => if the symbol for drawing the points is a character, increment the
//       character for each new point
// 2) on a right click, clear all added points
// 3) on a keyboard command, print out the received command
class ExampleHandler : public PlotClickEventHandler,
                       public PlotKeyEventHandler {
public:
    ExampleHandler(PlotFactoryPtr factory, PlotSymbolPtr symbol) :
            m_factory(factory), m_symbol(symbol) { }
    
    ~ExampleHandler() { }
    
    void handleClick(const PlotClickEvent& event) {
        if(event.button() == PlotClickEvent::DOUBLE) {
            PlotCoordinate coord = event.where();
            if(coord.system() != PlotCoordinate::WORLD)
                coord = event.canvas()->convertCoordinate(coord,
                                        PlotCoordinate::WORLD);
            
            PlotPointPtr point = m_factory->point(coord.x(), coord.y());
            point->setSymbol(m_symbol);
            event.canvas()->plotPoint(point);
            
            if(m_symbol->isCharacter()) {
                char c = m_symbol->symbolChar();
                c++;
                m_symbol->setSymbol(c);
            }
        } else if(event.button() == PlotClickEvent::CONTEXT)
            event.canvas()->clearPoints();
    }
    
    void handleKey(const PlotKeyEvent& event) {
        cout << event.toString() << endl;
    }
    
private:
    PlotFactoryPtr m_factory; // Factory
    PlotSymbolPtr m_symbol;   // Symbol (will be incremented if character)
};

}

int main(int argc, char** argv) {
    String flag;
    if(argc > 1) {
        flag = String(argv[1]);
        if(flag.size() >= 2 && flag[0] == '-' && flag [1] == '-')
            flag = flag.substr(2);
        flag.downcase();
    }
    
    const String FLAG_HELP = "help", FLAG_SIMPLE = "simple",
          FLAG_ERROR = "error", FLAG_MASKED = "masked", FLAG_BAR = "bar",
          FLAG_HISTOGRAM = "histogram", FLAG_RASTER = "raster",
          FLAG_SPECTROGRAM = "spectrogram";
    const String DEFAULT_FLAG = FLAG_ERROR;
    
    if(flag == FLAG_HELP) {
        cout << "usage: " << argv[0] << " [flag]\n";
        cout << "Available flags (no flag defaults to "<<DEFAULT_FLAG<<"):\n";
        cout << "--" << FLAG_SIMPLE << "     \tFor simple plotter.\n";
        cout << "--" << FLAG_ERROR << "      \tFor error bar plot.\n";
        cout << "--" << FLAG_MASKED << "     \tFor masked scatter plot.\n";
        cout << "--" << FLAG_BAR << "        \tFor bar plot.\n";
        cout << "--" << FLAG_HISTOGRAM << "  \tFor histogram plot.\n";
        cout << "--" << FLAG_RASTER << "     \tFor raster plot.\n";
        cout << "--" << FLAG_SPECTROGRAM << "\tFor spectrogram plot.\n";
        cout << "--" << FLAG_HELP << "       \tPrints this message and exits.";
        cout << endl;
        return 0;
    }
    
    if(!flag.empty() && flag != FLAG_SIMPLE && flag != FLAG_ERROR &&
       flag != FLAG_MASKED && flag != FLAG_BAR && flag != FLAG_HISTOGRAM &&
       flag != FLAG_RASTER && flag != FLAG_SPECTROGRAM) {
        cout << "Unknown flag: \"" << flag << "\", using default ("
             << DEFAULT_FLAG << ")." << endl;
        flag = DEFAULT_FLAG;
    } else if(flag.empty()) flag = DEFAULT_FLAG;
    
    // FOR SIMPLE PLOTTER
    if(flag == FLAG_SIMPLE) {
        SimplePlotterPtr plotter = simplePlotter(Plotter::QWT);    
        plotter->setWindowTitle("Simple qwt plotter test");
        
        // Hold drawing until we've added everything.
        plotter->holdDrawing();
    
        // Get some data
        int from = -25, to = 25;
        unsigned int n = to - from + 1;
        Vector<int> x(n), y(n);
        for(int i = from; i <= to; i++) {
            x[i - from] = i;
            y[i - from] = i * i * i;
        }
        
        plotter->plotxy(x, y);
        
        // Plot some random points
        srand(time(NULL));
        n = 10;
        int minX = x[0], minY = y[0];
        int maxX = x[x.size() - 1], maxY = y[y.size() - 1];
        int randX, randY;
        
        plotter->setSymbol(PlotSymbol::CIRCLE, "green", 8);
        
        for(unsigned int i = 0; i < n; i++) {
            randX = (rand() % (maxX - minX)) + minX;
            randY = (rand() % (maxY - minY)) + minY;
            plotter->plotPoint(randX, randY);
        }
        
        plotter->showCartesianAxes();
        
        plotter->annotation(5, 10000, "Test annotation.");
        plotter->rectangle(5, -5000, 15, -10000);
        plotter->ellipse(-15, 10000, 5, 2500);
        plotter->arrow(0, 0, 5, 5000);
        
        // Release drawing.
        plotter->releaseDrawing();
        
        // Export a screenshot
        plotter->exportPS(plotter->fileChooserDialog("Export to PS file"));
        
        return plotter->execLoop();
    }

    
    // FOR NORMAL INTERFACE
    else {
        // Get the qwt factory
        PlotFactoryPtr factory = plotterImplementation(Plotter::QWT);
    
        // Set up a plotter window with a single canvas
        PlotterPtr plotter = factory->plotter("Qwt Plotter Test");
        
        plotter->showDefaultPanel(Plotter::HAND_TOOLS);
        plotter->showDefaultPanel(Plotter::EXPORT_TOOLS);
        
        // Get the canvas
        PlotCanvasPtr canvas = plotter->canvas();
        
        // Hold canvas drawing until we've added everything
        canvas->holdDrawing();
        
        // Change canvas title and font
        canvas->setTitle("Random plotting stuff on a canvas");
        PlotFontPtr font = canvas->titleFont();
        font->setColor("60A0C0");
        font->setPointSize(14);
        font->setBold();
        canvas->setTitleFont(font);
        
        // Change background to a light grey
        PlotAreaFillPtr area = canvas->background();
        area->setColor("EEEEEE");
        canvas->setBackground(area);
        
        // Register example handler
        ExampleHandler handler(factory, factory->symbol('a'));
        // pass false to smart pointers so that it won't be deleted prematurely
        canvas->registerClickHandler(PlotClickEventHandlerPtr(&handler,false));
        canvas->registerKeyHandler(PlotKeyEventHandlerPtr(&handler, false));
    
    
        // Set up a scatter plot with error bars
    
        // Plot data (x^2)
        int from = -25, to = 25;
        unsigned int n = to - from + 1;
    
        Vector<float> x(n);
        Vector<float> y(n);
    
        for(int i = from; i <= to; i++) {
            x[i - from] = i;
            y[i - from] = i * i;
            if(i < 0) y[i - from] = -y[i - from];
        }
    
        // Set up some random errors
        srand(time(NULL));
        Vector<float> xError(n);
        Vector<float> yError(n);
        double rnum;
        for(unsigned int i = 0; i < n; i++) {
            rnum = (rand() % 10) / 100.0;
            xError[i] = x[i] * rnum;
            if(xError[i] < 0) xError[i] = -xError[i];
            
            rnum = (rand() % 10) / 100.0;
            yError[i] = y[i] * rnum;
            if(yError[i] < 0) yError[i] = -yError[i];
        }

        // Generate the plot
        PlotErrorDataPtr data = factory->data(x, y, xError, xError, yError,
                yError, false);// make sure data isn't deleted by smart pointer
        ErrorPlotPtr plot = factory->errorPlot(data, "x<sup>2</sup> plot");
        
        // Customize the plot
        
        // Solid blue line between points, a different blue for error lines
        plot->setLine("60A0C0");
        plot->setErrorLine("307090");
        
        // 8x8 blue diamond symbol
        PlotSymbolPtr symbol = factory->symbol(PlotSymbol::DIAMOND);
        symbol->setSize(8, 8);
        symbol->setAreaFill("9999EE");
        plot->setSymbol(symbol);
        
        // Draw the plot as necessary.
        if(flag == FLAG_ERROR) canvas->plotItem(plot);    
        
    
        // Add a random mask.
        Vector<bool> mask(n, false);
        for(unsigned int i = 0; i < n; i++) mask[i] = (rand() % 2) == 1;
        PlotMaskedPointDataPtr maskedData = factory->data(x, y, mask, false);
        MaskedScatterPlotPtr maskedPlot = factory->maskedPlot(maskedData,
                "Masked x<sup>2</sup> plot");
        maskedPlot->setSymbol(plot->symbol());
        maskedPlot->setMaskedSymbol(PlotSymbol::SQUARE);
    
        // Draw the masked plot as necessary.
        if(flag == FLAG_MASKED) canvas->plotItem(maskedPlot);
    
        
        // Set up rectangle from (5, 250) to (20, -200)
        PlotCoordinate c1(5, 250), c2(20, -200);
        PlotShapePtr rect = factory->shapeRectangle(c1, c2);
        rect->setLine("000000", PlotLine::DASHED, 2.0);// 2px black dashed line
        rect->setAreaFill("60A0C0", PlotAreaFill::MESH3); // meshed blue        
        if(flag == FLAG_ERROR || flag == FLAG_MASKED) canvas->plotItem(rect);      
        
        // Set up annotation with an outline and background
        c1 = PlotCoordinate(10, -200);
        PlotAnnotationPtr annot= factory->annotation("Rotated annotation.",c1);
        annot->setOutline("black");
        annot->setBackground("339933", PlotAreaFill::MESH3);
        annot->setOrientation(30);
        if(flag == FLAG_ERROR || flag == FLAG_MASKED) canvas->plotItem(annot);
        
        // Set up lines at x = 10 and y = 200
        PlotShapePtr pline = factory->shapeLine(200, Y_LEFT);
        pline->setLine("60A0C0", PlotLine::DOTTED, 2);
        if(flag == FLAG_ERROR || flag == FLAG_MASKED) canvas->plotItem(pline);
        
        pline = factory->shapeLine(10, X_BOTTOM);
        pline->setLine("106080", PlotLine::DOTTED, 2);
        if(flag == FLAG_ERROR || flag == FLAG_MASKED) canvas->plotItem(pline);
        
        // Set up arrow from (0, 0) to (-10, -600)
        c1 = PlotCoordinate(0, 0);
        c2 = PlotCoordinate(-10, -600);
        PlotShapePtr arrow = factory->shapeArrow(c1, c2);
        if(flag == FLAG_ERROR || flag == FLAG_MASKED) canvas->plotItem(arrow);
        
        // Set up ellipse centered on (-10, 400)
        c1 = PlotCoordinate(-10, 400);
        c2 = PlotCoordinate(10, 200);
        PlotShapeEllipsePtr ellipse = factory->shapeEllipse(c1, c2);
        ellipse->setAreaFill("669933", PlotAreaFill::MESH3);
        if(flag== FLAG_ERROR || flag== FLAG_MASKED) canvas->plotItem(ellipse);
        
        
        // Set up a histogram for some random data
        
        // Get random data
        Vector<double> v(n);
        // make sure we get the whole range
        v[0] = 0;
        v[n - 1] = n - 1;
        // assign random to the rest
        for(unsigned int i = 1; i < n - 1; i++)
            v[i] = rand() % n;
    
        // Get histogram plot with ten bins
        BarPlotPtr hist = factory->histogramPlot(factory->singleData(v), 10);
        if(flag == FLAG_HISTOGRAM) canvas->plotItem(hist);
        
        
        // Set up a bar plot for some random data
        
        Vector<int> by(n);
        for(unsigned int i = 0; i < n; i++) by[i] = rand() % n;
        
        BarPlotPtr bar = factory->barPlot(factory->data(by));
        if(flag == FLAG_BAR) canvas->plotItem(bar);
        
      
        // Set up a raster plot with contours
        
        // Raster plot data
        Matrix<uInt> rdata(5, 5, 0x336699);
        
        // row x column indexing
        rdata(0, 0) = rdata(1, 0) = rdata(2, 0) = rdata(3, 0) = rdata(4, 0) =
            rdata(2, 1) = rdata(0, 2) = rdata(1, 2) = rdata(2, 2) = rdata(3, 2) =
            rdata(4, 2) = rdata(0, 4) = rdata(1, 4) = rdata(2, 4) = rdata(4, 4) =
                0x000000;
        
        RasterPlotPtr raster = factory->rasterPlot(factory->data(rdata));
        vector<double> contours(1, 0x000000);
        raster->setContourLines(contours);
        raster->setLine("green", PlotLine::SOLID, 2.0);
        
        if(flag == FLAG_RASTER) {
            canvas->plotItem(raster);
            canvas->showColorBar();
        }
        
        
        // Set up a spectrogram with contours
        
        // Get some spectrogram data
        Matrix<double> sdata(500, 500, 0);
        const double c = 10;
        for(unsigned int i = 0; i < sdata.nrow(); i++) {
            for(unsigned int j = 0; j < sdata.ncolumn(); j++) {
                double x = (((double)i / sdata.nrow()) * 2) - 1;
                double y = (((double)j / sdata.ncolumn()) * 2) - 1;
                
                if(x < 0) x = -x;
                if(y < 0) y = -y;
                x = 1 - x; y = 1 - y;
    
                sdata(i, j) = (x * c/2) + (y * c/2);
            }
        }
        
        // Set up data ranges, contours, etc.
        RasterPlotPtr spect = factory->spectrogramPlot(factory->data(sdata));
        spect->setXRange(-1, 1); spect->setYRange(-1, 1);
        contours.resize(((unsigned int)c) / 2);
        for(unsigned int i = 0; i < contours.size(); i++) contours[i] = i * 2 + 1;
        spect->setContourLines(contours);
        spect->setLine("black", PlotLine::SOLID, 1.0);
        
        if(flag == FLAG_SPECTROGRAM) {
            canvas->plotItem(spect);
            canvas->showColorBar();
        }
        
        
        // Customize canvas some more
        
        if(flag == FLAG_ERROR || flag == FLAG_MASKED) {
            // Set axes titles
            canvas->setAxisLabel(X_BOTTOM, "x");
            canvas->setAxisLabel(Y_LEFT, "x<sup>2</sup>");
    
            // Show major grid lines
            canvas->setGridMajorLine("AAAAAA", PlotLine::DASHED);
        }
        
        // Release canvas drawing
        canvas->releaseDrawing();
        
        return factory->execLoop();
    }
}

#else

// for non-qwt to be able to compile
int main(int argc, char** argv) { return 0; }

#endif
