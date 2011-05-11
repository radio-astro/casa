//#---------------------------------------------------------------------------
//# STFITSImageWriter.cc: Class to write out single dish spectra as FITS images
//#---------------------------------------------------------------------------
//# Copyright (C) 2008
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
//# $Id: $
//#---------------------------------------------------------------------------

#include <fitsio.h>
#include <images/Images/TempImage.h>

#include <lattices/Lattices/ArrayLattice.h>

#include <measures/Measures/MEpoch.h>
#include <measures/Measures/Stokes.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/TableRecord.h>
#include <casa/Containers/RecordField.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>


#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/ObsInfo.h>

#include <images/Images/ImageFITSConverter.h>
#include <images/Images/TempImage.h>

#include <lattices/Lattices/ArrayLattice.h>


//#include "STDefs.h"
#include "STHeader.h"
#include "Scantable.h"
#include "STFITSImageWriter.h"

using namespace casa;
using namespace asap;


STFITSImageWriter::STFITSImageWriter() : isClass_(False)
{;}

STFITSImageWriter::~STFITSImageWriter()
{;}


Bool STFITSImageWriter::write(const Scantable& stable, 
                              const String& fileName)
{

// Get global Header from Table

   STHeader hdr = stable.getHeader();

// Column keywords

   Table tab = stable.table();

// Temps

   const Unit RAD(String("rad"));

// Open and write header file

   String rootName(fileName);
   if (rootName.length()==0) rootName = String("fits");
   
   ObsInfo oi;
   oi.setTelescope(String(hdr.antennaname));
   oi.setObserver(String(hdr.observer));
   
   uInt maxMem = 128;
   Bool preferVelocity = False;
   Bool opticalVelocity = False;
   Int bitPix = -32;                     // FLoating point
   Float minPix = 1.0;
   Float maxPix = -1.0;
   Bool overWrite = True;
   Bool degLast = False;
   Bool reallyVerbose = False;
   String errMsg;


  Block<String> cols(5);
  cols[0] = String("SCANNO");
  cols[1] = String("CYCLENO");
  cols[2] = String("BEAMNO");
  cols[3] = String("IFNO");
  cols[4] = String("POLNO");
  TableIterator iter(tab, cols);
  // Open data file

  while ( !iter.pastEnd() ) {
    Table t = iter.table();
    ROTableRow row(t);
    const TableRecord& rec = row.get(0);
    String dirtype = stable.getDirectionRefString();
    ostringstream onstr;
    if (rootName.length()==0) {
      rootName = "fits";
    }
    if (tab.nrow() > 1) {
      if (stable.nscan() > 1) 
        onstr << "_SCAN" << rec.asuInt("SCANNO");
      if (stable.ncycle(rec.asuInt("SCANNO")) > 1) 
        onstr << "_CYCLE" << rec.asuInt("CYCLENO");
      if (stable.nbeam(rec.asuInt("SCANNO")) > 1) 
        onstr << "_BEAM" << rec.asuInt("BEAMNO");
      if (stable.nif(rec.asuInt("SCANNO")) > 1) 
        onstr << "_IF" << rec.asuInt("IFNO");
      if (stable.npol(rec.asuInt("SCANNO")) > 1) 
        onstr << "_POL" << rec.asuInt("POLNO");
    }
    String fileName = rootName + String(onstr) + String(".fits");
    int row0 = t.rowNumbers(tab)[0];

    const MPosition& mp = stable.getAntennaPosition();
    const MDirection& md = stable.getDirection(row0);
    const MEpoch& me = stable.getEpoch(row0);
    oi.setObsDate(me);
    
    //const Double& rf =  
    //  stable.molecules().getRestFrequency(rec.asuInt("MOLECULE_ID") );
    const std::vector<double>& rf = 
      stable.molecules().getRestFrequency(rec.asuInt("MOLECULE_ID") );
    SpectralCoordinate sC =
      stable.frequencies().getSpectralCoordinate(md, mp, me, rf, 
                                                 rec.asuInt("FREQ_ID"));

    Int polno = rec.asuInt("POLNO");
    Stokes::StokesTypes stokes = 
      Stokes::type(stable.getPolarizationLabel(polno, stable.getPolType()));
    Vector<Int> whichStokes(1);
    whichStokes(0) = Int(stokes);
    StokesCoordinate stC(whichStokes);


    CoordinateSystem cSys;
    Vector<Double> lonlat = md.getAngle().getValue();
    DirectionCoordinate dC = 
      getDirectionCoordinate(stable.getDirectionRefString(), 
                             lonlat[0], lonlat[1]); 
    cSys.addCoordinate(sC);
    cSys.addCoordinate(dC);
    cSys.addCoordinate(stC);
    cSys.setObsInfo(oi);
    
    Vector<Float> spec = rec.asArrayFloat("SPECTRA");
    Vector<uChar> flag = rec.asArrayuChar("FLAGTRA");
    Vector<Bool> bflag(flag.shape());
    convertArray(bflag, flag);

    // Create casacore Image
    IPosition shp(4, 1);
    shp(0)= spec.nelements();
    TempImage<Float> tIm(shp, cSys);
    tIm.put(spec);
    ArrayLattice<Bool> latMask(shp);
    IPosition where(4,0);
    IPosition stride(4,1);
    latMask.putSlice(!bflag, where, stride);
    tIm.attachMask(latMask);
    if (isClass_) {
      preferVelocity = True;
    }
    Bool ok = ImageFITSConverter::ImageToFITS(errMsg, tIm, fileName, 
                                              maxMem, preferVelocity,
                                              opticalVelocity, bitPix, 
                                              minPix, maxPix, overWrite,
                                              degLast, reallyVerbose);
    if (!ok) {
      throw(AipsError(errMsg));
    }
    
    int status = 0;
    fitsfile *fptr;     
    if( fits_open_file(&fptr, fileName.c_str(), READWRITE, &status) ) 
      throw AipsError("Coudn't open fits file for modification");

    if (isClass_) {
      // Add CLASS specific information
      // Apply hacks to fits file so it can be read by class
      // modifications are
      // CTYPE1 :  FREQ-XYZ -> FREQ
      // CRVAL1 :  -> CRVAL1-RESTFREQ
      
      // Use preferVelocity which seems to work.
      // When no restfrequency is given this will set up the correct
      // frequency axis. Otherwise it uses a velocity axis
      char ctyp[84];
      fits_read_key(fptr, TSTRING, "CTYPE1", ctyp, NULL, &status);
      if (String(ctyp).contains("FREQ", 0)) {
        float restf,refval;
        fits_read_key(fptr, TFLOAT, "CRVAL1", 
                      &refval, NULL, &status);
        restf = 0.0;
        if ( fits_update_key(fptr, TFLOAT, "CRVAL1", &restf,
                             NULL, &status) ) {
          throw AipsError("Couldn't modify CRVAL1");        
        }
        if ( fits_update_key(fptr, TFLOAT, "RESTFREQ", &refval,
                             NULL, &status) ) {
          throw AipsError("Couldn't modify RESTFREQ");        
        }
      } else {
        float refval = sC.referenceValue()[0];
        if ( fits_update_key(fptr, TFLOAT, "RESTFREQ", &refval,
                             NULL, &status) ) {
          throw AipsError("Couldn't modify RESTFREQ");        
        }
        
      }
    }

    Float tsys = stable.getTsys(row0);
    if ( fits_update_key(fptr, TFLOAT, "TSYS", &tsys,
                         "System temperature", &status) ) {
      throw AipsError("Couldn't modify TSYS");        
    }
    Float otime = Float(stable.getIntTime(row0));
    if ( fits_update_key(fptr, TFLOAT, "OBSTIME", &otime,
                         "Integration time", &status) ) {
      throw AipsError("Couldn't modify OBSTIME");        
    }
    
    const char* oname = stable.getSourceName(row0).c_str();
    if ( fits_update_key(fptr, TSTRING, "OBJECT", (char *)oname,
                         NULL, &status) ) {
      throw AipsError("Couldn't modify OBJECT");        
    }
    Float elev = stable.getElevation(row0)/C::pi*180.0;
    if ( fits_update_key(fptr, TFLOAT, "ELEVATIO", &elev,
                         "Telescope elevation (degrees)", &status) ) {
      throw AipsError("Couldn't modify ELEVATIO");        
    }
    Float azi = stable.getAzimuth(row0)/C::pi*180.0;
    if ( fits_update_key(fptr, TFLOAT, "AZIMUTH", &azi,
                         "Telescope azimuth (degrees)", &status) ) {
      throw AipsError("Couldn't modify AZIMUTH");        
    }
    fits_close_file(fptr, &status);
    ostringstream oss;
    oss << "Wrote " << fileName;
    pushLog(String(oss));  
    //pushLog(String(oss));
    ++iter;
  }
  return True;
}

DirectionCoordinate 
STFITSImageWriter::getDirectionCoordinate(const String& reff,
                                          Double lon, Double lat)
{
   
   Projection proj(Projection::SIN);                   // What should we use ?
   Matrix<Double> xForm(2,2);
   xForm = 0.0; xForm.diagonal() = 1.0;
   Vector<Double> incLonLat(2,0.0);
   Vector<Double> refPixLonLat(2,0.0);
   MDirection::Types mdt;
   if (!MDirection::getType(mdt, reff)) {
     throw(AipsError("Illegal Direction frame."));
   }
   return DirectionCoordinate(mdt, proj, lon, lat,
                              incLonLat[0], incLonLat[1], xForm, 
                              refPixLonLat[0], refPixLonLat[1]);
   
}

