//# PCTestPattern.cc: PixelCanvas test pattern display refresh event handler
//# Copyright (C) 2000,2001
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
#include <casa/aips.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayEvents/PCRefreshEvent.h>
#include <display/DisplayEvents/PCTestPattern.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Default Constructor Required
	PCTestPattern::PCTestPattern() :
		itsFirstTime(True),
		itsListLength(0),
		itsImList(0) {
	}

// Destructor
	PCTestPattern::~PCTestPattern() {
	}

// Refresh event handling operator.
	void PCTestPattern::operator ()(const PCRefreshEvent &ev) {
		PixelCanvas *pc = ev.pixelCanvas();
		if (ev.reason() == Display::BackCopiedToFront ||
		        ev.reason() == Display::UserCommand) {
			// This operator draws to the back buffer, so should ignore
			// back-copied-to-front events.
			return;
		}
		pc->setDrawBuffer(Display::BackBuffer);

		if ((pc->pcctbl()->colorModel() == Display::RGB) ||
		        (pc->pcctbl()->colorModel() == Display::HSV)) {
			// RGB/HSV mode: draw 3 slices of the color cube accessed in
			// each mode...
			uInt blockwidth = 0, blockheight = 0;
			uInt skipw = 0, skiph = 0;
			if (itsFirstTime || (pc->width() != itsWidth)  ||
			        (pc->height() != itsHeight) ||
			        (ev.reason() == Display::ColorTableChange) ||
			        (ev.reason() == Display::ColormapChange)) {
				itsFirstTime = False;
				itsWidth = pc->width();
				itsHeight = pc->height();
				uInt i;
				for (i = 0; i < itsListLength; i++) {
					pc->deleteList(itsImList[i]);
				}
				if (itsListLength) {
					delete [] itsImList;
				}

				itsListLength = 6;
				itsImList = new uInt[itsListLength];

				blockwidth = uInt(itsWidth / (4./3. * 3.));
				blockheight = uInt(itsHeight / (4./3. * 2.));
				skipw = uInt((itsWidth - 3 * blockwidth) / 4.);
				skiph = uInt((itsHeight - 2 * blockheight) / 3.);

				Matrix<Float> xramp(blockwidth, blockheight);
				Matrix<Float> yramp(blockwidth, blockheight);
				Matrix<Float> ones(blockwidth, blockheight);
				Matrix<Float> zeros(blockwidth, blockheight);

				for (i = 0; i < blockwidth; i++) {
					for (uInt j = 0; j < blockheight; j++) {
						xramp(i, j) = Float(i) / Float(blockwidth - 1);
						yramp(i, j) = Float(j) / Float(blockheight - 1);
						ones(i, j) = Float(1.0);
						zeros(i, j) = Float(0.0);
					}
				}

				Matrix<uLong> theImage;
				theImage.resize(xramp.shape());
				theImage = 0;

				// build rgb images
				pc->setColorModel(Display::RGB);

				itsImList[0] = pc->newList();
				pc->pushMatrix();
				pc->mapToColor3(theImage, xramp, yramp, zeros);
				pc->drawImage(theImage, 0, 0);
				pc->translate(blockwidth / 2, blockheight / 2);
				pc->drawText(0, 0, "RG image");
				pc->popMatrix();
				pc->endList();

				pc->mapToColor3(theImage, xramp, zeros, yramp);
				itsImList[1] = pc->newList();
				pc->pushMatrix();
				pc->drawImage(theImage, 0, 0);
				pc->translate(blockwidth / 2, blockheight / 2);
				pc->drawText(0, 0, "RB image");
				pc->popMatrix();
				pc->endList();

				pc->mapToColor3(theImage, zeros, xramp, yramp);
				itsImList[2] = pc->newList();
				pc->pushMatrix();
				pc->drawImage(theImage, 0, 0);
				pc->translate(blockwidth / 2, blockheight / 2);
				pc->drawText(0, 0, "GB image");
				pc->popMatrix();
				pc->endList();

				// build hsv images
				pc->setColorModel(Display::HSV);

				pc->mapToColor3(theImage, xramp, yramp, ones);
				itsImList[3] = pc->newList();
				pc->pushMatrix();
				pc->drawImage(theImage, 0, 0);
				pc->translate(blockwidth / 2, blockheight / 2);
				pc->drawText(0, 0, "HS image");
				pc->popMatrix();
				pc->endList();

				pc->mapToColor3(theImage, xramp, ones, yramp);
				itsImList[5] = pc->newList();
				pc->pushMatrix();
				pc->drawImage(theImage, 0, 0);
				pc->translate(blockwidth / 2, blockheight / 2);
				pc->drawText(0, 0, "HV image");
				pc->popMatrix();
				pc->endList();

				pc->mapToColor3(theImage, zeros, xramp, yramp);
				itsImList[4] = pc->newList();
				pc->pushMatrix();
				pc->drawImage(theImage, 0, 0);
				pc->translate(blockwidth / 2, blockheight / 2);
				pc->drawText(0, 0, "SV image");
				pc->popMatrix();
				pc->endList();

			}

			pc->setColor("white");
			pc->setClearColor("black");
			pc->clear();
			pc->setFont("rock24");

			// apply bottom left offset
			pc->pushMatrix();
			pc->translate(skipw, skiph);
			pc->pushMatrix();

			pc->translate(0, blockheight + skiph);
			pc->drawList(itsImList[0]);
			pc->translate(blockwidth + skipw, 0);
			pc->drawList(itsImList[1]);
			pc->translate(blockwidth + skipw, 0);
			pc->drawList(itsImList[2]);

			pc->popMatrix();

			pc->drawList(itsImList[3]);
			pc->translate(blockwidth + skipw, 0);
			pc->drawList(itsImList[4]);
			pc->translate(blockwidth + skipw, 0);
			pc->drawList(itsImList[5]);

			pc->popMatrix();

		} else {
			// Index mode: draw all colormaps as horizontal strips...
			uInt blockwidth = 0, blockheight = 0;
			uInt skipw = 0, skiph = 0;
			ColormapManager cmm = pc->pcctbl()->colormapManager();
			if (itsFirstTime || (pc->width() != itsWidth)  ||
			        (pc->height() != itsHeight) ||
			        (ev.reason() == Display::ColorTableChange) ||
			        (ev.reason() == Display::ColormapChange)) {
				itsFirstTime = False;
				itsWidth = pc->width();
				itsHeight = pc->height();
				uInt i;
				for (i = 0; i < itsListLength; i++) {
					pc->deleteList(itsImList[i]);
				}
				if (itsListLength) {
					delete [] itsImList;
				}

				itsListLength = cmm.nMaps();
				if (itsListLength == 0) {
					return;
				}
				itsImList = new uInt[itsListLength];

				uInt maxsize = 0;
				for (i = 0; i < itsListLength; i++) {
					Colormap *cmap = (Colormap *)cmm.getMap(i);
					uInt thissize = pc->pcctbl()->getColormapSize(cmap);
					maxsize = thissize > maxsize ? thissize : maxsize;
				}

				blockheight = uInt(itsHeight * 3./4. / Float(itsListLength));
				skiph = uInt(Float(itsHeight - itsListLength * blockheight) /
				             Float(itsListLength + 1));
				skipw = skiph;
				blockwidth = itsWidth - 2 * skipw;

				for (uInt mi = 0; mi < itsListLength; mi++) {
					itsImList[mi] = pc->newList();
					Colormap *cmap = (Colormap *)cmm.getMap(mi);
					Int ncols = pc->pcctbl()->getColormapSize(cmap);
					pc->pushMatrix();
					uInt thiswidth = blockwidth;
					Matrix<uLong> theImage(thiswidth, blockheight);
					for (uInt k = 0; k < thiswidth; k++) {
						Int col = Int(Float(k) / Float(thiswidth - 1) * Float(ncols));
						col = col < 0 ? 0 : col;
						col = col > (ncols - 1) ? (ncols - 1) : col;
						for (uInt z = 0; z < blockheight; z++) {
							theImage(k, z) = col;
						}
					}
					Matrix<uLong> mappedImage(theImage.shape());
					pc->setColormap(cmap);
					pc->mapToColor(mappedImage, theImage);
					pc->drawImage(mappedImage, 0, 0);
					pc->translate(thiswidth / 2, blockheight / 2);
					char chlen[20];
					sprintf(chlen, " (%d colors)", ncols);
					pc->drawText(0, 0, cmap->name() + String(chlen));
					pc->popMatrix();
					pc->endList();
				}
			}

			pc->setColor("white");
			pc->setClearColor("black");
			pc->clear();
			pc->setFont("rock24");

			// apply bottom left offset
			pc->pushMatrix();
			pc->translate(skipw, skiph);

			for (uInt i = 0; i < itsListLength; i++) {
				pc->drawList(itsImList[i]);
				pc->translate(0, blockheight + skiph);
			}
			pc->popMatrix();
			pc->copyBackBufferToFrontBuffer();
		}
	}

// (Required) copy constructor.
	PCTestPattern::PCTestPattern(const PCTestPattern &other) :
		// below commented until PCRefreshEH has (required) copy ctor.
		//PCRefreshEH(other),
		itsFirstTime(True),
		itsListLength(0) {
	}

// (Required) copy assignment.
	PCTestPattern &PCTestPattern::operator=(const PCTestPattern &other) {
		if (this != &other) {
			// below commented until PCRefreshEH has (required) copy assignment.
			//PCRefreshEH::operator=(other);
			itsFirstTime = True;
			itsListLength = 0;
		}
		return *this;
	}



} //# NAMESPACE CASA - END

