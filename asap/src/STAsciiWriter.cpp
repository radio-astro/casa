//#---------------------------------------------------------------------------
//# STAsciiWriter.cc: ASAP class to write out single dish spectra as FITS images
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
//# $Id: STAsciiWriter.cpp 1446 2008-11-12 06:04:01Z TakTsutsumi $
//#---------------------------------------------------------------------------

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/VectorIter.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Utilities/Assert.h>

#include <casa/fstream.h>
#include <casa/sstream.h>
#include <casa/iomanip.h>

#include <measures/Measures/MEpoch.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/TableRecord.h>
#include <casa/Containers/RecordField.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>

#include "STDefs.h"
#include "STHeader.h"
#include "Scantable.h"
#include "STAsciiWriter.h"

using namespace casa;
using namespace asap;


STAsciiWriter::STAsciiWriter()
{;}

STAsciiWriter::~STAsciiWriter()
{;}


Bool STAsciiWriter::write(const Scantable& stable, const String& fileName)
{

// Get global Header from Table

   STHeader hdr = stable.getHeader();

// Column keywords

   Table tab = stable.table();

// Temps

   const Unit RAD(String("rad"));

// Open and write header file

   String rootName(fileName);
   if (rootName.length()==0) rootName = String("ascii");

  Block<String> cols(4);
  cols[0] = String("SCANNO");
  cols[1] = String("CYCLENO");
  cols[2] = String("BEAMNO");
  cols[3] = String("IFNO");
  TableIterator iter(tab, cols);
  // Open data file
  while ( !iter.pastEnd() ) {
    Table t = iter.table();
    ROTableRow row(t);
    const TableRecord& rec = row.get(0);
    String dirtype = stable.getDirectionRefString();
    ostringstream onstr;
    onstr << "SCAN" << rec.asuInt("SCANNO")
    << "_CYCLE" << rec.asuInt("CYCLENO")
    << "_BEAM" << rec.asuInt("BEAMNO")
    << "_IF" << rec.asuInt("IFNO");
    String fName = rootName + String(onstr) + String(".txt");
    ofstream of(fName.chars(), ios::trunc);
    int row0 = t.rowNumbers(tab)[0];
    MDirection mdir = stable.getDirection(row0);
    of << setfill('#') << setw(70) << "" << setfill(' ') << endl;
    addLine(of, "Name", rec.asString("SRCNAME"));
    addLine(of, "Position", String(dirtype+ " "+formatDirection(mdir)));
    addLine(of, "Time", stable.getTime(row0,true));
    addLine(of, "Flux Unit", hdr.fluxunit);
    addLine(of, "Pol Type", stable.getPolType());
    addLine(of, "Abcissa", stable.getAbcissaLabel(row0));
    addLine(of, "Beam No", rec.asuInt("BEAMNO"));
    addLine(of, "IF No", rec.asuInt("IFNO"));
    String wcs = stable.frequencies().print(rec.asuInt("FREQ_ID"), True);
    addLine(of, "WCS", wcs);
    std::vector<double> restfreqs= stable.molecules().getRestFrequency(rec.asuInt("MOLECULE_ID"));
    int nf = restfreqs.size();
    //addLine(of, "Rest Freq.", 
    //        stable.molecules().getRestFrequency(rec.asuInt("MOLECULE_ID") ));
    addLine(of, "Rest Freq.", restfreqs[0]);
    for ( unsigned int i=1; i<nf; ++i) {
        addLine(of, " ", restfreqs[i]);
    } 
    of << setfill('#') << setw(70) << "" << setfill(' ') << endl;

    of << std::left << setw(16) << "x";
    for ( unsigned int i=0; i<t.nrow(); ++i ) {
      ostringstream os,os1;
      os << "y" << i;
      os1 << "yf" << i;
      of << setw(16) << String(os);
      of << setw(7) << String(os1);
    }
    of << endl;
    std::vector<double> abc = stable.getAbcissa(row0);
    ROArrayColumn<Float> specCol(t,"SPECTRA");
    ROArrayColumn<uChar> flagCol(t,"FLAGTRA");
    Matrix<Float> specs = specCol.getColumn();
    Matrix<uChar> flags = flagCol.getColumn();
    for ( unsigned int i=0; i<specs.nrow(); ++i ) {
      of << setw(16) << setprecision(8) << abc[i] ;
      for ( unsigned int j=0; j<specs.ncolumn(); ++j ) {
        of << setw(16) << setprecision(8) << specs(i,j) ;
        of << setw(7) << Int(flags(i,j));
      }
      of << endl;
    }
    of.close();
    ostringstream oss;
    oss << "Wrote " << fName;
    pushLog(String(oss));
    ++iter;
  }
  return True;
}


String STAsciiWriter::formatDirection(const MDirection& md) const
{
  Vector<Double> t = md.getAngle(Unit(String("rad"))).getValue();
  Int prec = 7;

  MVAngle mvLon(t[0]);
  String sLon = mvLon.string(MVAngle::TIME,prec);
  uInt tp = md.getRef().getType();
  if (tp == MDirection::GALACTIC ||
      tp == MDirection::SUPERGAL ) {
    sLon = mvLon(0.0).string(MVAngle::ANGLE_CLEAN,prec);
  }
  MVAngle mvLat(t[1]);
  String sLat = mvLat.string(MVAngle::ANGLE+MVAngle::DIG2,prec);
  return sLon + String(" ") + sLat;
}

template <class T>
void STAsciiWriter::addLine(ostream& of, const String& lbl, const T& value)
{
  String label = lbl+String(": ");
  of << std::right << "# " << setw(15) << label << std::left
     << setw(52) << value << setw(0) << "#"<< endl;
}
