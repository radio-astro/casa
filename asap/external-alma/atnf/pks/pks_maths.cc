//#---------------------------------------------------------------------------
//# pks_maths.cc: Mathematical functions for Parkes single-dish data reduction
//#---------------------------------------------------------------------------
//# livedata - processing pipeline for single-dish, multibeam spectral data.
//# Copyright (C) 2004-2009, Australia Telescope National Facility, CSIRO
//#
//# This file is part of livedata.
//#
//# livedata is free software: you can redistribute it and/or modify it under
//# the terms of the GNU General Public License as published by the Free
//# Software Foundation, either version 3 of the License, or (at your option)
//# any later version.
//#
//# livedata is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with livedata.  If not, see <http://www.gnu.org/licenses/>.
//#
//# Correspondence concerning livedata may be directed to:
//#        Internet email: mcalabre@atnf.csiro.au
//#        Postal address: Dr. Mark Calabretta
//#                        Australia Telescope National Facility, CSIRO
//#                        PO Box 76
//#                        Epping NSW 1710
//#                        AUSTRALIA
//#
//# http://www.atnf.csiro.au/computing/software/livedata.html
//# $Id: pks_maths.cc,v 1.7 2009-09-29 07:45:02 cal103 Exp $
//#---------------------------------------------------------------------------
//# Original: 2004/07/16 Mark Calabretta
//#---------------------------------------------------------------------------

// AIPS++ includes.
#include <complex>
#include <casa/aips.h>
#include <casa/math.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Utilities/GenSort.h>

// Parkes includes.
#include <atnf/pks/pks_maths.h>


//----------------------------------------------------------------------- nint

// Nearest integral value; halfway cases are rounded to the integral value
// larger in value.  No check is made for integer overflow.

Int nint(Double v)
{
  return Int(floor(v + 0.5));
}

//---------------------------------------------------------------------- anint

// Nearest integral value; halfway cases are rounded to the integral value
// larger in value.

Double anint(Double v)
{
  return floor(v + 0.5);
}

//---------------------------------------------------------------------- round

// Round value v to the nearest integral multiple of precision p.

Double round(Double v, Double p)
{
  return p * floor(v/p + 0.5);
}

//--------------------------------------------------------------------- median

// Compute the weighted median value of an array.

Float median(const Vector<Float> &v, const Vector<Float> &wgt)
{
  uInt nElem = v.nelements();
  if (nElem == 0) return 0.0f;

  // Generate the sort index.
  Vector<uInt> sortindex(nElem);
  GenSortIndirect<Float>::sort(sortindex, v);

  // Find the middle weight.
  Float wgt_2 = sum(wgt)/2.0f;

  // Find the corresponding vector element.
  Float weight = 0.0f;
  Float accwgt = 0.0f;
  uInt j1 = 0;
  uInt j2;
  for (j2 = 0; j2 < nElem; j2++) {
    weight = wgt(sortindex(j2));
    if (weight == 0.0f) {
      // Ignore zero-weight data;
      continue;
    }

    // The accumulated weight.
    accwgt += weight;

    if (accwgt <= wgt_2) {
      // Keep looping.
      j1 = j2;
    } else {
      break;
    }
  }

  // Compute weighted median.
  Float v1 = v(sortindex(j1));
  Float v2 = v(sortindex(j2));

  // Compute pro-rata value from below.
  Float dw = wgt_2 - (accwgt - weight);
  v1 += (v2 - v1) * dw / weight;

  // Find next non-zero-weight value.
  for (j2++ ; j2 < nElem; j2++) {
    weight = wgt(sortindex(j2));
    if (weight != 0.0f) {
      break;
    }
  }

  if (j2 < nElem) {
    // Compute pro-rata value from above.
    Float v3 = v(sortindex(j2));

    v2 += (v3 - v2) * dw / weight;
  }

  return (v1 + v2)/2.0f;
}

//---------------------------------------------------------------- angularDist

// Determine the angular distance between two directions (angles in radians).

Double angularDist(Double lng0, Double lat0, Double lng, Double lat)
{
  Double costheta = sin(lat0)*sin(lat) + cos(lat0)*cos(lat)*cos(lng0-lng);
  return acos(costheta);
}

//--------------------------------------------------------------------- distPA

void distPA(Double lng0, Double lat0, Double lng, Double lat, Double &dist,
            Double &pa)

// Determine the generalized position angle of the field point (lng,lat) from
// the reference point (lng0,lat0) and the angular distance between them
// (angles in radians).

{
  // Euler angles which rotate the coordinate frame so that (lng0,lat0) is
  // at the pole of the new system, with the pole of the old system at zero
  // longitude in the new.
  Double phi0  =  C::pi_2 + lng0;
  Double theta =  C::pi_2 - lat0;
  Double phi   = -C::pi_2;

  // Rotate the field point to the new system.
  Double alpha, beta;
  eulerx(lng, lat, phi0, theta, phi, alpha, beta);

  dist = C::pi_2 - beta;
  pa   = -alpha;
  if (pa < -C::pi) pa = pa + C::_2pi;
}

//--------------------------------------------------------------------- eulerx

void eulerx(Double lng0, Double lat0, Double phi0, Double theta, Double phi,
            Double &lng1, Double &lat1)

