//# BeamCalc.cc: Implementation for BeamCalc
//# Copyright (C) 1996,1997,1998,1999,2000,2002
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

//#include <stdio.h>
//#include <complex.h>
#include <casa/math.h>
//#include <stdlib.h>
//#include <string.h>
#include <synthesis/MeasurementComponents/BeamCalcConstants.h>
#include <synthesis/MeasurementComponents/BeamCalc.h>
#include <synthesis/MeasurementComponents/ALMABeamCalc.h>
#include <synthesis/MeasurementComponents/BeamCalcAntenna.h>
#include <images/Images/TempImage.h>
#include <casa/Exceptions.h>
#include <synthesis/MeasurementComponents/SynthesisError.h>
namespace casa{
  
  
  calcAntenna *newALMAAntennafromApertureCalcParams(struct ALMAApertureCalcParams *ap)
  {
    calcAntenna *a;
    Double dir[3] = {0.0, 0.0, 1.0};
    Double sub_h, feed_x, feed_y, feed_z, thmax, ftaper;
    char geomfile[128], *feedfile;
    struct BeamCalcGeometry *geom;
    Int i;
    Double x, freq, df;
    
    geom = &ALMABeamCalcGeometryDefaults[ap->band];
    
    sub_h = geom->sub_h;
    feed_x = geom->feedpos[0]; feed_x = -feed_x;
    feed_y = geom->feedpos[1];
    feed_z = geom->feedpos[2];
    feedfile = 0;
    thmax = geom->subangle;
    
    freq = ap->freq;
    if(freq <= 0.0) freq = geom->reffreq;

    df = freq-geom->reffreq;
    x = 1.0;
    ftaper = 0.0;
    for(i = 0; i < geom->ntaperpoly; i++)
    {	
    	ftaper += geom->taperpoly[0]*x;
	x *= df;
    }
    sprintf(geomfile, "../../alma/%s.surface", geom->name);
    
    a = newAntenna(sub_h, feed_x, feed_y, feed_z, ftaper, thmax, geomfile);
    if(!a) return 0;
    
    strcpy(a->name, geom->name);
    
    /* feed pattern file is two column text file containing 
     * angle (in degrees) and power (in dBi) 
     */
    /* 
       if(feedfile != 0)
       {
       Double scale;
       scale = getKeyValueDouble(kv, "feedpatternscale");
       if(!Antennasetfeedpattern(a, feedfile, scale)) 
       {
       deleteAntenna(a);
       fprintf(stderr, "Problem with feed file <%s>\n",
       feedfile);
       return 0;
       }
       }
    */	
    Antennasetfreq(a, ap->freq);
    
    a->legwidth = geom->legwidth;
    a->legfoot  = geom->legfoot;
    a->legapex  = geom->legapex;
    
    a->hole_radius = geom->Rhole;
    
    Antennasetdir(a, dir);

    return a;
  }
  
