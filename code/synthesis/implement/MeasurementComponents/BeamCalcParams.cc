#include <synthesis/MeasurementComponents/BeamCalc.h>

namespace casa{
  struct BeamCalcGeometry BeamCalcGeometryDefaults[BeamCalc_NumBandCodes] = 
    {
      /* Format of each entry:
	 
      struct BeamCalcGeometry	all dimensions in meters, GHz 
      {
      char name[16];		name of antenna, e.g., VLA 
      Double sub_h;		subreflector vertex height above primary vertex 
      Double feedpos[3];		position of feed 
      Double subangle;		angle subtended by the subreflector 
      Double legwidth;		strut width 
      Double legfoot;		distance from optic axis of leg foot 
      Double legapex;		hight of leg intersection 
      Double Rhole;		radius of central hole 
      Double Rant;		antenna radius 
      Double reffreq;		a reference frequency 
      Double taperpoly[5];	polynomial expanded about reffreq 
      Int ntaperpoly;		number of terms in polynomial 
      */
      
      
      /* BeamCalc_VLA_L */
      {
	"VLA",
	8.47852,
	{0.68968367, 0.68968367, 1.67640},
	9.26,
	0.27, 7.55, 10.93876,
	2.0,
	12.5,
	1.5,
	{10.0, 0.0, 0.0, 0.0, 0.0}, 1
      },
      /* BeamCalc_VLA_C */
      {
	"VLA",
	8.47852,
	{-0.412205, -0.883976, 1.67640}, 
	9.26,
	0.27, 7.55, 10.93876,
	2.0,
	12.5,
	5.0,
	{12.8, 0.0, 0.0, 0.0, 0.0}, 1
      },
      /* BeamCalc_VLA_X */
      {
	"VLA",
	8.47852,
	{-0.6269493, 0.7471691, 1.67640},
	9.26,
	0.27, 7.55, 10.93876,
	2.0,
	12.5,
	8.0,
	{11.5, 0.0, 0.0, 0.0, 0.0}, 1
      },
      /* BeamCalc_VLA_U */
      {
	"VLA",
	8.47852,
	{0.4122049, -0.883976, 1.67640},
	9.26,
	0.27, 7.55, 10.93876,
	2.0,
	12.5,
	15.0,
	{11.5, 0.0, 0.0, 0.0, 0.0}, 1
      },
      /* BeamCalc_VLA_K */
      {
	"VLA",
	8.47852,
	{-0.937576, 0.268846, 1.67640},
	9.26,
	0.27, 7.55, 10.93876,
	2.0,
	12.5,
	22.0,
	{13.0, 0.0, 0.0, 0.0, 0.0}, 1
      },
      /* BeamCalc_VLA_Q */
      {
	"VLA",
	8.47852,
	{-0.9723533, -0.076526, 1.67640},
	9.26,
	0.27, 7.55, 10.93876,
	2.0,
	12.5,
	45.0,
	{13.0, 0.0, 0.0, 0.0, 0.0}, 1
      },
      
      /* BeamCalc_EVLA_L */
      {
	"VLA",
	8.47852,
	{-0.10026, 0.97019, 1.67640}, 
	9.26,
	0.27, 7.55, 10.93876,
	2.0,
	12.5,
	1.5,
	{10.0, 2.0, 0.0, 0.0, 0.0}, 2
      },
      /* BeamCalc_EVLA_S */
      {
	"VLA",
	8.47852,
	{0.196123, -0.955439, 1.67640},
	9.26,
	0.27, 7.55, 10.93876,
	2.0,
	12.5,
	3.0,
	{12.75, 0.75, 0.0, 0.0, 0.0}, 2
      },
      /* BeamCalc_EVLA_C */
      {
	"VLA",
	8.47852,
	{0.94300, -0.249152, 1.67640},
	9.26,
	0.27, 7.55, 10.93876,
	2.0,
	12.5,
	6.0,
	{12.75, 0.375, 0.0, 0.0, 0.0}, 2
      },
      /* BeamCalc_EVLA_X */
      {
	"VLA",
	8.47852,
	{0.893100, 0.392044, 1.67640}, 
	9.26,
	0.27, 7.55, 10.93876,
	2.0,
	12.5,
	10.0,
	{13.0, 0.0, 0.0, 0.0, 0.0}, 1
      },
      /* BeamCalc_EVLA_U */
      {
	"VLA",
	8.47852,
	{-0.657688, -0.720260, 1.67640},
	9.26,
	0.27, 7.55, 10.93876,
	2.0,
	12.5,
	15.0,
	{13.0, 0.0, 0.0, 0.0, 0.0}, 1
      },
      /* BeamCalc_EVLA_K */
      {
	"VLA",
	8.47852,
	{-0.877393, -0.426039, 1.67640},
	9.26,
	0.27, 7.55, 10.93876,
	2.0,
	12.5,
	22.0,
	{13.0, 0.0, 0.0, 0.0, 0.0}, 1
      },
      /* BeamCalc_EVLA_A */
      {
	"VLA",
	8.47852,
	{-0.933238, 0.283539, 1.67640}, 
	9.26,
	0.27, 7.55, 10.93876,
	2.0,
	12.5,
	33.0,
	{13.0, 0.0, 0.0, 0.0, 0.0}, 1
      },
      /* BeamCalc_EVLA_Q */
      {
	"VLA",
	8.47852,
	{-0.9723533, -0.0765259, 1.67640},
	9.26,
	0.27, 7.55, 10.93876,
	2.0,
	12.5,
	45.0,
	{13.0, 0.0, 0.0, 0.0, 0.0}, 1
      }
    };
};
