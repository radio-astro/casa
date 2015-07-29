//# CasaImageOpener.h: A class with static functions to open an image of any type
//# Copyright (C) 2005
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#

#ifndef IMAGEANALYSIS_CASAIMAGEOPENER_H
#define IMAGEANALYSIS_CASAIMAGEOPENER_H
#include <casacore/casa/aips.h>
#include <casacore/images/Images/ImageOpener.h>


namespace casa { //# NAMESPACE CASA - BEGIN

//# Forward Declarations
class LatticeBase;
class LatticeExprNode;

// <summary>
// Definition of image types and handlers
// </summary>
//
// <use visibility=local>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
// 
// <synopsis>
// Extension of imageopener to allow for virtual concat image hiding inside a directory
// </synopsis>
//



class CasaImageOpener : public ImageOpener
{
public:

  // Return the type of an image with the given name.  Will throw an
  // exception if file does not exist.
  static ImageTypes imageType (const String& fileName);
  static LatticeBase* openImage (const String& fileName,
				 const MaskSpecifier& spec=MaskSpecifier());
 

private:

};


} //# NAMESPACE CASA - END




#endif
