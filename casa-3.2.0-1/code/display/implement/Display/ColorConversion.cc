//# ColorConversion.cc: utilities for converting between color spaces
//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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

#include <display/Display/ColorConversion.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// assumes h, s, and v to be in range [0, 1]. Returns r, g, b in range [0, 1].
void hsvToRgb(Float h, Float s, Float v,
	      Float &red, Float &green, Float &blue) 
{
  Float f, a, b, c;
  Int i;

  if (s == 0) {
    /* must be grayscale */
    red = v;
    green = v;
    blue = v;
  }
  else {
    if (h == 1.0)
      h = 0.0;
    
    h = h * 6.0;
    i = Int(std::floor (h));
    f = h - i;

    a = v * (1 - s);
    b = v * (1 - (s * f));
    c = v * (1 - (s * (1 - f)));

    switch (i) {
    case 0:
      red = v;      green = c;      blue = a;      break;
    case 1:
      red = b;      green = v;      blue = a;      break;
    case 2:
      red = a;      green = v;      blue = c;      break;
    case 3:
      red = a;      green = b;      blue = v;      break;
    case 4:
      red = c;      green = a;      blue = v;      break;
    case 5:
      red = v;      green = a;      blue = b;      break;
    }
  }
}

void hsvToRgb(const Array<Float> & h, const Array<Float> & s, const Array<Float> & v,
	      Array<Float> & r, Array<Float> & g, Array<Float> & b)
{
  Bool hDel;
  const Float * hp = h.getStorage(hDel);
  Bool sDel;
  const Float * sp = s.getStorage(sDel);
  Bool vDel;
  const Float * vp = v.getStorage(vDel);
  Bool rDel;
  Float * rp = r.getStorage(rDel);
  Bool gDel;
  Float * gp = g.getStorage(gDel);
  Bool bDel;
  Float * bp = b.getStorage(bDel);

  const Float * hpend = hp + h.nelements();

  const Float * hq = hp;
  const Float * sq = sp;
  const Float * vq = vp;
  Float * rq = rp;
  Float * gq = gp;
  Float * bq = bp;

  while (hq < hpend)
    hsvToRgb(*hq++, *sq++, *vq++, *rq++, *gq++, *bq++);

  h.freeStorage(hp, hDel);
  s.freeStorage(sp, sDel);
  v.freeStorage(vp, vDel);

  r.putStorage(rp, rDel);
  g.putStorage(gp, gDel);
  b.putStorage(bp, bDel);

  return;
}

//Assumes r, g, and b to be in range [0, 1]. Returns h, s, v in range [0, 1].
void rgbToHsv(Float r, Float g, Float b, 
	      Float &h, Float &s, Float &v)
{
  Float max = r; if (g > max) max = g; if (b > max) max = b;
  Float min = r; if (g < min) min = g; if (b < min) min = b;
  Float delta = max - min;
  
  v = max;
  if (max != 0.0) s = delta / max;
  else s = 0.0;
  
  if (s == 0.0) h = 0.0;
  else {
    if (r == max) h = (g - b) / delta;
    else
      if (g == max) h = 2 + (b - r) / delta;
      else
	if (b == max) h = 4 + (r - g) / delta;
    h = h * 60.0;
    if (h < 0) h = h + 360.0;
    h = h / 360.0;
  }
}

void rgbToHsv(const Array<Float> & r, const Array<Float> & g, const Array<Float> & b,
	      Array<Float> & h, Array<Float> & s, Array<Float> & v)
{
  Bool rDel;
  const Float * rp = r.getStorage(rDel);
  Bool gDel;
  const Float * gp = g.getStorage(gDel);
  Bool bDel;
  const Float * bp = b.getStorage(bDel);
  Bool hDel;
  Float * hp = h.getStorage(hDel);
  Bool sDel;
  Float * sp = s.getStorage(sDel);
  Bool vDel;
  Float * vp = v.getStorage(vDel);

  const Float * rpend = rp + r.nelements();

  const Float * rq = rp;
  const Float * gq = gp;
  const Float * bq = bp;
  Float * hq = hp;
  Float * sq = sp;
  Float * vq = vp;

  while (rq < rpend)
    rgbToHsv(*rq++, *gq++, *bq++, *hq++, *sq++, *vq++);

  r.freeStorage(rp, rDel);
  g.freeStorage(gp, gDel);
  b.freeStorage(bp, bDel);
  
  h.putStorage(hp, hDel);
  s.putStorage(sp, sDel);
  v.putStorage(vp, vDel);

  return;
}


} //# NAMESPACE CASA - END

