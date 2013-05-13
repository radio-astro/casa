//# dPSLatticeAsRaster.cc: demo use of LatticeAsRaster class on a PSPixelCanvas
//# Copyright (C) 1999,2000,2001,2003
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
//# $Id$

#include <casa/stdio.h>
#include <casa/stdlib.h>
#include <casa/fstream.h>

//# aips includes:
#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>

//# trial includes:
#include <images/Images/PagedImage.h>
#include <casa/Arrays/IPosition.h>

//# display library includes:
#include <display/Display/PSWorldCanvasApp.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/LatticeAsRaster.h>
#include <display/DisplayDatas/LatticeAsContour.h>
#include <display/Display/Attribute.h>
#include <display/Display/StandAloneDisplayApp.h>

#include <casa/namespace.h>
// Called from help to print various paper types available.
static void printMedia(ostream &out) {
	int npages = PSDriver::numPageTypes();
	char buf[256];

	sprintf(buf,
	        "Primary Alias\t\tWidth   Height Margins\t\tAlias2\t\t\tName");
	out << buf << endl;
	for(int i=0; i < npages; i++) {
		const PSDriver::PageInfo *p = PSDriver::getPageInfo(i);

		sprintf(buf,"%-20s\t%6.1f, %6.1f %4.1f %-6s   %-18s %s",
		        p->alias1, p->width, p->height, p->lrmargin,
		        PSDriver::dimensionToString(p->dimension),
		        (p->alias2 != NULL) ? p->alias2 : "",
		        p->name);
		out << buf << endl;
	}
}

static void help(ostream &out, char *name, const int showmedia) {
	out << "Usage: " << name
	    << " [-a][-c][-e][-f file][-h][-H][-i][-l][-m paper][-p][-r res] <datasetname>"
	    << endl;
	out << "-a	Don't draw axes." << endl;
	out << "-b xll yll xur yur	User supplied bounding box." << endl;
	out << "-B	Draw a box around writable page boundaries." << endl;
	out << "-c	Don't show contours." << endl;
	out << "-e	Use EPS." << endl;
	out << "-f file	Write output to file." << endl;
	out << "-h	Help message." << endl;
	out << "-H	Help + available paper sizes.(long & wide)" << endl;
	out << "-i	Don't draw image." << endl;
	out << "-l	Use landscape." << endl;
	out << "-m pagetype	Use pagetype as page size.";
	out << " (Default is LETTER)" << endl;

	out << "-p	Portrait (default)." << endl;
	out << "-r res	Set declared resolution to res dots per inch."
	    << endl;
	out << "datasetname	AIPS++ data set to use." << endl;
	out <<
	    "There will be no output if neither the image nor contours are drawn."
	    << endl;
	if(showmedia) {
		out << endl;
		printMedia(out);
	}
}

