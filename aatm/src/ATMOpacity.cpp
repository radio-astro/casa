/*******************************************************************************
 * ALMA - Atacama Large Millimiter Array
 * (c) Institut de Radioastronomie Millimetrique, 2009
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
 * "@(#) $Id: ATMOpacity.cpp,v 1.5 2010/01/12 17:03:12 dbroguie Exp $"
 *
 * who       when      what
 * --------  --------  ----------------------------------------------
 * pardo     24/03/09  created
 */

#include <stdio.h>

#include "ATMOpacity.h"

using namespace std;

namespace atm
{

Opacity::Opacity() :
  valueIS_(0.0)
{
}

Opacity::Opacity(double opacity) :
  valueIS_(opacity)
{
}

Opacity::Opacity(double opacity, const string &units)
{
  valueIS_ = sput(opacity, units);
}

Opacity::Opacity(const Opacity &opacity) :
  valueIS_(opacity.valueIS_)
{
}

Opacity::~Opacity()
{
}

double Opacity::sget(double value, const string &units)
{
  if(units == "db" || units == "DB") {
    return value * 4.34294482;
  } else if(units == "np" || units == "NP" || units == "neper" || units == "NEPER"){
    return value;
  } else {
    // Exception: Unknown unit, neper (np) used by default)
    return value;
  }
}

double Opacity::sput(double value, const string &units)
{
  if(units == "db" || units == "DB") {
    return value / 4.34294482;
  } else if(units == "np" || units == "NP" || units == "neper" || units == "NEPER") {
    return value;
  } else {
    // Exception: Unknown unit, neper (np) used by default)
    return value;
  }
}

string Opacity::get(const string &form, const string &units) const
{
  char myString[18];
  //   if(form.length()==0)
  sprintf(myString, "%f %s", get(units), units.c_str());
  //     else
  //       sprintf( myString,"%form %s",
  // 	       get(units),
  // 	       units);

  return string(myString);
}

} // namespace atm

