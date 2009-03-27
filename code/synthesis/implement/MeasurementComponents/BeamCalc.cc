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
#include <synthesis/MeasurementComponents/BeamCalcAntenna.h>
#include <images/Images/TempImage.h>
#include <casa/Exceptions.h>
#include <synthesis/MeasurementComponents/SynthesisError.h>
namespace casa{
  /* normalizes a "vector" of 3 Doubles in the vector sense */
  
  static inline void norm3(Double *v)
  {
    Double s;
    s = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    v[0] /= s;
    v[1] /= s;
    v[2] /= s;
  }
  
  calcAntenna *newAntenna(Double sub_h, Double feed_x, Double feed_y, Double feed_z,
		      Double ftaper, Double thmax, const char *geomfile)
  {
    calcAntenna *a;
    Int i;
    Double d, r, m, z;
    FILE *in;
    const char *sep=" ";
    char *aipsPath = strtok(getenv("CASAPATH"),sep);
    if (aipsPath == NULL)
      throw(SynthesisError("CASAPATH not found."));

    String fullFileName(aipsPath);
    fullFileName = fullFileName + "/data/nrao/VLA/" + geomfile;

    in = fopen(fullFileName.c_str(), "r");

    if(!in)
      {
	String msg = "File " + fullFileName 
	  + " not found.\n   Did you forget to install package data repository?\n";
	throw(SynthesisError(msg));
      }
    
    a = (calcAntenna *)malloc(sizeof(calcAntenna));
    
    for(i = 0; i < MAXGEOM; i++)
      {
	if(fscanf(in, "%lf%lf%lf", &r, &z, &m) != 3) break;
	a->z[i] = z;
	a->m[i] = m;
	a->radius = r;
      }
    fclose(in);
    a->ngeom = i;
    a->zedge = z;
    a->deltar = a->radius/(float)(a->ngeom-1.0);
    a->bestparabola = a->zedge/(a->radius*a->radius);
    if(i < 3)
      {
	fprintf(stderr, "geom file not valid\n");
	free(a);
	return 0;
      }
    
    z = sub_h-feed_z;
    
    a->sub_h = sub_h;
    a->feed[0] = feed_x;
    a->feed[1] = feed_y;
    a->feed[2] = feed_z;
    d = sqrt(feed_x*feed_x + feed_y*feed_y + z*z);
    if(z > 0.0)
      {
	a->K = sub_h + d;
	a->feeddir[0] = -feed_x/d;
	a->feeddir[1] = -feed_y/d;
	a->feeddir[2] = (sub_h-feed_z)/d;
      }
    else
      {
	a->K = sqrt(feed_x*feed_x + feed_y*feed_y + feed_z*feed_z);
	a->feeddir[0] = -feed_x/d;
	a->feeddir[1] = -feed_y/d;
	a->feeddir[2] = (sub_h-feed_z)/d;
      }
    for(i = 0; i < 3; i++) a->pfeeddir[i] = a->feeddir[i];
    a->ftaper = fabs(ftaper);
    a->thmax = thmax;
    a->fa2pi = 2.0*M_PI*sqrt(ftaper)*0.1874/sin(thmax*M_PI/180.0);
    a->legwidth = 0.0;
    a->legfoot = a->radius/2.0;
    a->legapex = sub_h*1.2;
    a->legthick = 0.0;
    a->hole_radius = 0.0; 
    a->dir[0] = a->dir[1] = 0.0;
    a->dir[2] = 1.0;
    strcpy(a->name, "unnamed");
    a->k[0] = a->k[1] = a->k[2] = 0.0;
    /* default to no polarization state */
    Antennasetfreq(a, 1.0);
    Antennasetdir(a, 0);  /* compute hhat and vhat */
    a->gridsize = 0;
    dishvalue(a, a->legfoot, &a->legfootz, 0);
    
    return a;
  }
  
  void deleteAntenna(calcAntenna *a)
  {
    if(!a) return;
    
    free(a);
  }
  
  void Antennasetfreq(calcAntenna *a, Double freq)
  {
    Int i;
    
    a->freq = freq;
    a->lambda = NS_METER/freq;
    for(i = 0; i < 3; i++) a->k[i] = -2.0*M_PI*a->dir[i]/a->lambda;
  }
  
