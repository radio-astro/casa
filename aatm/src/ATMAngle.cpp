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
 * "@(#) $Id: ATMAngle.cpp,v 1.3 2009/05/04 21:31:19 dbroguie Exp $"
 *
 * who       when      what
 * --------  --------  ----------------------------------------------
 * pardo     24/03/09  created
 */

#include "ATMAngle.h"

using namespace std;

namespace atm
{

Angle::Angle() :
  valueIS_(0.0)
{
}

Angle::Angle(double angle) :
  valueIS_(angle)
{
}

Angle::Angle(double angle, const string &units)
{
  if(units == "Rad" || units == "RAD" || units == "rad") {
    valueIS_ = angle;
  } else if(units == "deg" || units == "DEG") {
    valueIS_ = (angle / 360.0) * 6.2831852;
  } else {
    valueIS_ = angle;
  }
}

Angle::~Angle()
{
}

double Angle::get(const string &units) const
{
  if(units == "Rad" || units == "RAD" || units == "rad") {
    return valueIS_;
  } else if(units == "deg" || units == "DEG") {
    return 360.0 * (valueIS_ / 6.2831852);
  } else {
    return valueIS_;
  }
}

} // namespace atm

