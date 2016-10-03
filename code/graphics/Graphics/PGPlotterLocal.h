//# PGPlotterLocal.h: Plot to a PGPLOT device "local" to this process.
//# Copyright (C) 1997,2001
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
//#
//# $Id$

#ifndef GRAPHICS_PGPLOTTERLOCAL_H
#define GRAPHICS_PGPLOTTERLOCAL_H

#include <casa/aips.h>
#include <casa/System/PGPlotter.h>

namespace casacore{

class String;
template<class T> class Vector;
}

namespace casa { //# NAMESPACE CASA - BEGIN


// <summary>
// Plot to a PGPLOT device "local" to this process.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class="casacore::PGPlotterInterface">casacore::PGPlotterInterface</linkto>
// </prerequisite>
//
// <etymology>
// "Local" is used to denote that the actuall plotting is done by PGPLOT calls
// linked into this executable, as opposed to the common case where the PGPLOT
// calls are sent over the Glish bus to a "remote" display.
// </etymology>
//
// <synopsis>
// Generally programmers should not use this class, instead they should use
// <linkto class="casacore::PGPlotter">casacore::PGPlotter</linkto> instead.
//
// This class make a concrete
// <linkto class="casacore::PGPlotterInterface">casacore::PGPlotterInterface</linkto> object which
// calls PGPLOT directly, i.e. PGPLOT is linked into the current executable.
// </synopsis>
//
// <example>
// <srcblock>
//     // plot y = x*x
//     casacore::Vector<casacore::Float> x(100), y(100);
//     indgen(x);
//     y = x*x;

//     PGPlotterLocal plotter("myplot.ps/ps");
//     plotter.env(0, 100, 0, 100*100, 0, 0);
//     plotter.line(x, y);
// </srcblock>
// </example>
//
// <motivation>
// It might be necessary to call PGPLOT directly in some circumstances. For
// example, it might be too inefficient to pass a lot of Image data over the
// glish bus.
// </motivation>
//
// <todo asof="1997/12/31">
//   <li> Add more plot calls.
// </todo>

class PGPlotterLocal : public casacore::PGPlotterInterface
{
public:
    // Open "device", which must be a valid PGPLOT style device, for example
    // <src>/cps</src> for colour postscript (or <src>myfile.ps/cps</src>
    // if you want to name the file), or <src>/xs</src> or <src>/xw</src> for
    // and X-windows display.
    // <thrown>
    //   <li> An <linkto class="casacore::AipsError">casacore::AipsError</linkto> will be thrown
    //        if the underlying PGPLOT open fails for some reason.
    // </thrown>
    PGPlotterLocal(const casacore::String &device);
    // The destructor closes the pgplot device.
    virtual ~PGPlotterLocal();

    // The create function to create a casacore::PGPlotter object using a PGPlotterLocal.
    // It only uses the device argument.
    static casacore::PGPlotter createPlotter (const casacore::String &device,
				    casacore::uInt, casacore::uInt, casacore::uInt, casacore::uInt);

    // This is an emulated standard PGPLOT command. It returns a record
    // containing the fields:
    // <srcblock>
    // [ok=casacore::Bool, x=casacore::Float, y=casacore::Float, ch=casacore::String];
    // If the remote device cannot do cursor feedback, ok==F.
    // </srcblock>
    virtual casacore::Record curs(casacore::Float x, casacore::Float y);


