//# VLAIlluminationConvFunc.h: Definition for VLAIlluminationConvFunc
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

#ifndef SYNTHESIS_BEAMCALC_H
#define SYNTHESIS_BEAMCALC_H

//#include <casa/complex.h>
#include <images/Images/TempImage.h>
#include <casa/Exceptions.h>
#include <casa/Logging/LogIO.h>

namespace casa
{

#define MAXGEOM 2000

  typedef struct 	/* all dimensions in meters, GHz */
  {
    char name[16];	/* name of antenna, e.g., VLA */
    Double sub_h;	/* subreflector vertex height above primary vertex */
    Double feedpos[3];	/* position of feed */
    Double subangle;	/* angle subtended by the subreflector */
    Double legwidth;	/* strut width */
    Double legfoot;		/* distance from optic axis of leg foot */
    Double legapex;		/* hight of leg intersection */
    Double Rhole;		/* radius of central hole */
    Double Rant;		/* antenna radius */
    Double reffreq;		/* a reference frequency */
    Double taperpoly[5];	/* polynomial expanded about reffreq */
    Int ntaperpoly;		/* number of terms in polynomial */
    
    Double astigm_0;     /* astigmatism: coefficient of Zernike Polyn. Z6 a.k.a. 0-90 */
    Double astigm_45;    /* astigmatism: coefficient of Zernike Polyn. Z5 a.k.a. 45-135 */

    /* to be added later
       Double focus;
       Double dfeedpos[3];
       Double dsub_z;
    */
  } BeamCalcGeometry;
  
  typedef struct 
  {
    Int oversamp;			/* average this many points per cell */
    TempImage<Complex> *aperture;	/* Jones planes [Nx,Ny,NStokes,NFreq]*/
    Double x0, y0;			/* center of cell 0, 0, meters */
    Double dx, dy;			/* increment in meters */
    Int nx, ny;			/* calculation plane size in cells */
    /* last cell is at coordinates:
       X = x0 + (nx-1)*dx
       Y = y0 + (ny-1)*dy
    */
    Double pa;			/* Parallactic angle, radians */
    Double freq;			/* GHz */
    Int band;
  } ApertureCalcParams;

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
    Double astigm_0;     /* astigmatism: coefficient of Zernike Polyn. Z6 a.k.a. 0-90 */
    Double astigm_45;    /* astigmatism: coefficient of Zernike Polyn. Z5 a.k.a. 45-135 */
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


  enum VLABeamCalcBandCode{
    BeamCalc_VLA_L = 0,
    BeamCalc_VLA_C,
    BeamCalc_VLA_X,
    BeamCalc_VLA_U,
    BeamCalc_VLA_K,
    BeamCalc_VLA_Q,
    BeamCalc_VLA_4,
    
    VLABeamCalc_NumBandCodes	/* this line last */
  };

  enum EVLABeamCalcBandCode{
    BeamCalc_EVLA_L = 0,
    BeamCalc_EVLA_S,
    BeamCalc_EVLA_C,
    BeamCalc_EVLA_X,
    BeamCalc_EVLA_U,
    BeamCalc_EVLA_K,
    BeamCalc_EVLA_A,
    BeamCalc_EVLA_Q,
    BeamCalc_EVLA_4,
    
    EVLABeamCalc_NumBandCodes	/* this line last */
  };

  enum ALMABeamCalcBandCode{
    BeamCalc_ALMA_1 = 0,
    BeamCalc_ALMA_2,
    BeamCalc_ALMA_3,
    BeamCalc_ALMA_4,
    BeamCalc_ALMA_5,
    BeamCalc_ALMA_6,
    BeamCalc_ALMA_7,
    BeamCalc_ALMA_8,
    BeamCalc_ALMA_9,
    BeamCalc_ALMA_10,
    
    ALMABeamCalc_NumBandCodes	/* this line last */
  };

  extern Double VLABandMinFreqDefaults[VLABeamCalc_NumBandCodes];
  extern Double VLABandMaxFreqDefaults[VLABeamCalc_NumBandCodes];
  extern BeamCalcGeometry VLABeamCalcGeometryDefaults[VLABeamCalc_NumBandCodes];
  extern Double EVLABandMinFreqDefaults[EVLABeamCalc_NumBandCodes];
  extern Double EVLABandMaxFreqDefaults[EVLABeamCalc_NumBandCodes];
  extern BeamCalcGeometry EVLABeamCalcGeometryDefaults[EVLABeamCalc_NumBandCodes];
  extern Double ALMABandMinFreqDefaults[ALMABeamCalc_NumBandCodes];
  extern Double ALMABandMaxFreqDefaults[ALMABeamCalc_NumBandCodes];
  extern BeamCalcGeometry ALMABeamCalcGeometryDefaults[ALMABeamCalc_NumBandCodes];

  class BeamCalc
  {
  public:

    // This is a SINGLETON class
    static BeamCalc* Instance();

    void setBeamCalcGeometries(const String& obsName, // (the observatory name, e.g. "ALMA" or "ACA")
			       const String& antennaType = "STANDARD",
			       const MEpoch& obsTime = MEpoch(Quantity(50000., "d")),
			       const String& otherAntRayPath=""); // override the AntennaResponses Table in Observatories

