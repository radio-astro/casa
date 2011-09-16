//# VPManager functionality sits here; 
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
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
//#
//# $Id$

#ifndef SYNTHESIS_VPMANAGER_H
#define SYNTHESIS_VPMANAGER_H

namespace casa {
  //Forward declarations
  class Record;

  class VPManager
    {
      
    public: 
      // no need of enums here use the one from PBMathInterface
      //      enum Type{NONE, COMMONPB, AIRY, GAUSS, POLY, IPOLY, COSPOLY,
      //		NUMERIC, IMAGE, ZERNIKE, MAX=ZERNIKE};

      // Default constructor                                        
      
      VPManager();

      // Destructor
      virtual ~VPManager();
      
      Bool saveastable(const String& tablename);

      Bool summarizevps(const Bool verbose);


      Bool setcannedpb(const String& tel, 
		       const String& other, 
		       const Bool dopb,
		       const String& commonpb,
		       const Bool dosquint, 
		       const Quantity& paincrement, 
		       const Bool usesymmetricbeam,
		       Record& rec);

      Bool setpbairy(const String& telescope, const String& othertelescope, 
		     const Bool dopb, const Quantity& dishdiam, 
		     const Quantity& blockagediam, 
		     const Quantity& maxrad, 
		     const Quantity& reffreq, 
		     MDirection& squintdir, 
		     const Quantity& squintreffreq, const Bool dosquint, 
		     const Quantity& paincrement, 
		     const Bool usesymmetricbeam,
		     Record& rec);

      Bool setpbcospoly(const String& telescope, const String& othertelescope,
			const Bool dopb, const Vector<Double>& coeff,
			const Vector<Double>& scale,
			const Quantity& maxrad,
			const Quantity& reffreq,
			const String& isthispb,
			MDirection& squintdir,
			const Quantity& squintreffreq, const Bool dosquint,
			const Quantity& paincrement,
			const Bool usesymmetricbeam,
			Record& rec);

      Bool setpbgauss(const String& tel, const String& other, const Bool dopb,
		      const Quantity& halfwidth, const Quantity maxrad, 
		      const Quantity& reffreq, const String& isthispb, 
		      MDirection& squintdir, const Quantity& squintreffreq,
		      const Bool dosquint, const Quantity& paincrement, 
		      const Bool usesymmetricbeam, Record& rec);

      Bool setpbinvpoly(const String& telescope, const String& othertelescope,
			const Bool dopb, const Vector<Double>& coeff,
			const Quantity& maxrad,
			const Quantity& reffreq,
			const String& isthispb,
			MDirection& squintdir,
			const Quantity& squintreffreq, const Bool dosquint,
			const Quantity& paincrement,
			const Bool usesymmetricbeam,
			Record& rec);

      Bool setpbnumeric(const String& telescope, const String& othertelescope,
			const Bool dopb, const Vector<Double>& vect,
			const Quantity& maxrad,
			const Quantity& reffreq,
			const String& isthispb,
			MDirection& squintdir,
			const Quantity& squintreffreq, const Bool dosquint,
			const Quantity& paincrement,
			const Bool usesymmetricbeam,
			Record &rec);

      Bool setpbimage(const String& telescope, const String& othertelescope, 
		      const Bool dopb, const String& realimage, 
		      const String& imagimage, Record& rec);

      Bool setpbpoly(const String& telescope, const String& othertelescope,
		     const Bool dopb, const Vector<Double>& coeff,
		     const Quantity& maxrad,
		     const Quantity& reffreq,
		     const String& isthispb,
		     MDirection& squintdir,
		     const Quantity& squintreffreq, const Bool dosquint,
		     const Quantity& paincrement,
		     const Bool usesymmetricbeam,
		     Record &rec);

    private:
      Record vplist_p;


    };

} //# NAMESPACE CASA - END
#endif
