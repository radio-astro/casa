#ifndef __BEAMCALC_CONSTANTS_H__
#define __BEAMCALC_CONSTANTS_H__

#include <math.h>

/* X_Y = multiplicitive constant required to convert a number in X units to
 * a number in Y units.
 *
 * 39.37 * INCH_METER = 1.0 
 */

#define METER_INCH	39.37008
#define INCH_METER	(1.0/METER_INCH)

#define NS_METER	0.299792458	/* Exact */
#define METER_NS	(1.0/NS_METER)

#define DEG_RAD		M_PI/180.0
#define RAD_DEG		180.0/M_PI

#endif
