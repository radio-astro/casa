#ifndef SYNTHESIS_ANTENNA_H
#define SYNTHESIS_ANTENNA_H

namespace casa
{
#include <synthesis/MeasurementComponents/BeamCalc.h>
  
#define MAXGEOM 2000
  /*
   * calcAntenna parameters
   */
  typedef struct
  {
    Double sub_h;               /* height of subreflector (on axis) */
    Double feed[3];             /* position of the feed */
    Double feeddir[3];          /* unit vector pointing along feed */
    Double pfeeddir[3];         /* same as above after pathology applied */
    Double radius;              /* antenna radius (m) */
    Double K;
    Double deltar;
    Double zedge;               /* height at the edge of the dish */
    Double bestparabola;        /* best fit parabola quadratic coef */
    Double ftaper;              /* taper of feed */
    Double thmax;               /* maximum angle of feed */
    Double fa2pi;
    Double legwidth;
    Double legfoot, legfootz;
    Double legapex;
    Double legthick;
    Double hole_radius;
    Double freq, lambda;
    Double dir[3];
    Double hhat[3], vhat[3];   /* unit vectors orthogonal to dir */
    Double z[MAXGEOM];
    Double m[MAXGEOM];
    Double k[3];
    Int ngeom;
    char name[16];
    Int gridsize;
  } calcAntenna;
  
  typedef struct
  {
    Double subrot[3][3];        /* 3x3 matrix rotating x,y,z or nx,ny,nz */
    Double feedrot[3][3];       /* 3x3 matrix rotating x,y,z or nx,ny,nz */
    Double subshift[3];         /* 3 length vector */
    Double feedshift[3];        /* 3 length vector */
    Double subrotpoint[3];      /* 3 vector describing point to rotate sub. */
    Double az_offset;           /* azimuth pointing offset (radians) */
    Double el_offset;           /* elevation pointing offset (radians) */
    Double phase_offset;        /* DC offset in phase (radians) */
    Double focus;               /* meters out of focus toward subreflector */
  } Pathology;
  
  typedef struct
  {
    Double aper[6];             /* aperture x, y, z, nx, ny, nz */
    Double dish[6];             /* dish x, y, z, nx, ny, nz */
    Double sub[6];              /* subreflector x, y, z, nx, ny, nz */
    Double feed[3];             /* feed x, y, z */
  } Ray;
  
  calcAntenna *newAntenna(Double sub_h, Double feed_x, Double feed_y, Double feed_z,
		      Double ftaper, Double thmax, const char *geomfile);
  void deleteAntenna(calcAntenna *a);
  void Antennasetfreq(calcAntenna *a, Double freq);
  void Antennasetdir(calcAntenna *a, const Double *dir);
  void alignfeed(calcAntenna *a, const Pathology *p);
  void getfeedbasis(const calcAntenna *a, Double B[3][3]);
  void Efield(const calcAntenna *a, 
	      const casa::Complex *pol, 
	      casa::Complex *E);
  Int Antennasetfeedpattern(calcAntenna *a, const char *filename, Double scale);
  calcAntenna *newAntennafromApertureCalcParams(struct ApertureCalcParams *ap);
  void applyPathology(Pathology *P, calcAntenna *a);
  Int dishvalue(const calcAntenna *a, Double r, Double *z, Double *m);
  Int subfromdish(const calcAntenna *a, Double x, Double y, Double *subpoint);
  Int dishfromsub(const calcAntenna *a, Double x, Double y, Double *dishpoint);
  
  Ray *newRay(const Double *sub);
  void deleteRay(Ray *ray);
  Pathology *newPathology();
  Pathology *newPathologyfromApertureCalcParams(struct ApertureCalcParams *ap);
  void deletePathology(Pathology *P);
  Double dAdOmega(const calcAntenna *a, const Ray *ray1, const Ray *ray2, 
		  const Ray *ray3, const Pathology *p);
  Double dOmega(const calcAntenna *a, const Ray *ray1, const Ray *ray2, 
		const Ray *ray3, const Pathology *p);
  Double Raylen(const Ray *ray);
  Double feedfunc(const calcAntenna *a, Double theta);
  void Pathologize(Double *sub, const Pathology *p);
  void intersectdish(const calcAntenna *a, const Double *sub, const Double *unitdir, 
		     Double *dish, Int niter);
  void intersectaperture(const calcAntenna *a, const Double *dish, 
			 const Double *unitdir, Double *aper);
  Ray *trace(const calcAntenna *a, Double x, Double y, const Pathology *p);
  Double feedgain(const calcAntenna *a, const Ray *ray, const Pathology *p);
  
  void tracepol(casa::Complex *E0, const Ray *ray, casa::Complex *E1);
  
  Int legplanewaveblock(const calcAntenna *a, Double x, Double y);
  Int legplanewaveblock2(const calcAntenna *a, const Ray *ray);
  Int legsphericalwaveblock(const calcAntenna *a, const Ray *ray);
};
#endif