  Pathology *newALMAPathologyfromApertureCalcParams(struct ALMAApertureCalcParams *ap)
  {
    Pathology *P;
    
    P = newPathology();
    
    return P;
  }

  
  Int ALMACalculateAperture(struct ALMAApertureCalcParams *ap)
  {
    Complex fp, Exr, Eyr, Exl, Eyl;
    Complex Er[3], El[3];
    Complex Pr[2], Pl[2]; 
    Complex E1[3], q[2];
    Double dx, dy, x0, y0, x, y, r2, Rhole, Rant, R2, H2, eps;
    Complex rr, rl, lr, ll, tmp;
    Double L0, L, amp, dP, dA, dO, x1, y1, dx1, dy1, dx2, dy2, phase;
    Double sp, cp;
    Ray *ray, *rayx, *rayy;
    Double B[3][3];
    calcAntenna *a;
    Pathology *p;
    Int nx, ny, os;
    Int iter, niter=6;
    Int i, j, index;
    Double pac, pas; /* parallactic angle cosine / sine */
    Complex Iota; Iota=Complex(0,1);
    
    a = newALMAAntennafromApertureCalcParams(ap);
    p = newALMAPathologyfromApertureCalcParams(ap);
    
    /* compute central ray pathlength */
    ray = trace(a, 0.0, 0.00001, p);
    L0 = Raylen(ray);
    deleteRay(ray);
    
    pac = cos(ap->pa+M_PI/2);
    pas = sin(ap->pa+M_PI/2);

#if 0
    /* compute polarization vectors in circular basis */
    Pr[0] = 1.0/M_SQRT2; Pr[1] =  Iota/M_SQRT2;
    Pl[0] = 1.0/M_SQRT2; Pl[1] = -Iota/M_SQRT2;
#endif
    
    /* in linear basis */
    Pr[0] = 1.0; Pr[1] = 0.0;
    Pl[0] = 0.0; Pl[1] = 1.0;
    
//     /* compensate for feed orientation */
//     getfeedbasis(a, B); 
//     phase = atan2(B[0][1], B[0][0]);
//     cp = cos(phase);
//     sp = sin(phase);
    
//     q[0] = Pr[0];
//     q[1] = Pr[1];
//     Pr[0] =  Complex(cp,0)*q[0] + Complex(sp,0)*q[1];
//     Pr[1] = -Complex(sp,0)*q[0] + Complex(cp,0)*q[1];
//     q[0] = Pl[0];
//     q[1] = Pl[1];
//     Pl[0] =  Complex(cp,0)*q[0] + Complex(sp,0)*q[1];
//     Pl[1] = -Complex(sp,0)*q[0] + Complex(cp,0)*q[1];
    
    
    /* compute 3-vector feed efields for the two polarizations */
    Efield(a, Pr, Er); 
    Efield(a, Pl, El); 
    
    /*
    printf("Er = %f %f  %f %f  %f %f\n",
	   creal(Er[0]), cimag(Er[0]),
	   creal(Er[1]), cimag(Er[1]),
	   creal(Er[2]), cimag(Er[2]));
    
    printf("El = %f %f  %f %f  %f %f\n",
	   creal(El[0]), cimag(El[0]),
	   creal(El[1]), cimag(El[1]),
	   creal(El[2]), cimag(El[2]));
    */
    
    /* zero aperture */
//     for(i = 0; i < ap->nx*ap->ny; i++) for(j = 0; j < 4; j++)
//       ap->aperture[j][i] = 0.0;
    ap->aperture->set(Complex(0.0));
    
    os = ap->oversamp;
    nx = ap->nx*os;
    ny = ap->ny*os;
    dx = ap->dx/os;
    dy = ap->dy/os;
    x0 = ap->x0 - ap->dx/2.0 + dx/2.0;
    y0 = ap->y0 - ap->dy/2.0 + dy/2.0;
    Rant = a->radius;
    Rhole = a->hole_radius;
    R2 = Rant*Rant;
    H2 = Rhole*Rhole;
    
    eps = dx/4.0;
    
    IPosition pos(4),shape;
    shape = ap->aperture->shape();

    for(j = 0; j < ny; j++)
      {
	for(i = 0; i < nx; i++)
	  {
	    /* determine parallactic angle rotated coordinates */
	    x = pac*(x0 + i*dx) - pas*(y0 + j*dy);
	    y = pas*(x0 + i*dx) + pac*(y0 + j*dy);
	    x = -x;
	    
	    if(fabs(x) > Rant) continue;
	    if(fabs(y) > Rant) continue;
	    r2 = x*x + y*y;
	    if(r2 > R2) continue;
	    if(r2 < H2) continue;
	    index = ((j/os)*nx + i)/os;

	    ray = rayx = rayy = 0;
	    
	    x1 = x;
	    y1 = y;
	    
	    for(iter = 0; iter < niter; iter++)
	      {
		ray = trace(a, x1, y1, p);
		if(!ray) goto nextpoint;
		x1 += (x - ray->aper[0]);
		y1 += (y - ray->aper[1]);
		deleteRay(ray);
		ray = 0;
	      }
	    
	    ray = trace(a, x1, y1, p);
	    
	    /* check for leg blockage */
	    if(legplanewaveblock2(a, ray))
	      goto nextpoint;
	    if(legsphericalwaveblock(a, ray))
	      goto nextpoint;
	    
	    if(y < 0) rayy = trace(a, x1, y1+eps, p);
	    else rayy = trace(a, x1, y1-eps, p);
	    
	    if(x < 0) rayx = trace(a, x1+eps, y1, p);
	    else rayx = trace(a, x1-eps, y1, p);
	    
	    if(ray == 0 || rayx == 0 || rayy == 0)
	      goto nextpoint;
	    
	    /* compute solid angle subtended at the feed */
	    dx1 = rayx->aper[0]-ray->aper[0];
	    dy1 = rayx->aper[1]-ray->aper[1];
	    dx2 = rayy->aper[0]-ray->aper[0];
	    dy2 = rayy->aper[1]-ray->aper[1];
	    
	    dA = 0.5*fabs(dx1*dy2 - dx2*dy1);
	    dO = (dOmega(a, rayx, rayy, ray, p)/dA)*dx*dx;
	    dP = dO*feedgain(a, ray, p);
	    amp = sqrt(dP);
	    
	    L = Raylen(ray);
	    phase = 2.0*M_PI*(L-L0)/a->lambda;
	    
	    /* phase retard the wave */
	    cp = cos(phase);
	    sp = sin(phase);
	    //	    fp = cp + sp*1.0i;

	    fp = Complex(cp,sp);
	    
	    
	    tracepol(Er, ray, E1);
	    Exr = fp*amp*E1[0];
	    Eyr = fp*amp*E1[1];
// 	    rr = Exr - 1.0i*Eyr;
// 	    rl = Exr + 1.0i*Eyr;
 	    rr = Exr - Iota*Eyr;
 	    rl = Exr + Iota*Eyr;
	    
	    tracepol(El, ray, E1);
	    Exl = fp*amp*E1[0];
	    Eyl = fp*amp*E1[1];
// 	    lr = Exl - 1.0i*Eyl;
// 	    ll = Exl + 1.0i*Eyl;
	    lr = Exl - Iota*Eyl;
	    ll = Exl + Iota*Eyl;
	    
// 	    pos(0)=(Int)((j/os) - (25.0/dy/os)/2 + shape(0)/2 - 0.5);
// 	    pos(1)=(Int)((i/os) - (25.0/dx/os)/2 + shape(1)/2 - 0.5);
	    // Following 3 lines go with ANT tag in VLACalc.....
	    //	    Double antRadius=BeamCalcGeometryDefaults[ap->band].Rant;
	    //	    pos(0)=(Int)((j/os) - (antRadius/dy/os) + shape(0)/2 - 0.5);
	    //	    pos(1)=(Int)((i/os) - (antRadius/dx/os) + shape(1)/2 - 0.5);
	    // Following 2 lines go with the PIX tag in VLACalc...
	    pos(0)=(Int)((j/os));
	    pos(1)=(Int)((i/os));
	    pos(3)=0;
	
	    pos(2)=0;tmp=ap->aperture->getAt(pos);ap->aperture->putAt(tmp+rr,pos);
	    pos(2)=1;tmp=ap->aperture->getAt(pos);ap->aperture->putAt(tmp+rl,pos);
	    pos(2)=2;tmp=ap->aperture->getAt(pos);ap->aperture->putAt(tmp+lr,pos);
	    pos(2)=3;tmp=ap->aperture->getAt(pos);ap->aperture->putAt(tmp+ll,pos);
	    /*
	    ap->aperture[0][index] += rr;//Exr;//
	    ap->aperture[1][index] += rl;//Eyr;//
	    ap->aperture[2][index] += lr;//Exl;//
	    ap->aperture[3][index] += ll;//Eyl;//
	    */
	  nextpoint:
	    if(ray)  deleteRay(ray);
	    if(rayx) deleteRay(rayx);
	    if(rayy) deleteRay(rayy);
	  }
      }
    
    
    deletePathology(p);
    deleteAntenna(a);
    
    return 1;
  }
};