main(int argc, char **argv) {
	PSDriver::MediaSize media = PSDriver::LETTER;
	PSDriver::Layout layout = PSDriver::PORTRAIT;
	float res = 100;	// Default is 100dpi.
	char *name = argv[0];
	char *file = "test.ps";
	Bool drawBox = False;
	Bool doImage = True;
	Bool doContours = True;
	Bool doAxes = True;
	Bool useBBox = False;
	float bbox[] = {0.0, 0.0, 0.0, 0.0};	// x0, y0, x1, y1
	argv += 1;
	argc -= 1;

	// Parse any switches.
	while( (argc > 0) && (**argv == '-')) {
		char *swtch = *argv++, *sptr = swtch+1;
		argc -= 1;
		char arg;
		while( (arg = *sptr++) != '\0')
			switch(arg) {
			case 'a':
				doAxes = False;
				break;
			case 'b':
				for(int ib=0; ib<4; ib++) {
					bbox[ib] = atof(*argv);
					argv++;
					argc--;
					if(bbox[ib] < 0.0) {
						cerr << "Bad box value: " << *argv << endl;
						exit(1);
					}
				}
				useBBox = True;
				break;
			case 'B':
				drawBox = True;
				break;
			case 'c':
				doContours = False;
				break;
			case 'e':
				if(layout == PSDriver::PORTRAIT)
					layout = PSDriver::EPS_PORTRAIT;
				else if(layout == PSDriver::LANDSCAPE)
					layout = PSDriver::EPS_LANDSCAPE;
				// Else already EPS.
				break;
			case 'f':
				file = *argv++;
				argc--;
				break;
			case 'h':
				help(cerr, name, 0);
				exit(0);
				break;
			case 'H':
				help(cerr, name, 1);
				exit(0);
				break;
			case 'i':
				doImage = False;
				break;
			case 'l': {
				if(layout & PSDriver::EPS)
					layout = PSDriver::EPS_LANDSCAPE;
				else
					layout = PSDriver::LANDSCAPE;
			}
			break;
			case 'p': {
				if(layout & PSDriver::EPS)
					layout = PSDriver::EPS_PORTRAIT;
				else
					layout = PSDriver::PORTRAIT;
			}
			break;
			case 'm': {
				const PSDriver::PageInfo *p =
				    PSDriver::lookupPageInfo(*argv);
				if(p == NULL) {
					cerr << "Unknown media type \"" << *argv
					     << "\"." << endl;
					exit(1);
				}
				media = p->media;
				argc--;
				argv++;
			}
			break;
			case 'r':
				res = atof(*argv);
				if(res <= 0) {
					cerr << "Bad resolution: " << *argv << endl;
					exit(1);
				}
				argv++;
				argc--;
				break;
			default:
				cerr	<< "Unknown switch: " << swtch
				        << " (" << arg << ")" <<endl;
				exit(1);
			}
	}

	try {

		// make a simple XWindow with embedded WorldCanvas
		cerr << 0 << endl;
		if (argc < 1) {
			help(cerr, name, 0);
			throw(AipsError("No image given on the command line."));
		}

		// PSInfo is optional. It is used here to embed a comment into the
		// PostScript file.
		PSDriver::PSInfo *psinfo = new PSDriver::PSInfo();
		{
			String comment(name);
			const PSDriver::PageInfo *pi = PSDriver::pageInfo(media);
			comment += " Data file: ";
			comment += argv[0];
			comment += "\nMedia: ";
			comment += pi->alias1;
			const char *cptr = NULL;
			switch(layout) {
			case PSDriver::PORTRAIT:
				cptr = "PORTRAIT";
				break;
			case PSDriver::LANDSCAPE:
				cptr = "LANDSCAPE";
				break;
			case PSDriver::EPS_PORTRAIT:
				cptr = "EPS_PORTRAIT";
				break;
			case PSDriver::EPS_LANDSCAPE:
				cptr = "EPS_LANDSCAPE";
				break;
			default:
				break;
			}
			comment += " Layout: ";
			comment += cptr;
			char buf[128];
			sprintf(buf, "\nDeclared resolution is: %.2f pixels per inch.", res);
			comment += buf;
			psinfo->Comment(comment);
		}

		// If file is NULL, PSDriver will create one.
		PSDriver *ps;
		if(useBBox)
			ps = new PSDriver(file, PSDriver::INCHES,
			                  bbox[0], bbox[1], bbox[2], bbox[3], layout, psinfo);
		else
			ps = new PSDriver(file, media, layout, psinfo);

		PSWorldCanvasApp *app = new PSWorldCanvasApp(ps);
		PSPixelCanvas *pc = app->pixelCanvas();
		WorldCanvas *wCanvas = app->worldCanvas();

#if 0
		((PSPixelCanvasColorTable *)(wCanvas->pixelCanvas()->pcctbl()))
		->setColorModel(Display::Index);
		((PSPixelCanvasColorTable *)(wCanvas->pixelCanvas()->pcctbl()))
		->resize(10);
#endif
		// Currently, the default declared resolution (1200dpi) causes problems.
		pc->setResolution(res, res, PSDriver::INCHES);
		cerr << 5 << endl;

		// add a WorldCanvasHolder
		WorldCanvasHolder *wcHolder = new WorldCanvasHolder(wCanvas);
		cerr << 15 << endl;

		// load the named image
		String fileName(argv[0]);

		cout << "Trying to load AIPS++ Image \"" << fileName << "\"" << endl;
		PagedImage<Float> pImage(fileName);

		// how many dimensions?
		uInt nDim = pImage.ndim();

		if (nDim < 2) {
			throw(AipsError("image has less than two dimensions"));
		}

		// a display data to make
		LatticeAsRaster<Float> *lar;
		LatticeAsContour<Float> *lac;

		if (nDim == 2) {
			lar = new LatticeAsRaster<Float>(&pImage, 0, 1);
			lac = new LatticeAsContour<Float>(&pImage, 0, 1);
		} else {
			IPosition fixedPos(nDim);
			fixedPos = 0;
			lar = new LatticeAsRaster<Float>(&pImage, 0, 1, 2, fixedPos);
			lac = new LatticeAsContour<Float>(&pImage, 0, 1, 2, fixedPos);
			Attribute zinAtt("zIndex", Int(pImage.shape()(2)/2));
			wcHolder->setRestriction(zinAtt);
		}
		if (!lar || !lac) {
			throw(AipsError("couldn't build the display data"));
		}
		cerr << 30 << endl;

		// request axis labels be drawn, and bilinear resampling
		Record rec, recOut;
		if(doAxes) {
			rec.define("axislabelswitch", True);
		}
		rec.define("color", "blue");
		rec.define("line", Int(1));
		rec.define("resample", "bilinear");
		lar->setOptions(rec, recOut);
		lac->setOptions(rec, recOut);

		if(doImage)
			wcHolder->addDisplayData((DisplayData *)lar);
		if(doContours)
			wcHolder->addDisplayData((DisplayData *)lac);
		cerr << 35 << endl;

		app->run();

		cerr << 40 << endl;
		if(drawBox) {
			float xxxwidth, xxxheight;
			ps->pageSize(xxxwidth, xxxheight);
			ps->drawRectangle(0.0, 0.0, xxxwidth, xxxheight);
		}
		cerr << 50 << endl;

		delete wcHolder;
		cerr << 55 << endl;
		delete app;
		cerr << 60 << endl;

		delete lac;
		delete lar;

	} catch (const AipsError &x) {
		cerr << "Exception caught" << endl;
		cerr << "Message: " << x.getMesg() << endl;
	}

}