    Int getBandID(const Double freq, // in Hz 
		  const String& obsname,
		  const String& antennaType="STANDARD",
		  const MEpoch& obsTime = MEpoch(Quantity(50000., "d")),
		  const String& otherAntRayPath=""); // override the AntennaResponses Table in Observatories 

    Int calculateAperture(ApertureCalcParams *ap);
    Int calculateAperture(ApertureCalcParams *ap, const Int& whichStokes);
    Int calculateApertureLinPol(ApertureCalcParams *ap, const Int& whichStokes);

  protected:
    BeamCalc();
    
  private:

  void computePixelValues(const ApertureCalcParams *ap, const calcAntenna *a, const Pathology *p,
			  const Double &L0, Complex *Er, Complex *El, 
			  const Int &i, const Int &j);
  void computePixelValues(const ApertureCalcParams *ap, 
			  const calcAntenna *a, const Pathology *p,
			  const Double &L0,
			  Complex *Er, Complex *El,
			  const Int &i, const Int &j,
			  const Int& whichStokes);
  void computePixelValuesLinPol(const ApertureCalcParams *ap, 
				const calcAntenna *a, const Pathology *p,
				const Double &L0,
				Complex *Ex, Complex *Ey,
				const Int &i, const Int &j,
				const Int& whichStokes);
    //normalizes a "vector" of 3 Doubles in the vector sense
    inline void norm3(Double *v)
    {
      Double s;
      s = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
      v[0] /= s;
      v[1] /= s;
      v[2] /= s;
    }

    calcAntenna *newAntenna(Double sub_h, Double feed_x, Double feed_y, Double feed_z,
			    Double ftaper, Double thmax, const char *geomfile);

    void deleteAntenna(calcAntenna *a);

    void Antennasetfreq(calcAntenna *a, Double freq);

    void Antennasetdir(calcAntenna *a, const Double *dir);

    // sets feeddir after pathology is considered 
    void alignfeed(calcAntenna *a, const Pathology *p);

    void getfeedbasis(const calcAntenna *a, Double B[3][3]);

    void Efield(const calcAntenna *a, const Complex *pol, Complex *E);

    Int Antennasetfeedpattern(calcAntenna *a, const char *filename, Double scale);

    calcAntenna *newAntennafromApertureCalcParams(ApertureCalcParams *ap);

    Int dishvalue(const calcAntenna *a, Double r, Double *z, Double *m);

    Int astigdishvalue(const calcAntenna *a, Double x, Double y, Double *z, Double *m); 

    // Returns position of subreflector piece (x, y, z) and
    // its normal (u, v, w)
    Int subfromdish(const calcAntenna *a, Double x, Double y, Double *subpoint);

    Int dishfromsub(const calcAntenna *a, Double x, Double y, Double *dishpoint);

    void printAntenna(const calcAntenna *a);

    Ray* newRay(const Double *sub);

    void deleteRay(Ray *ray);

    Pathology* newPathology();

    Pathology* newPathologyfromApertureCalcParams(ApertureCalcParams *ap);

    void deletePathology(Pathology *P);

    void normvec(const Double *a, const Double *b, Double *c);

    Double dAdOmega(const calcAntenna *a, const Ray *ray1, const Ray *ray2,
		    const Ray *ray3, const Pathology *p);

    Double dOmega(const calcAntenna *a, const Ray *ray1, const Ray *ray2,
		  const Ray *ray3, const Pathology *p);

    Double Raylen(const Ray *ray);

    void Pathologize(Double *sub, const Pathology *p);

    void applyPathology(Pathology *P, calcAntenna *a);

    void intersectdish(const calcAntenna *a, const Double *sub, const Double *unitdir,
		       Double *dish, Int niter);

    void intersectaperture(const calcAntenna *a, const Double *dish, 
			   const Double *unitdir, Double *aper);

    Double feedfunc(const calcAntenna *a, Double theta);

    Double feedgain(const calcAntenna *a, const Ray *ray, const Pathology *p);

    Ray* trace(const calcAntenna *a, Double x, Double y, const Pathology *p);

    void tracepol(Complex *E0, const Ray *ray, Complex *E1);

    Int legplanewaveblock(const calcAntenna *a, Double x, Double y);

    Int legplanewaveblock2(const calcAntenna *a, const Ray *ray);

    Int legsphericalwaveblock(const calcAntenna *a, const Ray *ray);

    void copyBeamCalcGeometry(BeamCalcGeometry* to, BeamCalcGeometry* from);

    static BeamCalc* instance_p;

    String obsName_p;
    String antType_p;
    MEpoch obsTime_p;
    uInt BeamCalc_NumBandCodes_p;
    Vector<BeamCalcGeometry> BeamCalcGeometries_p;
    Vector<Double> bandMinFreq_p; // in Hz
    Vector<Double> bandMaxFreq_p; // in Hz
    String antRespPath_p;

    static const Double METER_INCH;
    static const Double INCH_METER;
    static const Double NS_METER;
    static const Double METER_NS;
    static const Double DEG_RAD;
    static const Double RAD_DEG;

  };
  
};

#endif