  void Antennasetdir(calcAntenna *a, const Double *dir)
  {
    Double hmag;
    Int i;
    
    if(dir)
      {
	for(i = 0; i < 3; i++) a->dir[i] = dir[i];
	if(a->dir[0] == 0.0 && a->dir[1] == 0.0)
	  {
	    a->hhat[0] = 1.0;
	    a->hhat[1] = a->hhat[2] = 0.0;
	    a->vhat[1] = 1.0;
	    a->vhat[0] = a->vhat[2] = 0.0;
	  }
	else
	  {
	    a->hhat[0] = a->dir[1];
	    a->hhat[1] = -a->dir[0];
	    a->hhat[2] = 0.0;
	    hmag = sqrt(a->hhat[0]*a->hhat[0]
			+ a->hhat[1]*a->hhat[1]);
	    a->hhat[0] /= hmag;
	    a->hhat[1] /= hmag;
	    
	    a->vhat[0] = a->hhat[1]*a->dir[2] 
	      - a->hhat[2]*a->dir[1];
	    a->vhat[1] = a->hhat[2]*a->dir[0] 
	      - a->hhat[0]*a->dir[2];
	    a->vhat[2] = a->hhat[0]*a->dir[1] 
	      - a->hhat[1]*a->dir[0];
	  }
      }
    for(i = 0; i < 3; i++) a->k[i] = -2.0*M_PI*a->dir[i]/a->lambda;
  }
  
  /* sets feeddir after pathology is considered */
  void alignfeed(calcAntenna *a, const Pathology *p)
  {
    Int i, j;
    Double f[3], s0[3], s[3], d[3], m=0.0;
    
    for(i = 0; i < 3; i++) f[i] = a->feed[i] + p->feedshift[i];
    for(i = 0; i < 3; i++) s0[i] = -p->subrotpoint[i];
    s0[2] += a->sub_h;
    for(i = 0; i < 3; i++) 
      {
	s[i] = 0.0;
	for(j = 0; j < 3; j++) 
	  s[i] += p->subrot[i][j]*s0[j];
	s[i] += p->subrotpoint[i] + p->subshift[i];
	d[i] = s[i]-f[i];
	m += d[i]*d[i];
      }
    m = sqrt(m);
    for(i = 0; i < 3; i++) a->feeddir[i] = d[i]/m;
  }
  
  void getfeedbasis(const calcAntenna *a, Double B[3][3])
  {
    Int i;
    Double *dir, *vhat, *hhat;
    
    hhat = B[0];
    vhat = B[1];
    dir = B[2];
    
    for(i = 0; i < 3; i++) dir[i] = a->pfeeddir[i];
    
    if(dir[0] == 0.0 && dir[1] == 0.0)
      {
	vhat[0] = 1.0;
	vhat[1] = vhat[2] = 0.0;
	hhat[1] = 1.0;
	hhat[0] = hhat[2] = 0.0;
      }
    else
      {
	vhat[0] = dir[1];
	vhat[1] = -dir[0];
	vhat[2] = 0.0;
	norm3(vhat);
	
	hhat[0] = vhat[1]*dir[2] - vhat[2]*dir[1];
	hhat[1] = vhat[2]*dir[0] - vhat[0]*dir[2];
	hhat[2] = vhat[0]*dir[1] - vhat[1]*dir[0];
      }
  }
  
  void Efield(const calcAntenna *a, const Complex *pol, Complex *E)
  {
    Double B[3][3];
    Double *hhat, *vhat;
    
    getfeedbasis(a, B);
    hhat = B[0];
    vhat = B[1];

    for(Int i = 0; i < 3; i++)
      E[i] = Complex(hhat[i],0) * pol[0] + Complex(vhat[i],0) * pol[1];
  }
  
  Int Antennasetfeedpattern(calcAntenna *a, const char *filename, Double scale)
  {
#if 0
    Int i, N, Nmax;
    Double x, delta;
    VecArray pat;
    
    a->feedpatterndelta = 0.0;
    if(a->feedpattern) deleteVector(a->feedpattern);
    
    if(filename == 0) return 1;
    
    pat = VecArrayfromfile(filename, 2);
    
    if(!pat) return 0;
    N = VectorSize(pat[0]);
    g_assert(N > 2);
    g_assert(pat[0][0] == 0.0);
    
    delta = pat[0][1];
    g_assert(delta > 0.0);
    for(i = 2; i < N; i++) 
      {
	x = pat[0][i]-pat[0][i-1]-delta;
	g_assert(fabs(x) < delta/10000.0);
      }
    
    /* convert to radians */
    delta *= M_PI/180.0;
    
    /* and scale it */
    if(scale > 0.0) delta *= scale;
    
    /* Do we need to truncate the pattern? */
    Nmax = M_PI/delta;
    if(N > Nmax)
      {
	a->feedpattern = newVector(Nmax);
	for(i = 0; i < Nmax; i++) 
	  a->feedpattern[i] = fabs(pat[1][i]);
	deleteVector(pat[1]);
      }
    else a->feedpattern = pat[1];
    
    a->feedpatterndelta = delta;
    deleteVector(pat[0]);
    deleteVecArray(pat);
#endif
    return 1;
  }
  
