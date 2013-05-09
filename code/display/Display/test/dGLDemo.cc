//# dGLDemo.cc: Demo various GL canvas routines.
//# Copyright (C) 2001,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

// This program uses several GLPixelCanvas routines:
//	Line and point drawing with different styles, sizes and colors.
//	Images
//	Display lists.
//	Tracing with user supplied notations.


// The program will work with either PseudoColor or TrueColor visuals.
//
// This was derived from dMultichannelRaster's skeleton.
#if defined(OGL)
//# Includes
#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Inputs/Input.h>
#include <GL/gl.h>
#include <display/Display/SimpleWorldGLCanvasApp.h>
#include <display/Display/GLPixelCanvas.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayEvents/WCRefreshEH.h>
#include <display/DisplayEvents/WCRefreshEvent.h>

//# THIS IS A STAND-ALONE APPLICATION
#include <display/Display/StandAloneDisplayApp.h>
#include <casa/namespace.h>
// Program name.
static const char *PNAME=NULL;

//////////////////////////////////////////////////////////////////////////
static const char HELP[] = "\
Switch	values			Description\n\
-mode	rgb | index | hsv	What type of window to create. Depending on\n\
				the X server, any should work although hsv\n\
				may not be handled correctly. Default: rgb\n\
-refresh t | f			Should the GLCanvas handle refreshing. Mainly\n\
				used to show what happens when it doesn't.\n\
				Default: t\n\
-small	 t | f			If t, just a couple of images are displayed.\n\
-trace	 t | f			Display tracing information. Default: f\n\
-delay	 t | f			T(default): buffer trace output.\n\
-uselogio t | f			T(default): Trace output goes to logio.\n\
				F: Trace output goes to cout.\n\
				(Setting delay or uselogio to F sets trace\n\
				 to T).\n\
				If f:\n\
\n\
An image is displayed in the lower left corner.\n\
Above it is a row of four rectangles containing:\n\
	1) A rectangle\n\
	2) An octagon\n\
	3) Lines and points\n\
	4) Image.\n\
 Except for the lines & points, a diagonal line is drawn, then a filled\n\
object, then an unfilled object, then another diagonal line. (The image is\n\
just redrawn). Each time the window is resized the stipple pattern and \n\
color used for the polygons is changed... The bottom line may be seen\n\
 through the solid polygon.";
//////////////////////////////////////////////////////////////////////////
// NOTE: The application may crash when a window opened in index mode is
//	resized. I haven't been able to determine if there is a problem with
//	GLPixelCanvas or nVidia's glx driver.

// A colormap is created with a gray scale in the lower part and
// various colors in the high end.

// Offsets from the high end of the color table. (nColors - offset).
static const uInt BLACK = 1;
static const uInt WHITE = 2;
static const uInt RED = 3;
static const uInt GREEN = 4;
static const uInt BLUE = 5;
static const uInt YELLOW = 6;
static const uInt GRAY = 7;
static const uInt nROColors_ = 7;	// Number of RO colors.
static uInt nColors_=0;
static uInt nColorsAvailable_=0;
static Bool doSmall_ = False;

// Set current color to the given color 'index'.
static void setColor(const uInt ROColor, GLPixelCanvas *pc) {
	pc->setColor(nColors_ - ROColor);
}

// Initialize colormap to gray scale.
static void initColormap(GLPixelCanvasColorTable *ct) {
	Float scale = 1.0/(nColorsAvailable_-1);

	for(uInt i=0; i< nColorsAvailable_; i++) {
		Float clr = i*scale;
		ct->storeColor(i, clr, clr, clr);
	}
}

class MyRefresh : public WCRefreshEH {
public:
	MyRefresh() { };
	virtual void operator()(const WCRefreshEvent &ev);
};


