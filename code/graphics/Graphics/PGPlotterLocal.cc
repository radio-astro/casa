//# PGPlotterLocal.cc: Plot to a PGPLOT device "local" to this process.
//# Copyright (C) 1997,2001,2002
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

#include <graphics/Graphics/PGPlotterLocal.h>
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <casa/Containers/Record.h>
#include <casa/iostream.h>

#include <cpgplot.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// This helper class just avoids some repetitious typing in many functions.
// It's purpose is to just call getstorage/freestorage.
class PGPtrGetter {
public:
    PGPtrGetter(const Vector<Float> &thevec) :
	vec_p(thevec) { ptr_p = vec_p.getStorage(delete_p);}
    ~PGPtrGetter() { vec_p.freeStorage(ptr_p, delete_p); }

    const Float *ptr() {return ptr_p;}
    int n() {return vec_p.nelements();}
private:
    PGPtrGetter();
    PGPtrGetter(const PGPtrGetter&);
    PGPtrGetter &operator=(const PGPtrGetter&);

    const Vector<Float> &vec_p;
    const Float *ptr_p;
    Bool delete_p;
};

PGPlotterLocal::PGPlotterLocal(const String &device)
    : id_p(cpgopen(device.chars()))
{
    // If this fails, we need a bit more development to copy Float*'s to
    // float*'s.
    AlwaysAssertExit(sizeof(Float) == sizeof(float));

    if (id_p <= 0) {
	cerr << device << endl;
	throw(AipsError(String("PGPLOT cannot open device : ") + device));
    }
}

PGPlotterLocal::~PGPlotterLocal()
{
    cpgslct(id_p);
    cpgclos();
    id_p = -1;
}

PGPlotter PGPlotterLocal::createPlotter (const String &device,
					 uInt, uInt, uInt, uInt)
{
    return PGPlotter (new PGPlotterLocal (device));
}

Record PGPlotterLocal::curs(Float X, Float Y)
{
    float x=X, y=Y;
    char buf[] = {0,0,0,0,0,0,0};
    int ok = cpgcurs(&x, &y, &buf[0]);
    Record retval;
    retval.define("ok", (ok));
    retval.define("x", x);
    retval.define("y", y);
    retval.define("ch", String(buf));
    return retval;
}


void PGPlotterLocal::arro(Float x1, Float y1, Float x2, Float y2)
{
    cpgslct(id_p);
    cpgarro(x1, y1, x2, y2);
}

void PGPlotterLocal::ask(Bool flag)
{
    cpgslct(id_p);
    cpgask(Logical(flag));
}

void PGPlotterLocal::bbuf()
{
    cpgslct(id_p);
    cpgbbuf();
}

void PGPlotterLocal::box(const String &xopt, Float xtick, Int nxsub, 
	     const String &yopt, Float ytick, Int nysub)
{
    cpgslct(id_p);
    cpgbox(xopt.chars(), xtick, nxsub, yopt.chars(), ytick, nysub);
}

void PGPlotterLocal::circ(Float xcent, Float ycent, Float radius)
{
    cpgslct(id_p);
    cpgcirc(xcent, ycent, radius);
}

void PGPlotterLocal::draw(Float x, Float y)
{
    cpgslct(id_p);
    cpgdraw(x,y);
}

void PGPlotterLocal::ebuf()
{
    cpgslct(id_p);
    cpgebuf();
}

void PGPlotterLocal::env(Float xmin, Float xmax, Float ymin, Float ymax, 
			 Int just, Int axis)
{
    cpgslct(id_p);
    cpgenv(xmin, xmax, ymin, ymax, just, axis);
}

void PGPlotterLocal::eras()
{
    cpgslct(id_p);
    cpgeras();
}

void PGPlotterLocal::errb(Int dir, const Vector<Float> &x, 
			  const Vector<Float> &y,
			  const Vector<Float> &e, Float t)
{
    cpgslct(id_p);
    PGPtrGetter X(x), Y(y), E(e);
    cpgerrb(dir, X.n(), X.ptr(), Y.ptr(), E.ptr(), t);
}