  calcAntenna *newAntennafromApertureCalcParams(struct ApertureCalcParams *ap)
  {
    calcAntenna *a;
    Double dir[3] = {0.0, 0.0, 1.0};
    Double sub_h, feed_x, feed_y, feed_z, thmax, ftaper;
    char geomfile[128], *feedfile;
    struct BeamCalcGeometry *geom;
    Int i;
    Double x, freq, df;
    
    geom = &BeamCalcGeometryDefaults[ap->band];
    
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
    sprintf(geomfile, "%s.surface", geom->name);
    
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
  
  Int dishvalue(const calcAntenna *a, Double r, Double *z, Double *m)
  {
    Double ma, mb, mc, zav, A, B, C, D;
    Double x, d, dd;
    Double s = 1.0;
    Int n;
    
    if(r == 0)
      {
	*z = a->z[0];
	*m = 0.0;
	return 1;
      }
    
    if(r < 0) 
      {
	s = -1.0;
	r = -r;
      }
    d = a->deltar;
    dd = d*d;
    
    n = (Int)floor(r/d + 0.5);	/* the middle point */
    if(n > a->ngeom-2) n = a->ngeom-2;
    
    x = r - n*d;
    
    if(n == 0)
      {
	mc = a->m[1];
	ma = -mc;
	mb = 0.0;
	zav = 2.0*a->z[1] + a->z[0];
      }
    else
      {
	ma = a->m[n-1];
	mb = a->m[n];
	mc = a->m[n+1];
	zav = a->z[n-1] + a->z[n] + a->z[n+1];
      }
    
    A = mb;
    B = 0.5*(mc - ma)/d;
    C = 0.5*(mc - 2.0*mb + ma)/dd;
    
    D = (zav - B*dd)/3.0;
    
    if(m) *m = s*(A + B*x + C*x*x);
    if(z) *z = s*(D + A*x + B*x*x/2.0 + C*x*x*x/3.0);
    
    return 1;
  }
  
  /* Returns position of subreflector piece (x, y, z) and
   * its normal (u, v, w)
   */
  Int subfromdish(const calcAntenna *a, Double x, Double y, Double *subpoint)
  {
    Double r, z, m, u, v, w;
    Double dx, dy, dz, dl, t;
    Double n[3], sf[3], sd[3];
    Int i;
    
    r = sqrt(x*x + y*y);
    
    if(r == 0)
      {
	subpoint[0] = 0.0;
	subpoint[1] = 0.0;
	subpoint[2] = a->sub_h;
      }
    else
      {
	dishvalue(a, r, &z, &m);
	
	/* Compute reflected unit vector direction */
	m = tan(2.0*atan(m));
	w = 1.0/sqrt(1.0+m*m);
	u = -m*(x/r)*w;
	v = -m*(y/r)*w;
	
	dx = a->feed[0]-x;
	dy = a->feed[1]-y;
	dz = a->feed[2]-z;
	dl = a->K + z;
	
	t = 0.5*(dx*dx + dy*dy + dz*dz - dl*dl)
	  / (-dl + u*dx + v*dy + w*dz);
	
	subpoint[0] = x + u*t;
	subpoint[1] = y + v*t;
	subpoint[2] = z + w*t;
      }
    
    for(i = 0; i < 3; i++) sf[i] = a->feed[i] - subpoint[i];
    sd[0] = x - subpoint[0];
    sd[1] = y - subpoint[1];
    sd[2] = z - subpoint[2];
    
    norm3(sf);
    norm3(sd);
    
    for(i = 0; i < 3; i++) n[i] = sd[i]+sf[i];
    
    norm3(n);
    
    for(i = 0; i < 3; i++) subpoint[3+i] = n[i];
    
    return 1;
  }
  
  Int dishfromsub(const calcAntenna *a, Double x, Double y, Double *dishpoint)
  {
    Double x1, y1, dx, dy, mx, my, r, d;
    const Double eps = 0.001;
    Int iter, niter=500;
    Double sub[5][6];
    
    x1 = x;
    y1 = y;
    
    for(iter = 0; iter < niter; iter++)
      {
	subfromdish(a, x1, y1, sub[0]);
	subfromdish(a, x1-eps, y1, sub[1]);
	subfromdish(a, x1+eps, y1, sub[2]);
	subfromdish(a, x1, y1-eps, sub[3]);
	subfromdish(a, x1, y1+eps, sub[4]);
	mx = 0.5*(sub[2][0]-sub[1][0])/eps;
	my = 0.5*(sub[4][1]-sub[3][1])/eps;
	dx = (x-sub[0][0])/mx;
	dy = (y-sub[0][1])/my;
	if(fabs(dx) > a->radius/7.0) 
	  {
	    if(dx < 0) dx = -a->radius/7.0;
	    else dx = a->radius/7.0;
	  }
	if(fabs(dy) > a->radius/7.0) 
	  {
	    if(dy < 0) dy = -a->radius/7.0;
	    else dy = a->radius/7.0;
	  }
	r = sqrt(x1*x1 + y1*y1);
	if(r >= a->radius)
	  if(x1*dx + y1*dy > 0.0) return 0;
	x1 += 0.5*dx;
	y1 += 0.5*dy;
	if(fabs(dx) < 0.005*eps && fabs(dy) < 0.005*eps) break;
      }
    if(iter == niter) return 0;
    
    r = sqrt(x1*x1 + y1*y1);
    dishpoint[0] = x1;
    dishpoint[1] = y1;
    //	dishpoint[2] = polyvalue(a->shape, r);
    dishpoint[3] = sub[0][0];
    dishpoint[4] = sub[0][1];
    dishpoint[5] = sub[0][2];
    d = sqrt(1.0+mx*mx+my*my);
    dishpoint[6] = mx/d;
    dishpoint[7] = my/d;
    dishpoint[8] = 1.0/d;
    dishpoint[9] = sub[0][3];
    dishpoint[10] = sub[0][4];
    dishpoint[11] = sub[0][5];
    
    if(r > a->radius) return 0;
    else return 1;
  }
  
  void printAntenna(const calcAntenna *a)
  {
    printf("Antenna: %s  %p\n", a->name, a);
    printf("  freq    = %f GHz  lambda = %f m\n", a->freq, a->lambda);
    printf("  feeddir = %f, %f, %f\n", 
	   a->feeddir[0], a->feeddir[1], a->feeddir[2]); 
    printf("  pfeeddir = %f, %f, %f\n", 
	   a->pfeeddir[0], a->pfeeddir[1], a->pfeeddir[2]); 
  }
  
  Ray *newRay(const Double *sub)
  {
    Ray *ray;
    Int i;
    
    ray = (Ray *)malloc(sizeof(Ray));
    for(i = 0; i < 6; i++) ray->sub[i] = sub[i];
    
    return ray;
  }
  
  void deleteRay(Ray *ray)
  {
    if(ray) free(ray);
  }
  
  Pathology *newPathology()
  {
    Pathology *P;
    Int i, j;
    
    P = (Pathology *)malloc(sizeof(Pathology));
    
    for(i = 0; i < 3; i++) P->subrotpoint[i] = P->subshift[i] = P->feedshift[i] = 0.0;
    for(i = 0; i < 3; i++) for(j = 0; j < 3; j++) 
      P->feedrot[i][j] = P->subrot[i][j] = 0.0;
    for(i = 0; i < 3; i++) P->feedrot[i][i] = P->subrot[i][i] = 1.0;
    
    P->az_offset = 0.0;
    P->el_offset = 0.0;
    P->phase_offset = 0.0;
    P->focus = 0.0;
    
    return P;
  }
  
  Pathology *newPathologyfromApertureCalcParams(struct ApertureCalcParams *ap)
  {
    Pathology *P;
    
    P = newPathology();
    
    return P;
  }
  
  void deletePathology(Pathology *P)
  {
    if(P) free(P);
  }
  
  static void normvec(const Double *a, const Double *b, Double *c)
  {
    Int i;
    Double r;
    for(i = 0; i < 3; i++) c[i] = a[i] - b[i];
    r = sqrt(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]);
    for(i = 0; i < 3; i++) c[i] /= r;
  }
  