// Applies the Euler angle based transformation of spherical coordinates.
//
//     phi0  Longitude of the ascending node in the old system, radians.  The
//           ascending node is the point of intersection of the equators of
//           the two systems such that the equator of the new system crosses
//           from south to north as viewed in the old system.
//
//    theta  Angle between the poles of the two systems, radians.  THETA is
//           positive for a positive rotation about the ascending node.
//
//      phi  Longitude of the ascending node in the new system, radians.

{
  // Compute intermediaries.
  Double lng0p  = lng0 - phi0;
  Double slng0p = sin(lng0p);
  Double clng0p = cos(lng0p);
  Double slat0  = sin(lat0);
  Double clat0  = cos(lat0);
  Double ctheta = cos(theta);
  Double stheta = sin(theta);

  Double x = clat0*clng0p;
  Double y = clat0*slng0p*ctheta + slat0*stheta;

  // Longitude in the new system.
  if (x != 0.0 || y != 0.0) {
    lng1 = phi + atan2(y, x);
  } else {
    // Longitude at the poles in the new system is consistent with that
    // specified in the old system.
    lng1 = phi + lng0p;
  }
  lng1 = fmod(lng1, C::_2pi);
  if (lng1 < 0.0) lng1 += C::_2pi;

  lat1 = asin(slat0*ctheta - clat0*stheta*slng0p);
}

//------------------------------------------------------------------------ sol

// Low precision coordinates of the Sun (accurate to 1 arcmin between 1800 and
// 2200) from http://aa.usno.navy.mil/faq/docs/SunApprox.html matches closely
// that in the Astronomical Almanac.

void sol(Double mjd, Double &elng, Double &ra, Double &dec)
{
  Double d2r = C::pi/180.0;

  // Number of days since J2000.0.
  Double d = mjd - 51544.5;

  // Mean longitude and mean anomaly of the Sun (deg).
  Double L = 280.459 + 0.98564736*d;
  Double g = 357.529 + 0.98560028*d;

  // Apparent ecliptic longitude corrected for aberration (deg).
  g *= d2r;
  elng = L + 1.915*sin(g) + 0.020*sin(g+g);
  elng = fmod(elng, 360.0);
  if (elng < 0.0) elng += 360.0;

  // Obliquity of the ecliptic (deg).
  Double epsilon = 23.439 - 0.00000036*d;

  // Transform ecliptic to equatorial coordinates.
  elng *= d2r;
  epsilon *= d2r;
  ra  = atan2(cos(epsilon)*sin(elng), cos(elng));
  dec = asin(sin(epsilon)*sin(elng));
  if (ra < 0.0) ra += C::_2pi;
}

//------------------------------------------------------------------------ gst

// Greenwich mean sidereal time, and low precision Greenwich apparent sidereal
// time, both in radian, from http://aa.usno.navy.mil/faq/docs/GAST.html.  UT1
// is given in MJD form.

void gst(Double ut1, Double &gmst, Double &gast)
{
  Double d2r = C::pi/180.0;

  Double d  = ut1 - 51544.5;
  Double d0 = int(ut1) - 51544.5;
  Double h = 24.0*(d - d0);
  Double t = d / 35625.0;

  // GMST (hr).
  gmst = 6.697374558 + 0.06570982441908*d0 + 1.00273790935*h + 0.000026*t*t;
  gmst = fmod(gmst, 24.0);

  // Longitude of the ascending node of the Moon (deg).
  Double Omega = 125.04 - 0.052954*d;

  // Mean Longitude of the Sun (deg).
  Double L = 280.47 + 0.98565*d;

  // Obliquity of the ecliptic (deg).
  Double epsilon = 23.4393 - 0.0000004*d;

  // Approximate nutation in longitude (hr).
  Double dpsi = -0.000319*sin(Omega*d2r) - 0.000024*sin((L+L)*d2r);

  // Equation of the equinoxes (hr).
  Double eqeq = dpsi*cos(epsilon*d2r);

  // GAST (hr).
  gast = gmst + eqeq;
  gast = fmod(gast, 24.0);

  // Convert to radian.
  gmst *= C::pi/12.0;
  gast *= C::pi/12.0;
}

//----------------------------------------------------------------------- azel

// Convert (ra,dec) to (az,el).  Position as a Cartesian triplet in m, UT1 in
// MJD form, and all angles in radian.

void azel(const Vector<Double> position, Double ut1, Double ra, Double dec,
          Double &az, Double &el)
{
  // Get geocentric longitude and latitude (rad).
  Double x = position(0);
  Double y = position(1);
  Double z = position(2);
  Double r = sqrt(x*x + y*y + z*z);
  Double lng = atan2(y, x);
  Double lat = asin(z/r);

  // Get GAST (rad).
  Double gast, gmst;
  gst(ut1, gmst, gast);

  // Local hour angle (rad).
  Double ha = (gast + lng) - ra;

  // Azimuth and elevation (rad).
  az = atan2(-cos(dec)*sin(ha),
            sin(dec)*cos(lat) - cos(dec)*sin(lat)*cos(ha));
  el = asin(sin(dec)*sin(lat) + cos(dec)*cos(lat)*cos(ha));

  if (az < 0.0) az += C::_2pi;
}

//---------------------------------------------------------------------- solel

// Compute the Solar elevation using the above functions.

Double solel(const Vector<Double> position, Double ut1)
{
  Double az, dec, el, elng, gast, gmst, ra;
  sol(ut1, elng, ra, dec);
  gst(ut1, gmst, gast);
  azel(position, ut1, ra, dec, az, el);
  return el;
}
