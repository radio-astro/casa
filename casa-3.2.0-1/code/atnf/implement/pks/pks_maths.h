//#---------------------------------------------------------------------------
//# pks_maths.h: Mathematical functions for Parkes single dish data reduction
//#---------------------------------------------------------------------------
//# livedata - processing pipeline for single-dish, multibeam spectral data.
//# Copyright (C) 2000-2009, Australia Telescope National Facility, CSIRO
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
//# $Id: pks_maths.h,v 1.7 2009-09-29 07:45:02 cal103 Exp $
//#---------------------------------------------------------------------------
//# Original: 2004/07/16 Mark Calabretta
//#---------------------------------------------------------------------------
#ifndef ATNF_PKS_MATHS_H
#define ATNF_PKS_MATHS_H

// AIPS++ includes.
#include <casa/aips.h>
#include <casa/Arrays/Vector.h>


#include <casa/namespace.h>

// Global mathematical functions for single-dish data reduction.

// Nearest integral value.
Int nint(Double v);
Double anint(Double v);

// Round value v to the nearest integral multiple of precision p.
Double round(Double v, Double p);

// Compute the weighted median value of an array.
Float  median(const Vector<Float> &v, const Vector<Float> &wgt);

// Angular distance between two directions (angles in radian).
Double angularDist(Double lng1, Double lat1, Double lng2, Double lat2);

// Generalized position angle of the field point (lng,lat) from reference
// point (lng0,lat0) and the angular distance between them (angles in radian).
void distPA(Double lng0, Double lat0, Double lng, Double lat, Double &dist,
            Double &pa);

// Euler angle based transformation of spherical coordinates (radian).
void eulerx(Double lng0, Double lat0, Double phi0, Double theta, Double phi,
            Double &lng1, Double &lat1);

// Low precision coordinates of the sun.
void sol(Double mjd, Double &elng, Double &ra, Double &dec);

// Low precision Greenwich mean and apparent sidereal time (radian); UT1 is
// given in MJD form.
void gst(Double ut1, Double &gmst, Double &gast);

// Convert (ra,dec) to (az,el).  Position as a Cartesian triplet (m), UT1 in
// MJD form, and all angles in radian.
void azel(const Vector<Double> position, Double ut1, Double ra, Double dec,
          Double &az, Double &el);

// Compute the Solar elevation (radian) using the above functions.
Double solel(const Vector<Double> position, Double ut1);

#endif