int main(int argc, char **argv) {
	Boolean trace = False;		// Whether to show trace info.
	Boolean delay = True;		// Whether to delay trace output.
	Boolean useLogIO = True;	// Whether to trace to LogIO (or cout).
	Boolean refresh = True;
	String traceString;
	Display::ColorModel g_colormodel;

	PNAME = argv[0];

	try {

		// Open the X connection and handle any X arguments.
		SimpleWorldGLCanvasApp::openApplication("Gldemo",
		                                        NULL, 0, &argc, argv,
		                                        NULL, NULL, 0);

		// Deal with our arguments.
		Input inputs(1);
		inputs.version("");
		inputs.create("mode", "rgb", "drawing mode: rgb, index or hsv",
		              "drawing mode");
		inputs.create("trace", "f", "trace mode: t or f", "trace mode");
		inputs.create("delay", "t", "Buffer trace: t or f", "buffer mode");
		inputs.create("uselogio", "t", "Send trace to logio: t or f", "uselogio");

		inputs.create("refresh", "t", "refresh mode: t or f", "refresh mode");
		inputs.create("small", "f", "Just Image: t or f", "Full demo");
		inputs.create("H", "f", "Help: t or f", "Display help message");
		inputs.readArguments(argc, argv);

		SimpleWorldGLCanvasApp *glapp = 0;
		WorldCanvas *wCanvas = 0;

		Boolean showhelp = inputs.getBool("H");
		if(showhelp) {
			cout << HELP << endl;
			exit(0);
		}

		String reqMode = inputs.getString("mode");
		if (reqMode == "hsv") {
			g_colormodel = Display::HSV;
		} else if (reqMode == "rgb") {
			g_colormodel = Display::RGB;
		} else if(reqMode == "index") {
			g_colormodel = Display::Index;
		} else {
			throw(AipsError("Unknown drawing mode"));
		}

		trace = inputs.getBool("trace");
		delay = inputs.getBool("delay");
		useLogIO = inputs.getBool("uselogio");
		// If either delay or useLogIO isn't its default, turn on tracing.
		if(!(delay && useLogIO))
			trace = True;

		refresh = inputs.getBool("refresh");
		doSmall_ = inputs.getBool("small");

		// make a gl canvas.
		glapp = new SimpleWorldGLCanvasApp(argv[0], g_colormodel);
		wCanvas = glapp->worldCanvas();

		if (!wCanvas) {
			throw(AipsError("Couldn't construct WorldCanvas"));
		}

		MyRefresh refresher;
		wCanvas->addRefreshEventHandler(refresher);

		if (glapp) {
			GLPixelCanvas *glpc = glapp->glPixelCanvas();
			glpc->trace(trace);
			// AIPS++ Logging interferes with trace formatting so we supply
			// a string to copy trace information to and display it all at once.
			if(delay)
				glpc->postToString(&traceString);
			glpc->postToStream(!useLogIO);

			// Allow user to see what happens if the OpenGL canvas doesn't
			// handle refresh.
			glpc->autoRefresh(refresh);

			// Setup colortable
			GLPixelCanvasColorTable *ct = glpc->glpcctbl();
			nColors_ = glpc->pcctbl()->nColors();
			nColorsAvailable_ = nColors_ - nROColors_;

			initColormap(ct);
			// Defined colors are stored at the end of the colormap.
			ct->storeColor(nColors_-BLACK, 0.0, 0.0, 0.0);
			ct->storeColor(nColors_-WHITE, 1.0, 1.0, 1.0);
			ct->storeColor(nColors_-RED, 1.0, 0.0, 0.0);
			ct->storeColor(nColors_-GREEN, 0.0, 1.0, 0.0);
			ct->storeColor(nColors_-BLUE, 0.0, 0.0, 1.0);
			ct->storeColor(nColors_-YELLOW, 1.0, 1.0, 0.0);
			ct->storeColor(nColors_-GRAY, 0.5, 0.5, 0.5);
			if(trace)
				printf( "Ncolors %d, nColorsAvailable %d VisualID: 0x%x\n",
				        nColors_, nColorsAvailable_,
				        (uInt)ct->visualInfo()->visualid);
			glapp->run();
		} else {
			throw(AipsError("An application was not built"));
		}

		if(glapp)
			delete glapp;
	} catch (const AipsError &x) {
		cerr << "Exception caught:" << endl;
		cerr << x.getMesg() << endl;
	}
}

