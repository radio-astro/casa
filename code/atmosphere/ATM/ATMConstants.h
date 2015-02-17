#ifndef _ATM_CONSTANTS_H
#define _ATM_CONSTANTS_H
/****************************************************************************
 * ALMA - Atacama Large Millimiter Array
 * (c) Instituto de Estructura de la Materia, 2009
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * "@(#) $Id: ATMConstants.h Exp $"
 *
 * who       when      what
 * --------  --------  ----------------------------------------------
 * jroche    09/09/09  created
 */

#ifndef __cplusplus
#error "This is a C++ include file and cannot be used from plain C"
#endif

namespace atm { namespace constants {

//static const double lightSpeed = 2.99792458e8; //!< light speed [m/s]

//static const double pi = 3.141592654;
static const double pi = 3.141592653589793;

/*
// AtmProfile
static const double avogad = 6.022045E+23;
static const double airmwt = 28.964;


// AtmRefractiveIndex
static const double abun_18o = 0.0020439;
static const double abun_17o = 0.0003750;
static const double o2_mixing_ratio = 0.2092;

static const double abun_18o = 0.0020439;
static const double abun_17o = 0.0003750;
static const double abun_D = 0.000298444;
static const double mmol_h2o = 18.005059688;

static const double abun_18o = 0.0020439;
static const double abun_17o = 0.0003750;

static const double abun_18o = 0.0020439;
static const double abun_17o = 0.0003750;

static const double mmol = 18.0;

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19; // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mu = 0.161; //Debyes
static const double mmol = 44.0;

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19; // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mu = 0.112; //Debyes
static const double mmol = 28.0;

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19; // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mu = 1.82332; //Debyes

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19; // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mu = 1.855; //Debyes

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19; // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mu = 1.855; //Debyes

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19; // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mua = 0.657; //Debyes
static const double mub = 1.732; //Debyes

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19; // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mu = 1.855; //Debyes

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19; // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mu = 0.0186; //Debyes (M1 Transitions)

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19;  // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mu = 0.0186;  //Debyes (M1 Transitions)

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19; // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mu = 0.0186; //Debyes (M1 Transitions)

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19; // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mu = 0.0186; //Debyes (M1 Transitions)

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19; // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mu = 0.53; //Debyes
static const double mmol = 48.0;

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19; // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mu = 0.53; //Debyes
static const double mmol = 50.0;

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19; // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mu = 0.53; //Debyes
static const double mmol = 49.0;

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19; // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mu = 0.53; //Debyes
static const double mmol = 50.0;

static const double pi = 3.141592654;
static const double fac2fixed = 4.1623755E-19; // (8*pi**3/(3*h*c))*(1e-18)**2 = 4.1623755E-19
static const double mu = 0.53; //Debyes
static const double mmol = 49.0;
*/
} } // namespace atm::constants

#endif /*!_ATM_CONSTANTS_H*/