void PGPlotterLocal::erry(const Vector<Float> &x, const Vector<Float> &y1,
	      const Vector<Float> &y2, Float t)
{
    cpgslct(id_p);
    PGPtrGetter X(x), Y1(y1), Y2(y2);
    cpgerry(X.n(), X.ptr(), Y1.ptr(), Y2.ptr(), t);
}

void PGPlotterLocal::hist(const Vector<Float> &data, Float datmin, 
			  Float datmax, 
			  Int nbin, Int pcflag)
{
    PGPtrGetter DATA(data);
    cpgslct(id_p);
    cpghist(DATA.n(), DATA.ptr(), datmin, datmax, nbin, pcflag);
}

void PGPlotterLocal::lab(const String &xlbl, const String &ylbl, 
		   const String &toplbl)
{
    cpgslct(id_p);
    cpglab(xlbl.chars(), ylbl.chars(), toplbl.chars());
}

void PGPlotterLocal::line(const Vector<Float> &xpts, const Vector<Float> &ypts)
{
    cpgslct(id_p);
    PGPtrGetter XPTS(xpts), YPTS(ypts);
    cpgline(XPTS.n(), XPTS.ptr(), YPTS.ptr());
}

void PGPlotterLocal::move(Float x, Float y)
{
    cpgslct(id_p);
    cpgmove(x,y);
}

void PGPlotterLocal::mtxt(const String &side, Float disp, Float coord, 
			  Float fjust, const String &text)
{
    cpgslct(id_p);
    cpgmtxt(side.chars(), disp, coord, fjust, text.chars());
}

void PGPlotterLocal::page()
{
    cpgslct(id_p);
    cpgpage();
}

void PGPlotterLocal::poly(const Vector<Float> &xpts, const Vector<Float> &ypts)
{
    cpgslct(id_p);
    PGPtrGetter XPTS(xpts), YPTS(ypts);
    cpgpoly(XPTS.n(), XPTS.ptr(), YPTS.ptr());
}

void PGPlotterLocal::pt(const Vector<Float> &xpts, const Vector<Float> &ypts, 
		  Int symbol)
{
    cpgslct(id_p);
    PGPtrGetter XPTS(xpts), YPTS(ypts);
    cpgpt(XPTS.n(), XPTS.ptr(), YPTS.ptr(), symbol);
}

void PGPlotterLocal::ptxt(Float x, Float y, Float angle, Float fjust, 
		    const String &text)
{
    cpgslct(id_p);
    cpgptxt(x, y, angle, fjust, text.chars());
}

Int PGPlotterLocal::qci()
{
    cpgslct(id_p);
    int i;
    cpgqci(&i);
    return i;
}

Int PGPlotterLocal::qtbg()
{
    cpgslct(id_p);
    int i;
    cpgqtbg(&i);
    return i;
}

Vector<Float> PGPlotterLocal::qtxt(Float x, Float y, Float angle, Float fjust, 
		    const String &text)
{
    cpgslct(id_p);
    Vector<Float> xboxybox(8);
    Bool del;
    Float *ptr = xboxybox.getStorage(del);
    Float *xbox = ptr, *ybox = ptr + 4;
    cpgqtxt(x, y, angle, fjust, text.chars(), xbox, ybox);
    return xboxybox;
}

Vector<Float> PGPlotterLocal::qwin()
{
    cpgslct(id_p);
    Vector<Float> retval(4);
    Bool del;
    Float *ptr = retval.getStorage(del);
    Float *x1 = ptr, *x2 = ptr+1, *y1=ptr+2, *y2=ptr+3;
    cpgqwin(x1, x2, y1, y2);
    return retval;
}

void PGPlotterLocal::rect(Float x1, Float x2, Float y1, Float y2)
{
    cpgslct(id_p);
    cpgrect(x1, x2, y1, y2);
}

void PGPlotterLocal::sah(Int fs, Float angle, Float vent)
{
    cpgslct(id_p);
    cpgsah(fs, angle, vent);
}

