#ifndef _ATM_ANGLE_H
#define _ATM_ANGLE_H
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
 * "@(#) $Id: ATMAngle.h,v 1.4 2009/05/04 21:30:54 dbroguie Exp $"
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

/*! \brief Defines a Class for those parameters being angles.
 *
 *   Defines a Class for those physical parameters having units of angle. For
 *   example the Total Phase Delay can be expressed as an angle (but also as a length).
 */
class Angle
{
public:
  /** Default constructor */
  Angle();
  /** A full constructor: Angle value in default units (SI: radians) */
  Angle(double angle);
  /** A full constructor: Angle value + units. Implemented units are: deg [DEG], rad [RAD] [Rad]*/
  Angle(double angle, const string &units);

  /** Destructor */
  virtual ~Angle();

  /** Accessor to get the angle value in SI units (rad) */
  inline double get() const { return valueIS_; }
  /** Accessor to the angle value in specified units. Implemented units are: deg [DEG], rad [RAD] [Rad].
   *  If none of these implemented units is given, the SI value will be returned. */
  double get(const string &units) const;

  /** Operator "equal to a Angle" */
  inline Angle& operator=(const Angle &rhs) { if(&rhs != this) valueIS_ = rhs.valueIS_; return *this; }
  /** Operator "equal to a double converted to Angle in Hz" */
  inline Angle& operator=(double rhs) { valueIS_=rhs; return *this; }
  /** Operator "addition of angles" */
  inline Angle operator+(const Angle &rhs) { return Angle(valueIS_+rhs.get()); }
  /** Operator "substraction of angles" */
  inline Angle operator-(const Angle &rhs) { return Angle(valueIS_-rhs.get()); }
  /** Operator "multiplication of a angle by a double" */
  inline Angle operator*(double scf) { return Angle(valueIS_*scf); }
  /** Operator "multiplication of a angle by a float" */
  inline Angle operator*(float scf) { return Angle(valueIS_*(double)scf); }
 /** Operator "multiplication of a angle by an int" */
  inline Angle operator*(int scf) { return Angle(valueIS_*(double)scf); } // rhs scale factor
 /** Operator "multiplication of a angle by an unsigned int" */
  inline Angle operator*(unsigned int scf) { return Angle(valueIS_*(double)scf); } // rhs scale factor
  /** Operator "division of a angle by a double" */
  inline Angle operator/(double scf) { return Angle(valueIS_/scf); }
  /** Operator "division of a angle by a float" */
  inline Angle operator/(float scf) { return Angle(valueIS_/(double)scf); }
  /** Operator "division of a angle by an int" */
  inline Angle operator/(int scf) { return Angle(valueIS_/(double)scf); }
  /** Operator "division of a angle by an unsigned int" */
  inline Angle operator/(unsigned int scf) { return Angle(valueIS_/(double)scf); }
  /** Operator "comparator < for two angles" */
  inline bool operator<(const Angle & rhs) const  { return (valueIS_<rhs.get()); }
  /** Operator "comparator > for two angles" */
  inline bool operator>(const Angle & rhs) const  { return (valueIS_>rhs.get()); }
  /** Operator "comparator <= for two angles" */
  inline bool operator<=(const Angle & rhs) const  { return (valueIS_<=rhs.get()); }
  /** Operator "comparator >= for two angles" */
  inline bool operator>=(const Angle & rhs) const  { return (valueIS_>=rhs.get()); }
  /** Operator "comparator == for two angles" */
  inline bool operator==(const Angle & rhs) const  { return (valueIS_==rhs.get()); }
  /** Operator "comparator != for two angles" */
  inline bool operator!=(const Angle & rhs) const  { return (valueIS_!=rhs.get()); }

private:
  double valueIS_;
}; // class Angle

} // namespace atm

#endif /*!_ATM_ANGLE_H*/
