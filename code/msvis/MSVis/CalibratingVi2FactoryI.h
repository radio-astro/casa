//# CalibratingVi2FactoryI.h: MSVis Interface definition of the CalibratingVi2Factory class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#ifndef CalibratingVi2FactoryI_H_
#define CalibratingVi2FactoryI_H_

// Where ViFactory interface is defined
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/IteratingParameters.h>


namespace casa { //# NAMESPACE CASA - BEGIN
namespace vi { //# NAMESPACE VI - BEGIN

class CalibratingVi2FactoryI : public vi::ViFactory
{

friend class LayeredVi2Factory;

public:

  typedef CalibratingVi2FactoryI* (*CalViFacGenerator)(MeasurementSet*,
						       const Record&,
						       const IteratingParameters&);
  static Bool setGenerator(CalViFacGenerator);

  CalibratingVi2FactoryI() {}
  virtual ~CalibratingVi2FactoryI() {}

  // Public function to generate myself via static generator_p
  static CalibratingVi2FactoryI* generate(MeasurementSet* ms,
					  const Record& calrec,
					  const IteratingParameters& iterpar);

protected:

  // Create CalibratingVi2 implementation
  //  this version makes plain one inside
  virtual vi::ViImplementation2 * createVi (vi::VisibilityIterator2 * vi) const = 0;
  //  If vii not NULL, then use it as the underlying one
  virtual vi::ViImplementation2 * createVi (vi::VisibilityIterator2 * vi,
					    vi::ViImplementation2 * vii) const = 0;

private:

  static CalViFacGenerator generator_p;

};


} //# NAMESPACE VI - END
} //# NAMESPACE CASA - END


#endif /* CalibratingVi2FactoryI_H_ */