void PGPlotterLocal::save()
{
    cpgslct(id_p);
    cpgsave();
}

void PGPlotterLocal::sch(Float size)
{
    cpgslct(id_p);
    cpgsch(size);
}

void PGPlotterLocal::sci(Int ci)
{
    cpgslct(id_p);
    cpgsci(ci);
}

void PGPlotterLocal::scr(Int ci, Float cr, Float cg, Float cb)
{
    cpgslct(id_p);
    cpgscr(ci, cr, cg, cb);
}

void PGPlotterLocal::sfs(Int fs)
{
    cpgslct(id_p);
    cpgsfs(fs);
}

void PGPlotterLocal::sls(Int ls)
{
    cpgslct(id_p);
    cpgsls(ls);
}

void PGPlotterLocal::slw(Int lw)
{
    cpgslct(id_p);
    cpgslw(lw);
}

void PGPlotterLocal::stbg(Int tbci)
{
    cpgslct(id_p);
    cpgstbg(tbci);
}

void PGPlotterLocal::subp(Int nxsub, Int nysub)
{
    cpgslct(id_p);
    cpgsubp(nxsub, nysub);
}

void PGPlotterLocal::svp(Float xleft, Float xright, Float ybot, Float ytop)
{
    cpgslct(id_p);
    cpgsvp(xleft, xright, ybot, ytop);
}

void PGPlotterLocal::swin(Float x1, Float x2, Float y1, Float y2)
{
    cpgslct(id_p);
    cpgswin(x1, x2, y1, y2);
}

void PGPlotterLocal::tbox(const String &xopt, Float xtick, Int nxsub,
			  const String &yopt, Float ytick, Int nysub)
{
    cpgslct(id_p);
    cpgtbox(xopt.chars(), xtick, nxsub, yopt.chars(), ytick, nysub);
}

void PGPlotterLocal::text(Float x, Float y, const String &text)
{
    cpgslct(id_p);
    cpgtext(x,y,text.chars());
}

void PGPlotterLocal::unsa()
{
    cpgslct(id_p);
    cpgunsa();
}

void PGPlotterLocal::updt()
{
    cpgslct(id_p);
    cpgupdt();
}

void PGPlotterLocal::vstd()
{
    cpgslct(id_p);
    cpgvstd();
}

void PGPlotterLocal::wnad(Float x1, Float x2, Float y1, Float y2)
{
    cpgslct(id_p);
    cpgwnad(x1, x2, y1, y2);
}

void PGPlotterLocal::conl(const Matrix<Float> &a, Float c,
			  const Vector<Float> &tr, const String &label,
			  Int intval, Int minint)
{
    cpgslct(id_p);
    Bool dela, deltr;
    const Float *aptr = a.getStorage(dela);
    const Float *trptr = tr.getStorage(deltr);
    Int nx = a.ncolumn();
    Int ny = a.nrow();
    cpgconl(aptr, nx, ny, 1, nx, 1, ny, c, trptr, label.chars(), intval,
	    minint);
    a.freeStorage(aptr, dela);
    tr.freeStorage(trptr, deltr);
}

void PGPlotterLocal::cont(const Matrix<Float> &a, const Vector<Float> &c,
			  Bool nc, const Vector<Float> &tr)
{
    cpgslct(id_p);
    Bool dela, delc, deltr;
    const Float *aptr = a.getStorage(dela);
    const Float *cptr =  c.getStorage(delc);
    const Float *trptr = tr.getStorage(deltr);
    Int nx = a.ncolumn();
    Int ny = a.nrow();
    Int pgnc = c.nelements();
    if (!nc) pgnc *= -1;
    cpgcont(aptr, nx, ny, 1, nx, 1, ny, cptr, pgnc,
	    trptr);
    a.freeStorage(aptr, dela);
    c.freeStorage(cptr, delc);
    tr.freeStorage(trptr, deltr);
}

