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
 * "@(#) $Id: ATMPercent.cpp,v 1.4 2009/09/10 16:22:17 dbroguie Exp $"
 *
 * who       when      what
 * --------  --------  ----------------------------------------------
 * pardo     24/03/09  created
 */

#include "ATMPercent.h"


namespace atm {
  
  // Constructors
  
  Percent::Percent() : valueIS_(0.0){}

  Percent::Percent(double percent) : valueIS_(percent){}

  Percent::Percent(double percent, const string &units){
    if(units == "%" || units == "percent" || units == "PERCENT"){
      valueIS_ = percent / 100.0;
    } else {
      // Exception: Unknown percent unit
      valueIS_ = percent;
    }
  }

  // Destructor
  Percent::~Percent(){}
  
  // Accessors
  double Percent::get()const{return valueIS_;}
  double Percent::get(const string &units)const{  
    if(units == "%" || units == "percent" || units == "PERCENT"){
      return valueIS_ * 100.0;
    } else {
      // Exception: Unknown percent unit
      return valueIS_;
    }
  }

} // namespace atm
