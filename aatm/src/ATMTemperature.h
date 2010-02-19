#ifndef _ATM_TEMPERATURE_H
#define _ATM_TEMPERATURE_H
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
 * "@(#) $Id: ATMTemperature.h,v 1.4 2009/09/10 16:21:29 dbroguie Exp $"
 *
 * who       when      what
 * --------  --------  ----------------------------------------------
 * pardo     24/03/09  created
 */

#ifndef __cplusplus
#error This is a C++ include file and cannot be used from plain C
#endif

#include <string>

using namespace std;

namespace atm
{

/*! \brief Temperature value with units
 *
 *   This class defines physical parameters having units of Temperature.
 *   For example, the Radiance can be converted into Brightness Temperature.
 *   Default is K (International System), also used by the ATM library.
 */
class Temperature
{
public:
  /** Default constructor */
  Temperature();
  /** A full constructor: Temperature value assumed by default to be in K (International System) */
  Temperature(double temperature);
  /** A full constructor: Temperature value + unit. Valid units are K [k], mK [mk], C [c], and F [f].
   *  If none of these implemented units is given, the SI value will be returned. */
  Temperature(double temperature, const string &units);

  /** Destructor */
  ~Temperature();

  //@{
  /** Accessor to the temperature value in International System units (K) */
  inline double get() const { return valueIS_; }
  /** Accessor to the temperature value in specified units. Valid units are K [k], mK [mk], C [c], and F [f] */
  double get(const string &units) const;
  //@}

  inline Temperature& operator=(const Temperature &rhs){ if(&rhs != this) valueIS_ = rhs.valueIS_; return *this; }
  inline Temperature& operator=(double rhs) { valueIS_ = rhs; return *this; }
  inline Temperature operator+(const Temperature &rhs) { return Temperature(valueIS_ + rhs.get()); }
  inline Temperature operator-(const Temperature &rhs) { return Temperature(valueIS_ - rhs.get()); }
  inline Temperature operator*(double scf) { return Temperature(valueIS_ * scf); }
  inline Temperature operator*(float scf) { return Temperature(valueIS_ * (double) scf); }
  inline Temperature operator*(int scf) { return Temperature(valueIS_ * (double) scf); }
  inline Temperature operator*(unsigned int scf) { return Temperature(valueIS_ * (double) scf); }
  inline Temperature operator/(double scf) { return Temperature(valueIS_ / scf); }
  inline Temperature operator/(float scf) { return Temperature(valueIS_ / (double) scf); }
  inline Temperature operator/(int scf) { return Temperature(valueIS_ / (double) scf); }
  inline Temperature operator/(unsigned int scf) { return Temperature(valueIS_ / (double) scf); }
  inline bool operator<(const Temperature &rhs) const { return (valueIS_ < rhs.get()); }
  inline bool operator>(const Temperature &rhs) const { return (valueIS_ > rhs.get()); }
  inline bool operator<=(const Temperature &rhs) const { return (valueIS_ <= rhs.get()); }
  inline bool operator>=(const Temperature &rhs) const { return (valueIS_ >= rhs.get()); }
  inline bool operator==(const Temperature &rhs) const { return (valueIS_ == rhs.get()); }
  inline bool operator!=(const Temperature &rhs) const { return (valueIS_ != rhs.get()); }

private:
  double valueIS_;
}; // class Temperature

} // namespace atm

#endif /*!_ATM_TEMPERATURE_H*/