// Pointer to a function to generate some sort of drawing.
typedef void (*FP)(Boolean filled, Float x, Float y, Float w, Float h,
                   GLPixelCanvas *pc);


// Draw a, possibly filled, rectangle.
// x0, y0	Center of object.
static void drawRectangle(Boolean Filled,
                          Float xc, Float yc, Float width, Float height,
                          GLPixelCanvas *pc) {
	Float x0 = xc - width/2.0;
	Float y0 = yc - height/2.0;

	if(Filled)
		pc->drawFilledRectangle(x0, y0, x0+width, y0+height);
	else
		pc->drawRectangle(x0, y0, x0+width, y0+height);
}

// Draw an image. (Just a greyscale).
// This is called the same way the octagon and rectangle functions
// are called.
static void drawImage(Boolean /*Filled*/,
                      Float xc, Float yc, Float width, Float height,
                      GLPixelCanvas *pc) {
	Float x0 = xc - width/2.0;
	Float y0 = yc - height/2.0;
	const uInt W = (uInt)width, H = (uInt)height;

	Matrix<uInt> image(H, W);
	Float scale = (float)(nColorsAvailable_-1)/(float)(W*H-1);
	// Fill array.
	for(uInt h = 0; h < H; h++) {
		for(uInt w = 0; w < W; w++) {
			uInt pixel = (uInt)(( w + h*W)*scale);
			if(pixel >= nColorsAvailable_) {
				printf("dGLDemo::%d > %d, w=%d, h=%d\n",
				       pixel, nColors_, w,h);
				pixel = nColorsAvailable_-1;
			}
			image(h, w) = pixel;
		}
	}
	// This is a non standard function which always uses a colormap
	// to generate pixel values whether or not the underlying visual
	// is RGB or Indexed.
	pc->drawIndexedImage(image, x0, y0);
}

// Draw a, possibly filled, octagon.
// x0, y0	Center of object.
//  drawOctagon sort of assumes width = height. Drawing may not be correct
// if this isn't the case.
static void drawOctagon(Boolean Filled,
                        Float x0, Float y0, Float width, Float height,
                        GLPixelCanvas *pc) {
	static const float S2 = 1.41421356;
	static const float SCL = S2/(2.0+S2);
	Vector<Float> xv(8), yv(8);

	if(height == 0.0)
		height = width;
	Float x=width*SCL;
	Float y=height*SCL;
	Float xs = x/S2;
	Float ys = y/S2;
	x0 -= (x/2 + xs);		// Move from center to LL.
	y0 -= (y/2 + ys);
	// The different positions.
	Float x1 = xs+x0, x2 = x1+x, x3 = x2+xs;
	Float y1 = ys+y0, y2 = y1+y, y3 = y2+ys;

	xv(0) = x0;
	yv(0) = y1;
	xv(1) = x0;
	yv(1) = y2;
	xv(2) = x1;
	yv(2) = y3;
	xv(3) = x2;
	yv(3) = y3;

	xv(4) = x3;
	yv(4) = y2;
	xv(5) = x3;
	yv(5) = y1;
	xv(6) = x2;
	yv(6) = y0;
	xv(7) = x1;
	yv(7) = y0;
	if(Filled)
		pc->drawFilledPolygon(xv, yv);
	else
		pc->drawPolygon(xv, yv);
}

