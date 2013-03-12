/*******************************************************************************
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
* "@(#) $Id: ATMInverseLength.cpp,v 1.7 2011/11/22 18:08:06 jroche Exp $"
*
* who       when      what
* --------  --------  ----------------------------------------------
* pardo     24/03/09  created
*/

#include <stdio.h>
#include "ATMInverseLength.h"

using namespace std;

ATM_NAMESPACE_BEGIN

InverseLength::InverseLength() :
  valueIS_(0.0)
{
}

InverseLength::InverseLength(double inverseLength) :
  valueIS_(inverseLength)
{
}

InverseLength::InverseLength(double inverseLength, const string &units)
{
  valueIS_ = sput(inverseLength, units);
}

InverseLength::InverseLength(const InverseLength &inverseLength) :
  valueIS_(inverseLength.valueIS_)
{
}

InverseLength::~InverseLength()
{
}

double InverseLength::sget(double value, const string &units)
{
  if(units == "km-1" || units == "KM-1") {
    return 1.0E+3 * value;
  } else if(units == "m-1" || units == "M-1") {
    return value;
  } else if(units == "mm-1" || units == "MM-1") {
    return 1.0E-3 * value;
  } else if(units == "micron-1" || units == "MICRON-1") {
    return 1.0E-6 * value;
  } else if(units == "nm-1" || units == "NM-1") {
    return 1.0E-9 * value;
  } else {
    return value;
  }
}

double InverseLength::sput(double value, const string &units)
{
  if(units == "km-1" || units == "KM-1") {
    return 1.0E-3 * value;
  } else if(units == "m-1" || units == "M-1") {
    return value;
  } else if(units == "mm-1" || units == "MM-1") {
    return 1.0E+3 * value;
  } else if(units == "micron-1" || units == "MICRON-1") {
    return 1.0E+6 * value;
  } else if(units == "nm-1" || units == "NM-1") {
    return 1.0E+9 * value;
  } else {
    return value;
  }
}

string InverseLength::get(const string &form, const string &units) const
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

ATM_NAMESPACE_END
