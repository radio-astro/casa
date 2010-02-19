#ifndef _ATM_MASSDENSITY_H
#define _ATM_MASSDENSITY_H
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
 * "@(#) $Id: ATMMassDensity.h,v 1.3 2009/05/04 21:30:54 dbroguie Exp $"
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
/*! \brief Mass Density value with units
 *
 *   This class defines physical parameters having units of Mass Density.
 *   For example, the profiles of minor gases, such as O3, are in mass density.
 *   Default is m**-3 (International System).
 */
class MassDensity
{
public:
  /** Default constructor */
  MassDensity();
  /** A full constructor: Mass Density value assumed by default to be in kgm**-3 (International System) */
  MassDensity(double massdensity);
  /** A full constructor: Mass Density value + unit.
   *  Valid units are kgm**-3 [kg m**-3, KGM**-3, KG M**-3], gcm**-3 [g cm**-3, GCM**-3, G CM**-3].
   *  If none of these implented units is given, the SI value will be returned. */
  MassDensity(double massdensity, const string &units);

  /** Destructor */
  virtual ~MassDensity();

  /** Accessor to the massdensity value in International System units (kgm**-3) */
  inline double get() const { return valueIS_; }
  /** Accessor to the massdensity value in specified units.
   *  Valid units are kgm**-3 [kg m**-3, KGM**-3, KG M**-3], gcm**-3 [g cm**-3, GCM**-3, G CM**-3]. */
  double get(const string &units) const;

  inline MassDensity& operator=(const MassDensity &rhs) { if(&rhs != this) valueIS_ = rhs.valueIS_; return *this; }
  inline MassDensity& operator=(double rhs) { valueIS_ = rhs; return *this; }
  inline MassDensity operator+(const MassDensity &rhs) { return MassDensity(valueIS_ + rhs.get()); }
  inline MassDensity operator-(const MassDensity &rhs) { return MassDensity(valueIS_ - rhs.get()); }
  inline MassDensity operator*(double scf) { return MassDensity(valueIS_ * scf); }
  inline MassDensity operator*(float scf) { return MassDensity(valueIS_ * (double) scf); }
  inline MassDensity operator*(int scf) { return MassDensity(valueIS_ * (double) scf); }
  inline MassDensity operator*(unsigned int scf) { return MassDensity(valueIS_ * (double) scf); }
  inline MassDensity operator/(double scf) { return MassDensity(valueIS_ / scf); }
  inline MassDensity operator/(float scf) { return MassDensity(valueIS_ / (double) scf); }
  inline MassDensity operator/(int scf) { return MassDensity(valueIS_ / (double) scf); }
  inline MassDensity operator/(unsigned int scf) { return MassDensity(valueIS_ / (double) scf); }
  inline bool operator<(const MassDensity & rhs) const { return (valueIS_ < rhs.get()); }
  inline bool operator>(const MassDensity & rhs) const { return (valueIS_ > rhs.get()); }
  inline bool operator<=(const MassDensity & rhs) const { return (valueIS_ <= rhs.get()); }
  inline bool operator>=(const MassDensity & rhs) const { return (valueIS_ >= rhs.get()); }
  inline bool operator==(const MassDensity & rhs) const { return (valueIS_ == rhs.get()); }
  inline bool operator!=(const MassDensity & rhs) const { return (valueIS_ != rhs.get()); }

private:
  double valueIS_;
}; // class MassDensity

} // namespace atm

#endif /*!_ATM_MASSDENSITY_H*/