// Draw three lines, one for each line style.
// Then draw some colored points.
// Then a colored line.
// The solid line is centered on x0, y0.
static void drawLines(Float xc, Float yc, Float width, Float height,
                      GLPixelCanvas *pc) {
	Float xoff = xc - width/2.0;
	Float yoff = yc - height/2.0;
	Float x0 = 0.1 * width + xoff, x1 = 0.9*width + xoff;
	Float ystep = height/7.0;	// # functions called.
	Float y0 = ystep + yoff, y1 = ystep*2 + yoff;
	Float y2 = ystep*3 + yoff, y3 = ystep*4 + yoff;
	Float y4 = ystep*5, y5 = ystep*6;
	Vector<uInt> colors(6);
	Vector<Float> xv(6), yv(6);

	for(int i=0; i< 6; i++) {
		xv(i) = x0 + (x1-x0)*i/5.0;
		yv(i) = y3;
	}
	colors(0) = nColors_ - WHITE;
	colors(1) = nColors_ - RED;
	colors(2) = nColors_ - GREEN;
	colors(3) = nColors_ - BLUE;
	colors(4) = nColors_ - YELLOW;
	colors(5) = nColors_ - GRAY;

	pc->pushAttrib(GL_CURRENT_BIT);
	// Colored points
	pc->setPointSize(1);
	pc->drawColoredPoints(xv, yv, colors);
	pc->setPointSize(2);
	for(int i=0; i< 6; i++)
		yv(i) = y4;

	pc->drawColoredPoints(xv, yv, colors);

	// Colored lines.
	{
		Vector<Float> X1(6), X2(6), Y1(6), Y2(6);
		for(int i=0; i< 6; i++) {
			X1(i) = x0 + (x1-x0)*i/6.0;
			X2(i) = x0 + (x1-x0)*(i+1)/6.0;
			Y1(i) = y5;
			Y2(i) = y5;
		}
		pc->drawColoredLines(X1, Y1, X2, Y2, colors);
	}

	// Lines of various widths and styles.
	pc->setLineWidth(2.5);
	pc->setLineStyle(Display::LSDoubleDashed);
	pc->drawLine(x0, y2, x1, y2);

	pc->setLineWidth(2.0);
	pc->setLineStyle(Display::LSDashed);
	pc->drawLine(x0, y1, x1, y1);

	// Last so values get reset to defaults.
	pc->setLineStyle(Display::LSSolid);
	pc->setLineWidth(1.0);
	pc->drawLine(x0, y0, x1, y0);
	pc->popAttrib();
}

// Draw a bounding box. Inside it, draw a filled polygon 80%
// the box's size and centered in it. A white line is drawn under the filled
// polygon and another is drawn over it.
// A smaller unfilled polygon is drawn on top.
// Depending on how the filled polygon is drawn, the bottom white line
// may be partially visible.
//  color is the polygon's color index.
static void drawObj( Float x, Float y, Float w, Float h,
                     uInt color, FP poly, GLPixelCanvas *pc) {
	pc->pushAttrib(GL_CURRENT_BIT);
	// Draw a box around area.
	setColor(WHITE, pc);

	pc->drawRectangle( x, y, x+w, y+h);
	// A line underneath.
	pc->drawLine(x, y, x+w, y+h);

	// Draw filled polygon
	setColor(color, pc);

	poly(True, x + w/2.0, y + h/2.0, 0.8*w, 0.8*h, pc);

	// Then an overlaying line.
	setColor(WHITE, pc);
	pc->drawLine(x, y+h, x+w, y);
	// Finally an unfilled polygon inside first.
	setColor(GRAY, pc);
	poly(False, x +w/2.0, y + h*0.5, 0.5*w, 0.5*h, pc);
	pc->popAttrib();
}

// Draw everything.
// An image is drawn in the lower left corner.
// Create a display list comprising:
// 	A rectangle, octagon, lines and image.
// Draw the list in two places.
static struct {
	Display::FillStyle style;
	const char *name;
} fillstyles[] = {
	{ Display::FSSolid, "FSSolid"},
	{ Display::FSTiled, "FSTiled"},
	{ Display::FSStippled, "FSStippled"},
	{ Display::FSOpaqueStippled, "FSOpaqueStippled"}
};

