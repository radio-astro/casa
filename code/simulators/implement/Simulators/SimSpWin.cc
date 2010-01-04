//# SimSpWin.h: container for containing Spectral Window description
//# Copyright (C) 2002,2003
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
#include <simulators/Simulators/SimSpWin.h>

#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <casa/sstream.h>

namespace casa { //# NAMESPACE CASA - BEGIN

SimSpWin::SimSpWin() : 
    row_p(-1), nch_p(0), usb_p(True), unif_p(True), name_p(), 
    sfreq_p(MVFrequency(0.0)), ref_p(MVFrequency(0.0)), width_p(0.0), 
    step_p(0.0) 
{ }

SimSpWin::SimSpWin(uInt nchan, const MFrequency& start, 
		   const MVFrequency& width, const MVFrequency& step, 
		   const MFrequency refFreq, const String& name, Bool usb) 
    : row_p(-1), nch_p(nchan), usb_p(usb), unif_p(True), name_p(name), 
      sfreq_p(start), ref_p(refFreq), width_p(width), step_p(step) 
{ }

SimSpWin::SimSpWin(uInt nchan, const MFrequency& start, 
		   const MVFrequency& width, const MVFrequency& step, 
		   const String& name, Bool usb) 
    : row_p(-1), nch_p(nchan), usb_p(usb), unif_p(True), name_p(name), 
      sfreq_p(start), ref_p(start), width_p(width), step_p(step) 
{ }

SimSpWin::SimSpWin(const MFrequency& start, const MVFrequency& width,
		   const String& name, Bool usb) 
    : row_p(-1), nch_p(1), usb_p(usb), unif_p(True), name_p(), sfreq_p(start), 
      ref_p(start), width_p(width), step_p(width) 
{ }

SimSpWin::SimSpWin(const SimSpWin& w) 
    : row_p(w.row_p), nch_p(w.nch_p), usb_p(w.usb_p), unif_p(w.unif_p), 
      name_p(w.name_p), sfreq_p(w.sfreq_p), ref_p(w.ref_p), width_p(w.width_p),
      step_p(w.step_p) 
{ }

SimSpWin& SimSpWin::operator=(const SimSpWin& w) {
    row_p = w.row_p; 
    nch_p = w.nch_p; 
    usb_p = w.usb_p; 
    unif_p = w.unif_p; 
    name_p = w.name_p; 
    sfreq_p = w.sfreq_p; 
    ref_p = w.ref_p; 
    width_p = w.width_p;
    step_p = w.step_p;
    return *this;
}


//#########################################


SimSpWinList::SimSpWinList(const MSSpectralWindow& spwt, uInt stepsz)
    : n_p(0), chnk_p(stepsz), rec_p(spwt.nrow(), 0) 
{
    initFrom(spwt);
}

void SimSpWinList::initFrom(const MSSpectralWindow& spwt) {
    ROMSSpWindowColumns spwc(spwt);
    uInt nwin = spwt.nrow();
    Vector<Double> freq, diff;
    Vector<Quantity> qfreq;
    Int nchan = 0;
    Bool unif = True;
    Double step = 0.0, width = 0.0;
    MFrequency start;
    Vector<MFrequency> mfreq;

    for(uInt i=0; i < nwin; i++) {
	nchan = spwc.numChan()(i);
	freq.resize(nchan);
	mfreq.resize(nchan);

	// get start freq
	spwc.chanFreqMeas().get(i, mfreq);
	start = mfreq(0);

	// compute average width
	spwc.chanWidthQuant().get(i, qfreq, Unit("Hz"));
	for(Int j=0; j < nchan; j++) freq(j) = qfreq(j).getValue();
	if (nchan > 1) {
	    width = mean(freq);
	} else {
	    width = freq(0);
	}

	spwc.chanFreqQuant().get(i, qfreq, Unit("Hz"));
	for(Int j=0; j < nchan; j++) freq(j) = qfreq(j).getValue();

	// compute the average step size
	if (nchan > 1) {
	    diff.resize(nchan-1);
	    for(Int j=1; j < nchan; j++) {
		diff(j-1) = freq(j)-freq(j-1);
	    }
	    step = mean(diff);

	    // detect non uniform spacing of channels
	    diff /= step;
 	    unif = (allNear(diff, 1.0, 0.001*width));
	}
	else {
	    step = width;
	}

	SimSpWin &win = addWindow(nchan, mfreq(0), width, step,
				  spwc.refFrequencyMeas()(i),
				  spwc.name()(i),
				  (spwc.netSideband()(i) > 0));
	win.setId(i);
	win.setUniform(unif);
    }
}

SimSpWinList::SimSpWinList(const SimSpWinList& w) : n_p(w.n_p), 
    chnk_p(w.chnk_p), rec_p(w.rec_p.nelements())
{ 
    for(uInt i=0; i < n_p; i++) 
	rec_p[i] = new SimSpWin(*(w.rec_p[i]));
}

SimSpWinList& SimSpWinList::operator=(const SimSpWinList& w) {
    deleteRecs();
    n_p = w.n_p;
    chnk_p = w.chnk_p;
    rec_p.resize(w.rec_p.nelements());
    for(uInt i=0; i < n_p; i++) 
	rec_p[i] = new SimSpWin(*(w.rec_p[i]));
    return *this;
}

void SimSpWinList::flush(MSSpectralWindow& spwt) {
    MSSpWindowColumns spwc(spwt);
    uInt nrow = spwt.nrow();
    Vector<Double> freq, res;
    Double start, step;
    String Hz("Hz");
    SimSpWin *win;
    Bool hasDopID = spwt.isColumn(MSSpectralWindow::DOPPLER_ID);

    for(uInt i=0; i < n_p; i++) {
	if (rec_p[i]->getId() < 0) {
	    spwt.addRow(1);
	    win = rec_p[i];

	    spwc.numChan().put(nrow, win->numChannels());
	    spwc.name().put(nrow, win->getName());
	    spwc.netSideband().put(nrow, win->netSideband());
	    spwc.ifConvChain().put(nrow, 0);
	    spwc.freqGroup().put(nrow, 0);
	    spwc.freqGroupName().put(nrow, "Group 1");
	    spwc.flagRow().put(nrow, False);
	    spwc.measFreqRef().put(nrow, win->refFreq().type());
	    spwc.refFrequencyMeas().put(nrow, win->refFreq());

	    freq.resize(win->numChannels());
	    start = win->startFreq().get(Hz).getValue();
	    step = win->chanStep().get(Hz).getValue();
	    for(uInt j=0; j < win->numChannels(); j++) 
		freq(j) = start + j*step;
	    spwc.chanFreq().put(nrow,freq);

	    res.resize(win->numChannels());
	    res = win->chanWidth().get(Hz).getValue();
	    spwc.chanWidth().put(nrow,res);
	    spwc.effectiveBW().put(nrow,res);
	    spwc.resolution().put(nrow,res);

	    spwc.totalBandwidth().put(nrow, win->numChannels()*res(0));

	    if (hasDopID) spwc.dopplerId().put(nrow, -1);

	    win->setId(nrow++);
	}
    }
}

void SimSpWinList::numChannels(Vector<Int> nchan) const {
    nchan.resize(numWindows());
    for(uInt i=0; i < nchan.nelements(); i++) 
	nchan(i) = rec_p[i]->numChannels();
}

void SimSpWinList::throwOutOfRange(uInt legallength, Int index) const {
    ostringstream msg;
    msg << "index out of range of [0," << legallength 
	<< "]: " << index;
    throw AipsError(String(msg));
}

} //# NAMESPACE CASA - END