void PGPlotterLocal::ctab(const Vector<Float> &l, const Vector<Float> &r,
			  const Vector<Float> &g, const Vector<Float> &b,
			  Float contra, Float bright)
{
    cpgslct(id_p);
    Bool dell, delr, delg, delb;
    const Float *lptr = l.getStorage(dell);
    const Float *rptr = r.getStorage(delr);
    const Float *gptr = g.getStorage(delg);
    const Float *bptr = b.getStorage(delb);
    cpgctab(lptr, rptr, gptr, bptr, l.nelements(), contra, bright);
    l.freeStorage(lptr, dell);
    r.freeStorage(rptr, delr);
    g.freeStorage(gptr, delg);
    b.freeStorage(bptr, delb);
}

void PGPlotterLocal::gray(const Matrix<Float> &a, Float fg, Float bg,
			  const Vector<Float> &tr)
{
    cpgslct(id_p);
    Bool dela, deltr;
    const Float *aptr = a.getStorage(dela);
    const Float *trptr = tr.getStorage(deltr);
    Int nx = a.nrow();
    Int ny = a.ncolumn();
    cpggray(aptr, nx, ny, 1, nx, 1, ny, fg, bg, trptr);
    a.freeStorage(aptr, dela);
    tr.freeStorage(trptr, deltr);
} 

void PGPlotterLocal::iden()
{
    cpgslct(id_p);
    cpgiden();
}

void PGPlotterLocal::imag(const Matrix<Float> &a, Float a1, Float a2,
			  const Vector<Float> &tr)
{
    cpgslct(id_p);
    Bool dela, deltr;
    const Float *aptr = a.getStorage(dela);
    const Float *trptr = tr.getStorage(deltr);
    Int nx = a.nrow();
    Int ny = a.ncolumn();
    cpgimag(aptr, nx, ny, 1, nx, 1, ny, a1, a2, trptr);
    a.freeStorage(aptr, dela);
    tr.freeStorage(trptr, deltr);
}

Vector<Int> PGPlotterLocal::qcir()
{
    cpgslct(id_p);
    int i,j;
    Vector<Int> retval(2);
    cpgqcir(&i, &j);
    retval(0) = i; retval(1) = j;
    return retval;
}

Vector<Int> PGPlotterLocal::qcol()
{
    cpgslct(id_p);
    int i,j;
    Vector<Int> retval(2);
    cpgqcol(&i, &j);
    retval(0) = i; retval(1) = j;
    return retval;
}

void PGPlotterLocal::scir(Int icilo, Int icihi)
{
    cpgslct(id_p);
    cpgscir(icilo, icihi);
}

void PGPlotterLocal::sitf(Int itf)
{
    cpgslct(id_p);
    cpgsitf(itf);
}

void PGPlotterLocal::bin(const Vector<Float> &x, const Vector<Float> &data,
		     Bool center)
{
    cpgslct(id_p);
    PGPtrGetter X(x);
    PGPtrGetter DATA(data);
    cpgbin(X.n(), X.ptr(), DATA.ptr(), Logical(center));
}

void PGPlotterLocal::conb(const Matrix<Float> &a, const Vector<Float> &c,
		      const Vector<Float> &tr, Float blank)
{
    cpgslct(id_p);
    PGPtrGetter C(c);
    PGPtrGetter TR(tr);
    Bool dela;
    const Float *aptr = a.getStorage(dela);
    int nx = a.nrow();
    int ny = a.ncolumn();
    cpgconb(aptr, nx, ny, 1, nx, 1, ny, C.ptr(), C.n(), TR.ptr(), blank);
}

void PGPlotterLocal::cons(const Matrix<Float> &a, const Vector<Float> &c,
		      const Vector<Float> &tr)
{
    cpgslct(id_p);
    PGPtrGetter C(c);
    PGPtrGetter TR(tr);
    Bool dela;
    const Float *aptr = a.getStorage(dela);
    int nx = a.nrow();
    int ny = a.ncolumn();
    cpgcons(aptr, nx, ny, 1, nx, 1, ny, C.ptr(), C.n(), TR.ptr());
    a.freeStorage(aptr, dela);
}