static void doFull(GLPixelCanvas *pc, uInt width, uInt height) {
	// Size of each drawing region.
	Float W = width*0.25, H = height*0.25;
	static int entrycount = 0;
	uInt color;

	pc->pushAttrib(GL_CURRENT_BIT);
	// Reset
	pc->setLineWidth(1.0);
	pc->setLineStyle(Display::LSSolid);
	setColor(WHITE, pc);

	// A color that changes every time.
	switch(entrycount & 0x3) {
	case 0:
		color = WHITE;
		break;
	case 1:
		color = RED;
		break;
	case 2:
		color = GREEN;
		break;
	case 3:
		color = BLUE;
		break;
	default:
		color = YELLOW;
		break;
	}

	// Draw a filled rectangle with a non filled rectangle inside it.
	uInt rectList = pc->newList();
	// GLPixelCanvas users can insert notes into the trace list to
	// help debugging.
	pc->note("\"Draw a filled rectangle with a non filled rectangle inside it.\"");
	drawObj(0.0, 0.0, W, H, color, drawRectangle, pc);
	pc->endList();

	// Same thing but with a polygon.
	uInt octList = pc->newList();
	pc->note("\"Draw a filled polygon with a non filled polygon inside it.\"");

	drawObj(W, 0.0, W, H, RED, drawOctagon, pc);
	pc->endList();

	uInt lineList = pc->newList();
	pc->note("\"Draw different linestyles (with different widths).\"");
	setColor(WHITE, pc);
	pc->setLineWidth(2.0);
	pc->drawRectangle(W*2, 0.0, W*3, H);
	drawLines(W*2+W/2, H/2, W, H, pc);
	pc->endList();

	uInt imageList = pc->newList();
	pc->note("\"Drawing an image\"");
	drawObj(W*3, 0, W, H, WHITE, drawImage, pc);
	pc->endList();

	uInt list2 = pc->newList();

	// Every time the routine is called, we change the way the polys
	// are drawn.
	pc->setFillStyle(fillstyles[entrycount&0x3].style);
	pc->drawList(rectList);
	pc->setFillStyle(fillstyles[(entrycount+1)&0x3].style);
	pc->drawList(octList);

	pc->drawList(lineList);

	pc->drawList(imageList);
	pc->endList();

	pc->setLineWidth(1.0);
	setColor(GRAY, pc);
	pc->translateList(list2, 0.0, H);
	pc->drawList(list2);

	pc->translateList(list2, 0.0, H);
	setColor(WHITE,pc);
	pc->setLineWidth(2.0);
	pc->drawList(list2);
	drawImage(1, W/2, H/2, W, H, pc);

	// Cleanup.
	pc->deleteList(rectList);
	pc->deleteList(octList);
	pc->deleteList(lineList);

	pc->deleteList(imageList);
	pc->deleteList(list2);
	pc->popAttrib();
	entrycount++;
}

// Just draw an image or so.
static void doSmall(GLPixelCanvas *pc, uInt width, uInt height) {
	// Size of each drawing region.
	Float W = width*0.25, H = height*0.25;

	uInt imageList = pc->newList();
	drawImage(1, W/2, H/2, W, H, pc);
	drawObj(W, 0.0, W, H, YELLOW, drawImage, pc);
	pc->endList();
	pc->translateList(imageList, 0.0, H/4);
	pc->drawList(imageList);
	pc->deleteList(imageList);
}

void draw(WorldCanvas *wCanvas) {
	GLPixelCanvas *pc = (GLPixelCanvas *)wCanvas->pixelCanvas();
	uInt width = pc->width();
	uInt height = pc->height();

	// This isn't necessary for RGB mode, and probably not for
	// indexed.
	initColormap(pc->glpcctbl());

	if(doSmall_)
		doSmall(pc, width, height);
	else
		doFull(pc, width, height);

	// Dump any trace info.
	if(pc->tracing())
		pc->postString();
}

void MyRefresh::operator()(const WCRefreshEvent &ev) {
	if (ev.reason() == Display::BackCopiedToFront) {
		return;
	}
	try {
		WorldCanvas *wCanvas = ev.worldCanvas();
		draw(wCanvas);
	} catch (AipsError x) {
		cerr << PNAME << ": " << x.getMesg() << endl;
	}
}

#else

int main() {
	return 0;
}

#endif	// OGL
