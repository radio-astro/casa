//#---------------------------------------------------------------------------
//# STHeader.cpp: A container class for single dish integrations
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id:
//#---------------------------------------------------------------------------

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/iomanip.h>
#include <casa/Exceptions.h>
#include <casa/Utilities/Assert.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Quanta/MVTime.h>



#include "STDefs.h"
#include "STHeader.h"

using namespace casa;
using namespace asap;



bool STHeader::conformant( const STHeader& other )
{
  bool conforms;
  conforms = (this->antennaname == other.antennaname
              && this->equinox == other.equinox
              && this->fluxunit == other.fluxunit
              );
  return conforms;
}

String STHeader::diff( const STHeader& other )
{
  ostringstream thediff;
  if ( this->equinox != other.equinox ) {
    thediff  << "Equinox: "  << this->equinox << " <-> " 
             << other.equinox << endl;
  }
  if ( this->obstype != other.obstype ) {
    thediff << "Obs. Type: " << this->obstype << " <-> "
            << other.obstype << endl;
  }
  if ( this->fluxunit != other.fluxunit ) {
    thediff << "Flux unit: " << this->fluxunit << " <-> "
            << other.fluxunit << endl;
  }
  return String(thediff);
}

void STHeader::print() const {
  MVTime mvt(this->utc);
  mvt.setFormat(MVTime::YMD);
  cout << "Observer: " << this->observer << endl
       << "Project: " << this->project << endl
       << "Obstype: " << this->obstype << endl
       << "Antenna: " << this->antennaname << endl
       << "Ant. Position: " << this->antennaposition << endl
       << "Equinox: " << this->equinox << endl
       << "Freq. ref.: " << this->freqref << endl
       << "Ref. frequency: " << this->reffreq << endl
       << "Bandwidth: "  << this->bandwidth << endl
       << "Time (utc): "
       << mvt
       << endl;
  //setprecision(10) << this->utc << endl;
}

// SDDataDesc

uInt SDDataDesc::addEntry(const String& source, uInt ID,
			  const MDirection& dir, uInt secID)
{

// See if already exists

  if (n_ > 0) {
    for (uInt i=0; i<n_; i++) {
      if (source==source_[i] && ID==ID_[i]) {
         return i;
      }
    }
  }

// Not found - add it

  n_ += 1;
  source_.resize(n_,True);
  ID_.resize(n_,True);
  secID_.resize(n_,True);
  secDir_.resize(n_,True,True);
//
  source_[n_-1] = source;
  ID_[n_-1] = ID;
  secID_[n_-1] = secID;
  secDir_[n_-1] = dir;
//
  return n_-1;
}

void SDDataDesc::summary() const
{
   if (n_>0) {
      cerr << "Source    ID" << endl;
      for (uInt i=0; i<n_; i++) {
         cout << setw(11) << source_(i) << ID_(i) << endl;
      }
   }
}