void PGPlotterLocal::errx(const Vector<Float> &x1, const Vector<Float> &x2,
		      const Vector<Float> &y, Float t)
{
    cpgslct(id_p);
    PGPtrGetter X1(x1), X2(x2), Y(y);
    cpgerrx(X1.n(), X1.ptr(), X2.ptr(), Y.ptr(), t);
}

void PGPlotterLocal::hi2d(const Matrix<Float> &data, const Vector<Float> &x,
		      Int ioff, Float bias, Bool center, 
		      const Vector<Float> &ylims)
{
    //##### ylims should come out of the binding

    cpgslct(id_p);
    PGPtrGetter X(x), YLIMS(ylims);
    int nx = data.nrow();
    int ny = data.ncolumn();
    Bool deldata;
    const Float *dataptr = data.getStorage(deldata);
    cpghi2d(dataptr, nx, ny, 1, nx, 1, ny, X.ptr(), ioff, bias, Logical(center),
	    (float *)YLIMS.ptr());
    data.freeStorage(dataptr, deldata);
}

void PGPlotterLocal::ldev()
{
    cpgslct(id_p);
    cpgldev();
}

Vector<Float> PGPlotterLocal::len(Int units, const String &string)
{
    cpgslct(id_p);
    Vector<Float> retval(2);
    cpglen(units, string.chars(), &retval(0), &retval(1));
    return retval;
}

String PGPlotterLocal::numb(Int mm, Int pp, Int form)
{
    cpgslct(id_p);
    char chars[100];
    int string_length;
    cpgnumb(mm, pp, form, &chars[0], &string_length);
    chars[string_length] = 0;
    return String(chars);
}

void PGPlotterLocal::panl(Int ix, Int iy)
{
    cpgslct(id_p);
    cpgpanl(ix, iy);
}

void PGPlotterLocal::pap(Float width, Float aspect)
{
    cpgslct(id_p);
    cpgpap(width, aspect);
}

void PGPlotterLocal::pixl(const Matrix<Int> &ia, Float x1, Float x2,
		      Float y1, Float y2)
{
    cpgslct(id_p);
    Bool delia;
    const Int *iaptr = ia.getStorage(delia);
    int nx = ia.nrow();
    int ny = ia.ncolumn();
    cpgpixl(iaptr, nx, ny, 1, nx, 1, ny, x1, x2, y1, y2);
    ia.freeStorage(iaptr, delia);
}

void PGPlotterLocal::pnts(const Vector<Float> &x, const Vector<Float> &y,
		      const Vector<Int> symbol)
{
    cpgslct(id_p);
    PGPtrGetter X(x), Y(y);
    Bool delsymbol;
    const Int *symbolptr = symbol.getStorage(delsymbol);
    cpgpnts(X.n(), X.ptr(), Y.ptr(), symbolptr, symbol.nelements());
    symbol.freeStorage(symbolptr, delsymbol);
}

Vector<Float>  PGPlotterLocal::qah()
{
    cpgslct(id_p);
    Vector<Float> retval(3);
    int tmp;
    cpgqah(&tmp, &retval(1), &retval(2)); 
    retval(0) = tmp;
    return retval;
}

Int PGPlotterLocal::qcf()
{
    cpgslct(id_p);
    int retval;
    cpgqcf(&retval);
    return retval;
}

Float PGPlotterLocal::qch()
{
    cpgslct(id_p);
    float retval;
    cpgqch(&retval);
    return retval;
}

Vector<Float> PGPlotterLocal::qcr(Int ci)
{
    cpgslct(id_p);
    Vector<Float> retval(3);
    cpgqcr(ci, &retval(0), &retval(1), &retval(2));
    return retval;
}

Vector<Float> PGPlotterLocal::qcs(Int units)
{
    cpgslct(id_p);
    Vector<Float> retval(2);
    cpgqcs(units, &retval(0), &retval(1));
    return retval;
}

Int PGPlotterLocal::qfs()
{
    cpgslct(id_p);
    int retval;
    cpgqfs(&retval);
    return retval;
}