  Double dAdOmega(const calcAntenna *a, const Ray *ray1, const Ray *ray2,
		  const Ray *ray3, const Pathology *p)
  {
    Double A, Omega;
    Double n1[3], n2[3], n3[3], f[3], ci, cj, ck;
    Int i;
    
    /* Area in aperture is in a plane z = const */
    A = 0.5*fabs(
		 (ray1->aper[0]-ray2->aper[0])*(ray1->aper[1]-ray3->aper[1]) -
		 (ray1->aper[0]-ray3->aper[0])*(ray1->aper[1]-ray2->aper[1]) );
    
    for(i = 0; i < 3; i++) f[i] = a->feed[i] + p->feedshift[i];
    
    normvec(ray1->sub, f, n1);
    normvec(ray2->sub, f, n2);
    normvec(ray3->sub, f, n3);
    
    for(i = 0; i < 3; i++)
      {
	n1[i] -= n3[i];
	n2[i] -= n3[i];
      }
    
    ci = n1[1]*n2[2] - n1[2]*n2[1];
    cj = n1[2]*n2[0] - n1[0]*n2[2];
    ck = n1[0]*n2[1] - n1[1]*n2[0];
    
    Omega = 0.5*sqrt(ci*ci + cj*cj + ck*ck);
    
    return A/Omega;
  }
  
