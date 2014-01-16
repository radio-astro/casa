//# dNBody.cc: demo the NBody DisplayData
//# Copyright (C) 2000
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

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Inputs/Input.h>
#include <display/Display/SimpleWorldCanvasApp.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayDatas/NBody.h>

//# THIS IS A STAND-ALONE APPLICATION
#include <display/Display/StandAloneDisplayApp.h>

#include <casa/namespace.h>
main(int argc, char **argv) {

	try {

		SimpleWorldCanvasApp *x11app = 0;
		WorldCanvas *wCanvas = 0;

		x11app = new SimpleWorldCanvasApp;
		wCanvas = x11app->worldCanvas();

		if (!wCanvas) {
			throw(AipsError("Couldn't construct WorldCanvas"));
		}

		// manage it with a WorldCanvasHolder
		WorldCanvasHolder *wcHolder = 0;
		wcHolder = new WorldCanvasHolder(wCanvas);
		if (!wcHolder) {
			throw(AipsError("Couldn't construct WorldCanvasHolder"));
		}

		// create the nbody DisplayData
		NBody *nbody = new NBody;

		// add it to the display
		wcHolder->addDisplayData(nbody);

		// run the application
		x11app->run();

		if (wcHolder) {
			delete wcHolder;
		}
		if (x11app) {
			delete x11app;
		}
		if (nbody) {
			delete nbody;
		}

	} catch (const AipsError &x) {
		cerr << "Exception caught:" << endl;
		cerr << x.getMesg() << endl;
	}

}

