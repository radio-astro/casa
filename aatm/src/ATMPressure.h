#ifndef _ATM_PRESSURE_H
#define _ATM_PRESSURE_H
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
 * "@(#) $Id: ATMPressure.h,v 1.3 2009/05/04 21:30:54 dbroguie Exp $"
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
/*! \brief Defines pressure with units
 *
 *   This class defines physical parameters having units of Pressure.
 *   Default is Pa (International System) but the ATM library uses mb (hPa).
 */
class Pressure
{
public:

  /** Default constructor */
  Pressure();
  /** A full constructor: Pressure value in default SI units (Pa) */
  Pressure(double pressure);
  /** A full constructor: Pressure value + units. Valid units are hPa [HPA] [hpa], bar [BAR], mb [MB], mbar [MBAR], atm [ATM]. */
  Pressure(double pressure, const string &units);

  /** Destructor */
  virtual ~Pressure();

  /** Accessor to get the value in SI units (Pa) */
  double get() const { return valueIS_; }
  /** Accessor to get the value in the following (implemented) units: hPa [HPA] [hpa], bar [BAR], mb [MB], mbar [MBAR], atm [ATM].
   * If none of these implemented units is given, the SI value will be returned. */
  double get(const string &units) const;

  inline Pressure& operator=(const Pressure &rhs) { if(&rhs != this) valueIS_ = rhs.valueIS_; return *this; }
  inline Pressure& operator=(double rhs) { valueIS_ = rhs; return *this; }
  inline Pressure operator+(const Pressure &rhs) { return Pressure(valueIS_ + rhs.get()); }
  inline Pressure operator-(const Pressure &rhs) { return Pressure(valueIS_ - rhs.get()); }
  inline Pressure operator*(double scf) { return Pressure(valueIS_ * scf); }
  inline Pressure operator*(float scf) { return Pressure(valueIS_ * (double) scf); }
  inline Pressure operator*(int scf) { return Pressure(valueIS_ * (double) scf); }
  inline Pressure operator*(unsigned int scf) { return Pressure(valueIS_ * (double) scf); }
  inline Pressure operator/(double scf) { return Pressure(valueIS_ / scf); }
  inline Pressure operator/(float scf) { return Pressure(valueIS_ / (double) scf); }
  inline Pressure operator/(int scf) { return Pressure(valueIS_ / (double) scf); }
  inline Pressure operator/(unsigned int scf) { return Pressure(valueIS_ / (double) scf); }
  inline bool operator<(const Pressure &rhs) const { return (valueIS_ < rhs.get()); }
  inline bool operator>(const Pressure &rhs) const { return (valueIS_ > rhs.get()); }
  inline bool operator<=(const Pressure &rhs) const { return (valueIS_ <= rhs.get()); }
  inline bool operator>=(const Pressure &rhs) const { return (valueIS_ >= rhs.get()); }
  inline bool operator==(const Pressure &rhs) const { return (valueIS_ == rhs.get()); }
  inline bool operator!=(const Pressure &rhs) const { return (valueIS_ != rhs.get()); }

private:
  double valueIS_;
}; // class Pressure

} // namespace atm

#endif /*!_ATM_PRESSURE_H*/

