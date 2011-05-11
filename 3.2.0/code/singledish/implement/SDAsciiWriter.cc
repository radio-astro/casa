//#---------------------------------------------------------------------------
//# SDAsciiWriter.cc: ASAP class to write out single dish spectra as FITS images
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
//# $Id$
//#---------------------------------------------------------------------------

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/VectorIter.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Utilities/Assert.h>

#include <casa/iostream.h>
#include <casa/fstream.h>
#include <casa/sstream.h>

#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>

#include <measures/Measures/MEpoch.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>

#include <singledish/SDDefs.h>
#include <singledish/SDContainer.h>
#include <singledish/SDMemTable.h>
#include <singledish/SDAsciiWriter.h>

using namespace casa;
using namespace asap;


SDAsciiWriter::SDAsciiWriter()
{;}

SDAsciiWriter::~SDAsciiWriter()
{;}


Bool SDAsciiWriter::write(const SDMemTable& sdTable, const String& fileName, Bool toStokes)
{

// Get global Header from Table

   SDHeader header = sdTable.getSDHeader();
   MEpoch::Ref timeRef(MEpoch::UTC);              // Should be in header   
   MDirection::Types dirRef(MDirection::J2000);   // Should be in header   

// Column keywords

   Table tab = sdTable.table();
   ROArrayColumn<Double> dir(tab, String("DIRECTION"));
   ROScalarColumn<Double> time(tab, "TIME");
   ROArrayColumn<uInt> freqid(tab, "FREQID");
   ROScalarColumn<String> src(tab, "SRCNAME");

// Temps

   Vector<Int> whichStokes(1,1);
   Array<Double> whichDir;
   Vector<Double> lonLat(2);
   IPosition posDir(2,0);
   const Unit RAD(String("rad"));

// Open and write header file

   String rootName(fileName);
   if (rootName.length()==0) rootName = String("ascii");
   {
      String fName = String(rootName) + String("_header.txt");
      pushLog("Writing header to "+fName);
      ofstream of(fName.chars(), ios::trunc);
      std::string summary = sdTable.summary(true);
      of << summary;
      of.close();
   }

// Open data file

   String fName = rootName + String(".txt");
   ofstream of(fName.chars(), ios::trunc);

// Write header

   of << "row beam IF pol source longitude latitude time nchan spectrum mask" 
      << endl;
   
// Loop over rows

   const uInt nRows = sdTable.nRow();
   for (uInt iRow=0; iRow<nRows; iRow++) {

// Get data

      const MaskedArray<Float>& dataIn(sdTable.rowAsMaskedArray(iRow,toStokes));
      const Array<Float>& values = dataIn.getArray();
      const Array<Bool>& mask = dataIn.getMask();

// Get abcissa

      std::vector<double> abcissa = sdTable.getAbcissa(Int(iRow));
      const uInt n = abcissa.size();

// Iterate through data in this row by spectra

      ReadOnlyVectorIterator<Float> itData(values, asap::ChanAxis);
      ReadOnlyVectorIterator<Bool> itMask(mask, asap::ChanAxis);
      while (!itData.pastEnd()) {
         const IPosition& pos = itData.pos();
         AlwaysAssert(itData.vector().nelements()==n,AipsError);

// FreqID

         Vector<uInt> iTmp;
         freqid.get(iRow, iTmp);

// Direction
 
         dir.get(iRow, whichDir);
         posDir(0) = pos(asap::BeamAxis);
         posDir(1) = 0;
         lonLat[0] = whichDir(posDir);
//
         posDir(0) = pos(asap::BeamAxis);
         posDir(1) = 1;
         lonLat[1] = whichDir(posDir);

// Write preamble

         of << iRow << "  " << pos(asap::BeamAxis) << " " <<  pos(asap::IFAxis) << " " << 
               pos(asap::PolAxis) << " " <<
               src(iRow) <<  " " << formatDirection(lonLat) << " " << 
               sdTable.getTime(iRow,True) << " " << n << " ";

// Write abcissa

         of.setf(ios::fixed, ios::floatfield);
         of.precision(4);
         for (uInt i=0; i<n; i++) {
            of << abcissa[i] << " ";
         }

// Write data

         const Vector<Float>& data = itData.vector();
         const Vector<Bool>& mask = itMask.vector();
         for (uInt i=0; i<n; i++) {
            of << data[i] << " ";
         }
// Write mask

         for (uInt i=0; i<n; i++) {
            of << mask[i] << " ";
         }
         of << endl;

// Next spectrum

         itData.next();
         itMask.next();
      }
   }

   of.close();
   ostringstream oss;
   oss << "Wrote " << nRows << " rows into file " << fileName;
   pushLog(String(oss));
   return True;
}



String SDAsciiWriter::formatDirection(const Vector<Double>& lonLat)
{ 
   MVAngle x1(lonLat(0));
   String s1 = x1.string(MVAngle::TIME, 12);

   MVAngle x2(lonLat(1));
   String s2 = x2.string(MVAngle::ANGLE, 12);

   String ss = s1 + String(" ") + s2;
   return ss;
}

