#ifndef SYNTHESIS_ANTENNA_H
#define SYNTHESIS_ANTENNA_H

namespace casa
{
#include <synthesis/TransformMachines/BeamCalc.h>
  
#define MAXGEOM 2000
  /*
   * calcAntenna parameters
   */
  typedef struct
  {
    casacore::Double sub_h;               /* height of subreflector (on axis) */
    casacore::Double feed[3];             /* position of the feed */
    casacore::Double feeddir[3];          /* unit vector pointing along feed */
    casacore::Double pfeeddir[3];         /* same as above after pathology applied */
    casacore::Double radius;              /* antenna radius (m) */
    casacore::Double K;
    casacore::Double deltar;
    casacore::Double zedge;               /* height at the edge of the dish */
    casacore::Double bestparabola;        /* best fit parabola quadratic coef */
    casacore::Double ftaper;              /* taper of feed */
    casacore::Double thmax;               /* maximum angle of feed */
    casacore::Double fa2pi;
    casacore::Double legwidth;
    casacore::Double legfoot, legfootz;
    casacore::Double legapex;
    casacore::Double legthick;
    casacore::Double hole_radius;
    casacore::Double freq, lambda;
    casacore::Double dir[3];
    casacore::Double hhat[3], vhat[3];   /* unit vectors orthogonal to dir */
    casacore::Double z[MAXGEOM];
    casacore::Double m[MAXGEOM];
    casacore::Double k[3];
    casacore::Int ngeom;
    char name[16];
    casacore::Int gridsize;
  } calcAntenna;
  
  typedef struct
  {
    casacore::Double subrot[3][3];        /* 3x3 matrix rotating x,y,z or nx,ny,nz */
    casacore::Double feedrot[3][3];       /* 3x3 matrix rotating x,y,z or nx,ny,nz */
    casacore::Double subshift[3];         /* 3 length vector */
    casacore::Double feedshift[3];        /* 3 length vector */
    casacore::Double subrotpoint[3];      /* 3 vector describing point to rotate sub. */
    casacore::Double az_offset;           /* azimuth pointing offset (radians) */
    casacore::Double el_offset;           /* elevation pointing offset (radians) */
    casacore::Double phase_offset;        /* DC offset in phase (radians) */
    casacore::Double focus;               /* meters out of focus toward subreflector */
  } Pathology;
  
  typedef struct
  {
    casacore::Double aper[6];             /* aperture x, y, z, nx, ny, nz */
    casacore::Double dish[6];             /* dish x, y, z, nx, ny, nz */
    casacore::Double sub[6];              /* subreflector x, y, z, nx, ny, nz */
    casacore::Double feed[3];             /* feed x, y, z */
  } Ray;
  
  calcAntenna *newAntenna(casacore::Double sub_h, casacore::Double feed_x, casacore::Double feed_y, casacore::Double feed_z,
		      casacore::Double ftaper, casacore::Double thmax, const char *geomfile);
  void deleteAntenna(calcAntenna *a);
  void Antennasetfreq(calcAntenna *a, casacore::Double freq);
  void Antennasetdir(calcAntenna *a, const casacore::Double *dir);
  void alignfeed(calcAntenna *a, const Pathology *p);
  void getfeedbasis(const calcAntenna *a, casacore::Double B[3][3]);
  void Efield(const calcAntenna *a, 
	      const casa::Complex *pol, 
	      casa::Complex *E);
  casacore::Int Antennasetfeedpattern(calcAntenna *a, const char *filename, casacore::Double scale);
  calcAntenna *newAntennafromApertureCalcParams(struct ApertureCalcParams *ap);
  void applyPathology(Pathology *P, calcAntenna *a);
  casacore::Int dishvalue(const calcAntenna *a, casacore::Double r, casacore::Double *z, casacore::Double *m);
  casacore::Int subfromdish(const calcAntenna *a, casacore::Double x, casacore::Double y, casacore::Double *subpoint);
  casacore::Int dishfromsub(const calcAntenna *a, casacore::Double x, casacore::Double y, casacore::Double *dishpoint);
  
  Ray *newRay(const casacore::Double *sub);
  void deleteRay(Ray *ray);
  Pathology *newPathology();
  Pathology *newPathologyfromApertureCalcParams(struct ApertureCalcParams *ap);
  void deletePathology(Pathology *P);
  casacore::Double dAdOmega(const calcAntenna *a, const Ray *ray1, const Ray *ray2, 
		  const Ray *ray3, const Pathology *p);
  casacore::Double dOmega(const calcAntenna *a, const Ray *ray1, const Ray *ray2, 
		const Ray *ray3, const Pathology *p);
  casacore::Double Raylen(const Ray *ray);
  casacore::Double feedfunc(const calcAntenna *a, casacore::Double theta);
  void Pathologize(casacore::Double *sub, const Pathology *p);
  void intersectdish(const calcAntenna *a, const casacore::Double *sub, const casacore::Double *unitdir, 
		     casacore::Double *dish, casacore::Int niter);
  void intersectaperture(const calcAntenna *a, const casacore::Double *dish, 
			 const casacore::Double *unitdir, casacore::Double *aper);
  Ray *trace(const calcAntenna *a, casacore::Double x, casacore::Double y, const Pathology *p);
  casacore::Double feedgain(const calcAntenna *a, const Ray *ray, const Pathology *p);
  
  void tracepol(casa::Complex *E0, const Ray *ray, casa::Complex *E1);
  
  casacore::Int legplanewaveblock(const calcAntenna *a, casacore::Double x, casacore::Double y);
  casacore::Int legplanewaveblock2(const calcAntenna *a, const Ray *ray);
  casacore::Int legsphericalwaveblock(const calcAntenna *a, const Ray *ray);
};
#endif