Vector<Float> PGPlotterLocal::qhs()
{
    cpgslct(id_p);
    Vector<Float> retval(3);
    cpgqhs(&retval(0), &retval(1), &retval(2));
    return retval;
}

Int PGPlotterLocal::qid()
{
    cpgslct(id_p);
    int retval;
    cpgqid(&retval);
    return retval;
}

String PGPlotterLocal::qinf(const String &item)
{
    cpgslct(id_p);
    char chars[10000];
    int chars_length;
    cpgqinf(item.chars(), &chars[0], &chars_length);
    chars[chars_length] = 0;
    return String(chars);
}

Int PGPlotterLocal::qitf()
{
    cpgslct(id_p);
    int retval;
    cpgqitf(&retval);
    return retval;
}

Int PGPlotterLocal::qls()
{
    cpgslct(id_p);
    int retval;
    cpgqls(&retval);
    return retval;
}

Int PGPlotterLocal::qlw()
{
    cpgslct(id_p);
    int retval;
    cpgqlw(&retval);
    return retval;
}

Vector<Float> PGPlotterLocal::qpos()
{
    cpgslct(id_p);
    Vector<Float> retval(2);
    cpgqpos(&retval(0), &retval(1));
    return retval;
}

Vector<Float> PGPlotterLocal::qvp(Int units)
{
    cpgslct(id_p);
    Vector<Float> retval(4);
    cpgqvp(units, &retval(0), &retval(1), &retval(2), &retval(3));
    return retval;
}

Vector<Float> PGPlotterLocal::qvsz(Int units)
{
    cpgslct(id_p);
    Vector<Float> retval(4);
    cpgqvsz(units, &retval(0), &retval(1), &retval(2), &retval(3));
    return retval;
}

Float PGPlotterLocal::rnd(Float x, Int nsub)
{
    // #### Signature wrong!!!! nsub should be output!!!
    cpgslct(id_p);
    return cpgrnd(x, &nsub);
}

Vector<Float> PGPlotterLocal::rnge(Float x1, Float x2)
{
    cpgslct(id_p);
    Vector<Float> retval(2);
    cpgrnge(x1, x2, &retval(0), &retval(1));
    return retval;
}

void PGPlotterLocal::scf(Int font)
{
    cpgslct(id_p);
    cpgscf(font);
}

void PGPlotterLocal::scrn(Int ci, const String &name)
{
    cpgslct(id_p);
    int ierr;
    cpgscrn(ci, name.chars(), &ierr);
    AlwaysAssert(ierr == 0, AipsError);
}

void PGPlotterLocal::shls(Int ci, Float ch, Float cl, Float cs)
{
    cpgslct(id_p);
    cpgshls(ci, ch, cl, cs);
}

void PGPlotterLocal::shs(Float angle, Float sepn, Float phase)
{
    cpgslct(id_p);
    cpgshs(angle, sepn, phase);
}

void PGPlotterLocal::vect(const Matrix<Float> &a, const Matrix<Float> &b,
		      Float c, Int nc, 
		      const Vector<Float> &tr, Float blank)
{
    cpgslct(id_p);
    Bool dela, delb;
    const Float *aptr = a.getStorage(dela);
    const Float *bptr = b.getStorage(delb);
    PGPtrGetter TR(tr);
    int nx = a.nrow();
    int ny = a.ncolumn();
    cpgvect(aptr, bptr, nx, ny, 1, nx, 1, ny, c, nc, TR.ptr(), blank);
    a.freeStorage(aptr, dela);
    b.freeStorage(bptr, delb);
}

void PGPlotterLocal::vsiz(Float xleft, Float xright, Float ybot,
		      Float ytop)
{
    cpgslct(id_p);
    cpgvsiz(xleft, xright, ybot, ytop);
}

void PGPlotterLocal::wedg(const String &side, Float disp, Float width,
		      Float fg, Float bg, const String &label)
{
    cpgslct(id_p);
    cpgwedg(side.chars(), disp, width, fg, bg, label.chars());
}

} //# NAMESPACE CASA - END