  Double dOmega(const calcAntenna *a, const Ray *ray1, const Ray *ray2,
		const Ray *ray3, const Pathology *p)
  {
    Double Omega;
    Double n1[3], n2[3], n3[3], f[3], ci, cj, ck;
    Int i;
    
    for(i = 0; i < 3; i++) f[i] = a->feed[i] + p->feedshift[i];
    
    normvec(ray1->sub, f, n1);
    normvec(ray2->sub, f, n2);
    normvec(ray3->sub, f, n3);
    
    for(i = 0; i < 3; i++)
      {
	n1[i] -= n3[i];
	n2[i] -= n3[i];
      }
    
    ci = n1[1]*n2[2] - n1[2]*n2[1];
    cj = n1[2]*n2[0] - n1[0]*n2[2];
    ck = n1[0]*n2[1] - n1[1]*n2[0];
    
    Omega = 0.5*sqrt(ci*ci + cj*cj + ck*ck);
    
    return Omega;
  }
  
  Double Raylen(const Ray *ray)
  {
    Double len, d[3];
    Int i;
    
    /* feed to subreflector */
    for(i = 0; i < 3; i++) d[i] = ray->feed[i] - ray->sub[i];
    len  = sqrt(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
    
    /* subreflector to dish */
    for(i = 0; i < 3; i++) d[i] = ray->sub[i] - ray->dish[i];
    len += sqrt(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
    
    /* dish to aperture */
    for(i = 0; i < 3; i++) d[i] = ray->dish[i] - ray->aper[i];
    len += sqrt(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
    
    return len;
  }
  
  void Pathologize(Double *sub, const Pathology *p)
  {
    Int i;
    Int j;
    Double tmp[6];
    
    for(i = 0; i < 3; i++) sub[i] -= p->subrotpoint[i];
    
    for(i = 0; i < 3; i++) 
      {
	tmp[i] = 0.0;
	tmp[i+3] = 0.0;
	for(j = 0; j < 3; j++) tmp[i] += p->subrot[i][j]*sub[j];
	for(j = 0; j < 3; j++) tmp[i+3] += p->subrot[i][j]*sub[j+3];
      }
    
    for(i = 0; i < 3; i++) 
      sub[i] = tmp[i] + p->subrotpoint[i] + p->subshift[i];
    for(i = 4; i < 6; i++) 
      sub[i] = tmp[i];
  }
  
  void applyPathology(Pathology *P, calcAntenna *a)
  {
    Double dx[3];
    Int i, j;
    
    if(P->focus != 0.0)
      {
	dx[0] = -a->feed[0];
	dx[1] = -a->feed[1];
	dx[2] = a->sub_h-a->feed[2];
	norm3(dx);
	for(i = 0; i < 3; i++) P->feedshift[i] += P->focus*dx[i];
	
	P->focus = 0.0;
      }
    
    for(i = 0; i < 3; i++) a->pfeeddir[i] = 0.0;
    for(j = 0; j < 3; j++) for(i = 0; i < 3; i++)
      a->pfeeddir[j] += P->feedrot[j][i]*a->feeddir[i];
  }
  
  
  void intersectdish(const calcAntenna *a, const Double *sub, const Double *unitdir,
		     Double *dish, Int niter)
  {
    Double A, B, C, t, m, r;
    Double x[3], n[3];
    Int i, iter;
    
    /* First intersect with ideal paraboloid */
    A = a->bestparabola*(unitdir[0]*unitdir[0] + unitdir[1]*unitdir[1]);
    B = 2.0*a->bestparabola*(unitdir[0]*sub[0] + unitdir[1]*sub[1])
      -unitdir[2];
    C = a->bestparabola*(sub[0]*sub[0] + sub[1]*sub[1]) - sub[2];
    t = 0.5*(sqrt(B*B-4.0*A*C) - B)/A; /* take greater root */
    
    for(iter = 0; ; iter++)
      {
	/* get position (x) and normal (n) on the real dish */
	for(i = 0; i < 2; i++) x[i] = sub[i] + t*unitdir[i];
	r = sqrt(x[0]*x[0] + x[1]*x[1]);
	dishvalue(a, r, &(x[2]), &m);
	n[2] = 1.0/sqrt(1.0+m*m);
	n[0] = -m*(x[0]/r)*n[2];
	n[1] = -m*(x[1]/r)*n[2];
	
	if(iter >= niter) break;
	
	A = B = 0;
	for(i = 0; i < 3; i++)
	  {
	    A += n[i]*(x[i]-sub[i]);	/* n dot (x-sub) */
	    B += n[i]*unitdir[i];		/* n dot unitdir */
	  }
	t = A/B;
      }
    
    for(i = 0; i < 3; i++)
      {
	dish[i] = x[i];
	dish[i+3] = n[i];
      }
  }
  
  void intersectaperture(const calcAntenna *a, const Double *dish, 
			 const Double *unitdir, Double *aper)
  {
    Double t;
    Int i;
    
    t = (a->zedge-dish[2])/unitdir[2];
    for(i = 0; i < 3; i++) aper[i] = dish[i] + t*unitdir[i];
    
    aper[3] = aper[4] = 0.0;
    aper[5] = 1.0;
  }
  
  /* gain in power */
  Double feedfunc(const calcAntenna *a, Double theta)
  {
    Double stheta;
    
    stheta = sin(theta);
    return exp(2.0*(-0.083)*a->fa2pi*a->fa2pi*stheta*stheta);
  }
  
  /* gain in power */
  Double feedgain(const calcAntenna *a, const Ray *ray, const Pathology *p)
  {
    Double costheta = 0.0;
    Double v[3];
    Int i;
    
    for(i = 0; i < 3; i++) v[i] = ray->sub[i] - ray->feed[i];
    norm3(v);
    
    for(i = 0; i < 3; i++) costheta += a->pfeeddir[i]*v[i];
    
    return exp(2.0*(-0.083)*a->fa2pi*a->fa2pi*(1.0-costheta*costheta));
  }
  
  Ray *trace(const calcAntenna *a, Double x, Double y, const Pathology *p)
  {
    Ray *ray;
    Double idealsub[12];
    Double fu[3], du[3], au[3], ndotf=0.0, ndotd=0.0;
    Int i;
    const Int niter = 7;
    
    subfromdish(a, x, y, idealsub);
    
    ray = newRay(idealsub);
    
    Pathologize(ray->sub, p);
    
    if(ray->sub[5] < -1.0 || ray->sub[5] > -0.0) 
      {
	deleteRay(ray);
	return 0;
      }
    
    for(i = 0; i < 3; i++) ray->feed[i] = a->feed[i] + p->feedshift[i];
    
    /* now determine unit vector pointing to dish */
    
    /* unit toward feed */
    for(i = 0; i < 3; i++) fu[i] = ray->feed[i] - ray->sub[i];
    norm3(fu);
    
    /* unit toward dish */
    for(i = 0; i < 3; i++) ndotf += ray->sub[i+3]*fu[i];
    for(i = 0; i < 3; i++) du[i] = 2.0*ray->sub[i+3]*ndotf - fu[i];
    
    /* dish point */
    intersectdish(a, ray->sub, du, ray->dish, niter);
    
    /* unit toward aperture */
    for(i = 0; i < 3; i++) ndotd += ray->dish[i+3]*du[i];
    for(i = 0; i < 3; i++) au[i] = du[i] - 2.0*ray->dish[i+3]*ndotd;
    
    intersectaperture(a, ray->dish, au, ray->aper);
    
    return ray;
  }
  
  void tracepol(Complex *E0, const Ray *ray, Complex *E1)
  {
    Complex fac;
    Double v1[3], v2[3], v3[3];
    Double r[3];
    Int i;
    
    for(i = 0; i < 3; i++)
      {
	v1[i] = ray->sub[i]  - ray->feed[i];
	v2[i] = ray->dish[i] - ray->sub[i];
	v3[i] = ray->aper[i] - ray->dish[i];
	E1[i] = E0[i];
      }
    norm3(v1); 
    norm3(v2);
    norm3(v3);
    
    for(i = 0; i < 3; i++) r[i] = v1[i] - v2[i];
    norm3(r); 
    fac = Complex(r[0],0)*E1[0] + Complex(r[1],0)*E1[1] + Complex(r[2],0)*E1[2];
    for(i = 0; i < 3; i++) E1[i] = Complex(r[i],0)*fac*2.0 - E1[i];
    
    for(i = 0; i < 3; i++) r[i] = v2[i] - v3[i];
    norm3(r); 
    fac = Complex(r[0],0)*E1[0] + Complex(r[1],0)*E1[1] + Complex(r[2],0)*E1[2];
    for(i = 0; i < 3; i++) E1[i] = Complex(r[i],0)*fac*2.0 - E1[i];
  }
  
  Int legplanewaveblock(const calcAntenna *a, Double x, Double y)
  {
    /* outside the leg foot area, the blockage is spherical wave */
    if(x*x + y*y > a->legfoot*a->legfoot) return 0;
    
    if(a->legwidth == 0.0) return 0;
    
    if(strcmp(a->name, "VLBA") == 0) 
      {
	const Double s=1.457937;
	const Double c=1.369094;
	if(fabs(s*x+c*y) < -a->legwidth) return 1;
	if(fabs(s*x-c*y) < -a->legwidth) return 1;
      }
    else if(a->legwidth < 0.0)  /* "x shaped" legs */
      {
	if(fabs(x-y)*M_SQRT2 < -a->legwidth) return 1;
	if(fabs(x+y)*M_SQRT2 < -a->legwidth) return 1;
      }
    else if(a->legwidth > 0.0) /* "+ shaped" legs */
      {
	if(fabs(x)*2.0 < a->legwidth) return 1;
	if(fabs(y)*2.0 < a->legwidth) return 1;
      }
    
    return 0;
  }
  
  Int legplanewaveblock2(const calcAntenna *a, const Ray *ray)
  {
    Int i, n;
    Double dr2;
    Double theta, phi;
    Double r0[3], dr[3], l0[3], l1[3], dl[3], D[3]; 
    Double D2, N[3], ll, rr;
    const Double thetaplus[4] = 
      {0, M_PI/2.0, M_PI, 3.0*M_PI/2.0};
    const Double thetacross[4] = 
      {0.25*M_PI, 0.75*M_PI, 1.25*M_PI, 1.75*M_PI};
    const Double thetavlba[4] =
      {0.816817, 2.3247756, 3.9584096, 5.466368};
    const Double *thetalut;
    
    if(a->legwidth == 0.0) return 0;
    
    if(strcmp(a->name, "VLBA") == 0) thetalut = thetavlba;
    else if(a->legwidth < 0.0) thetalut = thetacross;
    else thetalut = thetaplus;
    
    /* inside the leg feet is plane wave blockage */
    dr2 = ray->dish[0]*ray->dish[0] + ray->dish[1]*ray->dish[1];
    if(dr2 >= a->legfoot*a->legfoot) return 0;
    
    for(i = 0; i < 3; i++)
      {
	r0[i] = ray->dish[i];
	dr[i] = ray->aper[i] - r0[i];
      }
    rr = r0[0]*r0[0] + r0[1]*r0[1];
    
    l0[2] = a->legfootz;
    l1[0] = l1[1] = 0.0;
    l1[2] = a->legapex;
    phi = atan2(r0[1], r0[0]);
    
    for(n = 0; n < 4; n++)
      {
	theta = thetalut[n];
	l0[0] = a->legfoot*cos(theta);
	l0[1] = a->legfoot*sin(theta);
	ll = l0[0]*l0[0] + l0[1]*l0[1];
	if((l0[0]*r0[0] + l0[1]*r0[1])/sqrt(ll*rr) < 0.7) continue;
	for(i = 0; i < 3; i++) dl[i] = l1[i] - l0[i];
	for(i = 0; i < 3; i++) D[i] = r0[i] - l0[i];
	
	N[0] = dr[1]*dl[2] - dr[2]*dl[1];
	N[1] = dr[2]*dl[0] - dr[0]*dl[2];
	N[2] = dr[0]*dl[1] - dr[1]*dl[0];
	norm3(N);
	
	D2 = D[0]*N[0] + D[1]*N[1] + D[2]*N[2];
	
	if(fabs(D2) <= 0.5*fabs(a->legwidth)) return 1;
      }
    
    return 0;
  }
  
  Int legsphericalwaveblock(const calcAntenna *a, const Ray *ray)
  {
    Int i, n;
    Double dr2;
    Double theta, phi;
    Double r0[3], dr[3], l0[3], l1[3], dl[3], D[3]; 
    Double D2, N[3], ll, rr;
    const Double thetaplus[4] = 
      {0, M_PI/2.0, M_PI, 3.0*M_PI/2.0};
    const Double thetacross[4] = 
      {0.25*M_PI, 0.75*M_PI, 1.25*M_PI, 1.75*M_PI};
    const Double thetavlba[4] =
      {0.816817, 2.3247756, 3.9584096, 5.466368};
    const Double *thetalut;
    
    if(a->legwidth == 0.0) return 0;
    
    if(strcmp(a->name, "VLBA") == 0) thetalut = thetavlba;
    else if(a->legwidth < 0.0) thetalut = thetacross;
    else thetalut = thetaplus;
    
    /* inside the leg feet is plane wave blockage */
    dr2 = ray->dish[0]*ray->dish[0] + ray->dish[1]*ray->dish[1];
    if(dr2 < a->legfoot*a->legfoot) return 0;
    
    for(i = 0; i < 3; i++)
      {
	r0[i] = ray->dish[i];
	dr[i] = ray->sub[i] - r0[i];
      }
    rr = r0[0]*r0[0] + r0[1]*r0[1];
    
    l0[2] = a->legfootz;
    l1[0] = l1[1] = 0.0;
    l1[2] = a->legapex;
    phi = atan2(r0[1], r0[0]);
    
    for(n = 0; n < 4; n++)
      {
	theta = thetalut[n];
	l0[0] = a->legfoot*cos(theta);
	l0[1] = a->legfoot*sin(theta);
	ll = l0[0]*l0[0] + l0[1]*l0[1];
	if((l0[0]*r0[0] + l0[1]*r0[1])/sqrt(ll*rr) < 0.7) continue;
	for(i = 0; i < 3; i++) dl[i] = l1[i] - l0[i];
	for(i = 0; i < 3; i++) D[i] = r0[i] - l0[i];
	
	N[0] = dr[1]*dl[2] - dr[2]*dl[1];
	N[1] = dr[2]*dl[0] - dr[0]*dl[2];
	N[2] = dr[0]*dl[1] - dr[1]*dl[0];
	norm3(N);
	
	D2 = D[0]*N[0] + D[1]*N[1] + D[2]*N[2];
	
	if(fabs(D2) <= 0.5*fabs(a->legwidth)) return 1;
      }
    
    return 0;
  }
  
  
  /* The meat of the calculation */
  
  
  Int calculateAperture(struct ApertureCalcParams *ap)
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
    
    a = newAntennafromApertureCalcParams(ap);
    p = newPathologyfromApertureCalcParams(ap);
    
    /* compute central ray pathlength */
    ray = trace(a, 0.0, 0.00001, p);
    L0 = Raylen(ray);
    deleteRay(ray);
    
    pac = cos(ap->pa+M_PI/2);
    pas = sin(ap->pa+M_PI/2);

    /* compute polarization vectors in circular basis */
    Pr[0] = 1.0/M_SQRT2; Pr[1] =  Iota/M_SQRT2;
    Pl[0] = 1.0/M_SQRT2; Pl[1] = -Iota/M_SQRT2;
    
#if 0
    /* in linear basis */
    Pr[0] = 1.0; Pr[1] = 0.0;
    Pl[0] = 0.0; Pl[1] = 1.0;
#endif
    
    /* compensate for feed orientation */
    getfeedbasis(a, B); 
    phase = atan2(B[0][1], B[0][0]);
    cp = cos(phase);
    sp = sin(phase);
    
    q[0] = Pr[0];
    q[1] = Pr[1];
    Pr[0] =  Complex(cp,0)*q[0] + Complex(sp,0)*q[1];
    Pr[1] = -Complex(sp,0)*q[0] + Complex(cp,0)*q[1];
    q[0] = Pl[0];
    q[1] = Pl[1];
    Pl[0] =  Complex(cp,0)*q[0] + Complex(sp,0)*q[1];
    Pl[1] = -Complex(sp,0)*q[0] + Complex(cp,0)*q[1];
    
    
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