    // Standard PGPLOT commands. Documentation for the individual commands
    // can be found in the Glish manual and in the standard PGPLOT documentation
    // which may be found at <src>http://astro.caltech.edu/~tjp/pgplot/</src>.
    // The Glish/PGPLOT documentation is preferred since this interface follows
    // it exactly (e.g. the array sizes are inferred both here and in Glish,
    // whereas they must be passed into standard PGPLOT).
    // <group>
    virtual void arro(casacore::Float x1, casacore::Float y1, casacore::Float x2, casacore::Float y2);
    virtual void ask(casacore::Bool flag);
    virtual void bbuf();
    virtual void bin(const casacore::Vector<casacore::Float> &x, const casacore::Vector<casacore::Float> &data,
		     casacore::Bool center);
    virtual void box(const casacore::String &xopt, casacore::Float xtick, casacore::Int nxsub, 
	     const casacore::String &yopt, casacore::Float ytick, casacore::Int nysub);
    virtual void circ(casacore::Float xcent, casacore::Float ycent, casacore::Float radius);
    virtual void conb(const casacore::Matrix<casacore::Float> &a, const casacore::Vector<casacore::Float> &c,
		      const casacore::Vector<casacore::Float> &tr, casacore::Float blank);
    virtual void conl(const casacore::Matrix<casacore::Float> &a, casacore::Float c,
		      const casacore::Vector<casacore::Float> &tr, const casacore::String &label,
		      casacore::Int intval, casacore::Int minint);
    virtual void cons(const casacore::Matrix<casacore::Float> &a, const casacore::Vector<casacore::Float> &c,
		      const casacore::Vector<casacore::Float> &tr);
    virtual void cont(const casacore::Matrix<casacore::Float> &a, const casacore::Vector<casacore::Float> &c,
		      casacore::Bool nc, const casacore::Vector<casacore::Float> &tr);
    virtual void ctab(const casacore::Vector<casacore::Float> &l, const casacore::Vector<casacore::Float> &r,
		      const casacore::Vector<casacore::Float> &g, const casacore::Vector<casacore::Float> &b,
		      casacore::Float contra, casacore::Float bright);
    virtual void draw(casacore::Float x, casacore::Float y);
    virtual void ebuf();
    virtual void env(casacore::Float xmin, casacore::Float xmax, casacore::Float ymin, casacore::Float ymax, casacore::Int just,
	     casacore::Int axis);
    virtual void eras();
    virtual void errb(casacore::Int dir, const casacore::Vector<casacore::Float> &x, const casacore::Vector<casacore::Float> &y,
	      const casacore::Vector<casacore::Float> &e, casacore::Float t);
    virtual void errx(const casacore::Vector<casacore::Float> &x1, const casacore::Vector<casacore::Float> &x2,
		      const casacore::Vector<casacore::Float> &y, casacore::Float t);
    virtual void erry(const casacore::Vector<casacore::Float> &x, const casacore::Vector<casacore::Float> &y1,
	      const casacore::Vector<casacore::Float> &y2, casacore::Float t);
    virtual void gray(const casacore::Matrix<casacore::Float> &a, casacore::Float fg, casacore::Float bg,
		      const casacore::Vector<casacore::Float> &tr); 
    virtual void hi2d(const casacore::Matrix<casacore::Float> &data, const casacore::Vector<casacore::Float> &x,
		      casacore::Int ioff, casacore::Float bias, casacore::Bool center, 
		      const casacore::Vector<casacore::Float> &ylims);
    virtual void hist(const casacore::Vector<casacore::Float> &data, casacore::Float datmin, casacore::Float datmax, 
		    casacore::Int nbin, casacore::Int pcflag);
    virtual void iden();
    virtual void imag(const casacore::Matrix<casacore::Float> &a, casacore::Float a1, casacore::Float a2,
		      const casacore::Vector<casacore::Float> &tr);
    virtual void lab(const casacore::String &xlbl, const casacore::String &ylbl, 
		   const casacore::String &toplbl);
    virtual void ldev();
    virtual casacore::Vector<casacore::Float> len(casacore::Int units, const casacore::String &string);
    virtual void line(const casacore::Vector<casacore::Float> &xpts, const casacore::Vector<casacore::Float> &ypts);
    virtual void move(casacore::Float x, casacore::Float y);
    virtual void mtxt(const casacore::String &side, casacore::Float disp, casacore::Float coord, casacore::Float fjust,
		    const casacore::String &text);
    virtual casacore::String numb(casacore::Int mm, casacore::Int pp, casacore::Int form);
    virtual void page();
    virtual void panl(casacore::Int ix, casacore::Int iy);
    virtual void pap(casacore::Float width, casacore::Float aspect);
    virtual void pixl(const casacore::Matrix<casacore::Int> &ia, casacore::Float x1, casacore::Float x2,
		      casacore::Float y1, casacore::Float y2);
    virtual void pnts(const casacore::Vector<casacore::Float> &x, const casacore::Vector<casacore::Float> &y,
		      const casacore::Vector<casacore::Int> symbol);
    virtual void poly(const casacore::Vector<casacore::Float> &xpts, const casacore::Vector<casacore::Float> &ypts);
    virtual void pt(const casacore::Vector<casacore::Float> &xpts, const casacore::Vector<casacore::Float> &ypts, 
		  casacore::Int symbol);
    virtual void ptxt(casacore::Float x, casacore::Float y, casacore::Float angle, casacore::Float fjust, 
		    const casacore::String &text);
    virtual casacore::Vector<casacore::Float>  qah();
    virtual casacore::Int qcf();
    virtual casacore::Float qch();
    virtual casacore::Int qci();
    virtual casacore::Vector<casacore::Int> qcir();
    virtual casacore::Vector<casacore::Int> qcol();
    virtual casacore::Vector<casacore::Float> qcr(casacore::Int ci);
    virtual casacore::Vector<casacore::Float> qcs(casacore::Int units);
    virtual casacore::Int qfs();
    virtual casacore::Vector<casacore::Float> qhs();
    virtual casacore::Int qid();
    virtual casacore::String qinf(const casacore::String &item);
    virtual casacore::Int qitf();
    virtual casacore::Int qls();
    virtual casacore::Int qlw();
    virtual casacore::Vector<casacore::Float> qpos();
    virtual casacore::Int qtbg();
    virtual casacore::Vector<casacore::Float> qtxt(casacore::Float x, casacore::Float y, casacore::Float angle, casacore::Float fjust, 
		    const casacore::String &text);
    virtual casacore::Vector<casacore::Float> qvp(casacore::Int units);
    virtual casacore::Vector<casacore::Float> qvsz(casacore::Int units);
    virtual casacore::Vector<casacore::Float> qwin();
    virtual void rect(casacore::Float x1, casacore::Float x2, casacore::Float y1, casacore::Float y2);
    virtual casacore::Float rnd(casacore::Float x, casacore::Int nsub);
    virtual casacore::Vector<casacore::Float> rnge(casacore::Float x1, casacore::Float x2);
    virtual void sah(casacore::Int fs, casacore::Float angle, casacore::Float vent);
    virtual void save();
    virtual void scf(casacore::Int font);
    virtual void sch(casacore::Float size);
    virtual void sci(casacore::Int ci);
    virtual void scir(casacore::Int icilo, casacore::Int icihi);
    virtual void scr(casacore::Int ci, casacore::Float cr, casacore::Float cg, casacore::Float cb);
    virtual void scrn(casacore::Int ci, const casacore::String &name);
    virtual void sfs(casacore::Int fs);
    virtual void shls(casacore::Int ci, casacore::Float ch, casacore::Float cl, casacore::Float cs);
    virtual void shs(casacore::Float angle, casacore::Float sepn, casacore::Float phase);
    virtual void sitf(casacore::Int itf);
    virtual void sls(casacore::Int ls);
    virtual void slw(casacore::Int lw);
    virtual void stbg(casacore::Int tbci);
    virtual void subp(casacore::Int nxsub, casacore::Int nysub);
    virtual void svp(casacore::Float xleft, casacore::Float xright, casacore::Float ybot, casacore::Float ytop);
    virtual void swin(casacore::Float x1, casacore::Float x2, casacore::Float y1, casacore::Float y2);
    virtual void tbox(const casacore::String &xopt, casacore::Float xtick, casacore::Int nxsub,
		    const casacore::String &yopt, casacore::Float ytick, casacore::Int nysub);
    virtual void text(casacore::Float x, casacore::Float y, const casacore::String &text);
    virtual void unsa();
    virtual void updt();
    virtual void vect(const casacore::Matrix<casacore::Float> &a, const casacore::Matrix<casacore::Float> &b,
		      casacore::Float c, casacore::Int nc, 
		      const casacore::Vector<casacore::Float> &tr, casacore::Float blank);
    virtual void vsiz(casacore::Float xleft, casacore::Float xright, casacore::Float ybot,
		      casacore::Float ytop);
    virtual void vstd();
    virtual void wedg(const casacore::String &side, casacore::Float disp, casacore::Float width,
		      casacore::Float fg, casacore::Float bg, const casacore::String &label);
    virtual void wnad(casacore::Float x1, casacore::Float x2, casacore::Float y1, casacore::Float y2);
    // </group>

 private:
    // Undefined and inaccessible
    PGPlotterLocal(const PGPlotterLocal &);
    PGPlotterLocal& operator=(const PGPlotterLocal &);

    // PGPLOT id
    int id_p;
};



} //# NAMESPACE CASA - END

#endif
