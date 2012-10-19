//# AsdmStMan.cc: Storage Manager for the main table of a raw ASDM MS
//# Copyright (C) 2012
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
//# You should have receied a copy of the GNU Library General Public License
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
//# $Id: AsdmStMan.cc 21600 2012-07-16 09:59:20Z diepen $

#include <asdmstman/AsdmStMan.h>
#include <asdmstman/AsdmColumn.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/DataManError.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/BlockIO.h>
#include <casa/IO/AipsIO.h>
#include <casa/OS/CanonicalConversion.h>
#include <casa/OS/HostInfo.h>
#include <casa/OS/DOos.h>
#include <casa/Utilities/BinarySearch.h>
#include <casa/Utilities/Assert.h>


namespace casa {

AsdmStMan::AsdmStMan (const String& dataManName)
: DataManager    (),
  itsDataManName (dataManName),
  itsBDF         (0),
  itsOpenBDF     (-1)
{}

AsdmStMan::AsdmStMan (const String& dataManName,
                      const Record&)
: DataManager    (),
  itsDataManName (dataManName),
  itsBDF         (0),
  itsOpenBDF     (-1)
{}

AsdmStMan::AsdmStMan (const AsdmStMan& that)
: DataManager    (),
  itsDataManName (that.itsDataManName),
  itsBDF         (0),
  itsOpenBDF     (-1)
{}

AsdmStMan::~AsdmStMan()
{
  for (uInt i=0; i<ncolumn(); i++) {
    delete itsColumns[i];
  }
  closeBDF();
}

DataManager* AsdmStMan::clone() const
{
  return new AsdmStMan (*this);
}

String AsdmStMan::dataManagerType() const
{
  return "AsdmStMan";
}

String AsdmStMan::dataManagerName() const
{
  return itsDataManName;
}

Record AsdmStMan::dataManagerSpec() const
{
  return itsSpec;
}


DataManagerColumn* AsdmStMan::makeScalarColumn (const String& name,
                                                int,
                                                const String&)
{
  throw DataManError(name + " is unknown scalar column for AsdmStMan");
}

DataManagerColumn* AsdmStMan::makeDirArrColumn (const String& name,
                                                int dataType,
                                                const String& dataTypeId)
{
  return makeIndArrColumn (name, dataType, dataTypeId);
}

DataManagerColumn* AsdmStMan::makeIndArrColumn (const String& name,
                                                int dtype,
                                                const String&)
{
  AsdmColumn* col;
  if (name == "DATA") {
    col = new AsdmDataColumn(this, dtype);
  } else if (name == "FLAG") {
    col = new AsdmFlagColumn(this, dtype);
  } else if (name == "WEIGHT") {
    col = new AsdmWeightColumn(this, dtype);
  } else if (name == "SIGMA") {
    col = new AsdmSigmaColumn(this, dtype);
  } else {
    throw DataManError (name + " is unknown array column for AsdmStMan");
  }
  itsColumns.push_back (col);
  return col;
}

DataManager* AsdmStMan::makeObject (const String& group, const Record& spec)
{
  // This function is called when reading a table back.
  return new AsdmStMan (group, spec);
}

void AsdmStMan::registerClass()
{
  DataManager::registerCtor ("AsdmStMan", makeObject);
}

Bool AsdmStMan::canAddRow() const
{
  return True;
}
Bool AsdmStMan::canRemoveRow() const
{
  return False;
}
Bool AsdmStMan::canAddColumn() const
{
  return True;
}
Bool AsdmStMan::canRemoveColumn() const
{
  return True;
}

void AsdmStMan::addRow (uInt)
{}
void AsdmStMan::removeRow (uInt)
{
  throw DataManError ("AsdmStMan cannot remove rows");
}
void AsdmStMan::addColumn (DataManagerColumn*)
{}
void AsdmStMan::removeColumn (DataManagerColumn*)
{}

Bool AsdmStMan::flush (AipsIO&, Bool)
{
  return False;
}

void AsdmStMan::create (uInt)
{}

void AsdmStMan::open (uInt, AipsIO&)
{
  // Read the index file.
  init();
}

void AsdmStMan::prepare()
{
  for (uInt i=0; i<ncolumn(); i++) {
    itsColumns[i]->prepareCol();
  }
}

void AsdmStMan::resync (uInt)
{}

void AsdmStMan::reopenRW()
{}

void AsdmStMan::deleteManager()
{
  closeBDF();
  // Remove index file.
  //DOos::remove (fileName()+"index", False, False);
  DOos::remove (table().tableName() + "/table.asdmindex", False, False);
}

void AsdmStMan::closeBDF()
{
  if (itsOpenBDF >= 0) {
    delete itsBDF;
    itsBDF = 0;
    LargeFiledesIO::close (itsFD);
    itsOpenBDF = -1;
  }
}

void AsdmStMan::init()
{
  // Open index file and check version.
  //AipsIO aio(fileName() + "index");
  cout << "Opening " << table().tableName() + "/table.asdmindex" << endl;
  AipsIO aio(table().tableName() + "/table.asdmindex");
  itsVersion = aio.getstart ("AsdmStMan");
  if (itsVersion > 1) {
    throw DataManError ("AsdmStMan can only handle up to version 1");
  }
  // Read the index info.
  Bool asBigEndian;
  aio >> asBigEndian >> itsBDFNames;
  aio.get (itsIndex);
  aio.getend();
  itsDoSwap = (asBigEndian != HostInfo::bigEndian());
  // Fill the vector with rows from the index.
  itsIndexRows.resize (itsIndex.size());
  for (uInt i=0; i<itsIndex.size(); ++i) {
    itsIndexRows[i] = itsIndex[i].row;
  }
  // Fill the specification record (only used for reporting purposes).
  itsSpec.define ("version", itsVersion);
  itsSpec.define ("bigEndian", asBigEndian);
  itsSpec.define ("BDFs", Vector<String>(itsBDFNames));
  // Set to nothing read yet.
  itsStartRow   = -1;
  itsEndRow     = -1;
  itsIndexEntry = 0;
}

uInt AsdmStMan::searchIndex (Int64 rownr)
{
  // Search index entry matching this rownr.
  // If not exactly found, it is previous one.
  ///  vector<uInt>::const_iterator low = std::lower_bound (itsIndexRows.begin(),
  ///                                              itsIndexRows.end(),
  ///                                                  rownr);
///return low - itsIndexRows.begin();
  Bool found;
  uInt v = binarySearchBrackets (found, itsIndexRows, rownr,
                                 itsIndexRows.size());
  cout << "searchIndex: search returned v = " << v << endl;

  if (!found){
    if(v>0){
      v--;
    }
    else{
      throw DataManError ("AsdmStMan: index empty.");
    }
  }
  return v;
}

const AsdmIndex& AsdmStMan::findIndex (Int64 rownr)
{
  // Only get if not current.
  if (rownr < itsStartRow  ||  rownr >= itsEndRow) {
    itsIndexEntry = searchIndex (rownr);
    const AsdmIndex& ix = itsIndex[itsIndexEntry];
    // Resize to indicate not read yet.
    // Note that reserved size stays the same, so no extra mallocs needed.
    itsCrossData.resize (0);
    itsAutoData.resize  (0);
    itsStartRow = ix.row;
    itsEndRow   = ix.row + ix.nrow();
  }
  return itsIndex[itsIndexEntry];
}

void AsdmStMan::readCross (const AsdmIndex& ix, Complex* buf, uInt rownr)
{
  int dataTypeSize;
  switch (ix.dataType) {
  case 0:
    dataTypeSize = 2;    // Short
    break;
  case 1:
    dataTypeSize = 4;    // Int
    break;
  case 2:
    dataTypeSize = 4;    // Float
    break;
  default:
    throw DataManError ("AsdmStMan: unknown datatype " +
                        String::toString(ix.dataType));
  }
  // Read data block if not done yet.
  if (itsCrossData.empty()) {
    itsCrossData.resize (ix.crossDataSize() * dataTypeSize * 2);
    itsBDF->seek (ix.crossOffset);
    itsBDF->read (itsCrossData.size(), &(itsCrossData[0]));
  }
  // Determine the spw and baseline from the row.
  // The rows are stored in order of spw,baseline.
  uInt spw = (rownr - ix.row) / ix.nBl;
  uInt bl  = (rownr - ix.row) - spw*ix.nBl;
  switch (ix.dataType) {
  case 0:
    {
      // Get pointer to the data in the block.
      Short* data = (reinterpret_cast<Short*>(&(itsCrossData[0])) +
                     2 * (spw*ix.crossStepSpw + bl*ix.crossStepBl));
      if (itsDoSwap) {
        Short real,imag;
        for (uInt j=0; j<ix.crossNchan; ++j) {
          for (uInt i=0; i<ix.crossNpol; ++i) {
            CanonicalConversion::reverse2 (&real, data);
            CanonicalConversion::reverse2 (&imag, data+1);
            *buf++ = Complex(real/ix.scaleFactors[spw],
                             imag/ix.scaleFactors[spw]);
            data += 2;
          }
        }
      } else {
        for (uInt j=0; j<ix.crossNchan; ++j) {
          for (uInt i=0; i<ix.crossNpol; ++i) {
            *buf++ = Complex(data[0]/ix.scaleFactors[spw],
                             data[1]/ix.scaleFactors[spw]);
            data += 2;
          }
        }
      }
    }
    break;
  case 1:
    {
      // Get pointer to the data in the block.
      Int* data = (reinterpret_cast<Int*>(&(itsCrossData[0])) +
                   2 * (spw*ix.crossStepSpw + bl*ix.crossStepBl));
      if (itsDoSwap) {
        Int real,imag;
        for (uInt j=0; j<ix.crossNchan; ++j) {
          for (uInt i=0; i<ix.crossNpol; ++i) {
            CanonicalConversion::reverse4 (&real, data);
            CanonicalConversion::reverse4 (&imag, data+1);
            *buf++ = Complex(real/ix.scaleFactors[spw],
                             imag/ix.scaleFactors[spw]);
            data += 2;
          }
        }
      } else {
        for (uInt j=0; j<ix.crossNchan; ++j) {
          for (uInt i=0; i<ix.crossNpol; ++i) {
            *buf++ = Complex(data[0]/ix.scaleFactors[spw],
                             data[1]/ix.scaleFactors[spw]);
            data += 2;
          }
        }
      }
    }
    break;
  case 3:
    {
      // Get pointer to the data in the block.
      Float* data = (reinterpret_cast<Float*>(&(itsCrossData[0])) +
                     2 * (spw*ix.crossStepSpw + bl*ix.crossStepBl));
      if (itsDoSwap) {
        Float real,imag;
        for (uInt j=0; j<ix.crossNchan; ++j) {
          for (uInt i=0; i<ix.crossNpol; ++i) {
            CanonicalConversion::reverse4 (&real, data);
            CanonicalConversion::reverse4 (&imag, data+1);
            *buf++ = Complex(real/ix.scaleFactors[spw],
                             imag/ix.scaleFactors[spw]);
            data += 2;
          }
        }
      } else {
        for (uInt j=0; j<ix.crossNchan; ++j) {
          for (uInt i=0; i<ix.crossNpol; ++i) {
            *buf++ = Complex(data[0]/ix.scaleFactors[spw],
                             data[1]/ix.scaleFactors[spw]);
            data += 2;
          }
        }
      }
    }
    break;
  default:
    throw DataManError ("AsdmStMan: Unknown data type");
  }
}

void AsdmStMan::readAuto (const AsdmIndex& ix, Complex* buf, uInt rownr)
{
  if (itsAutoData.empty()) {
    itsAutoData.resize (ix.autoDataSize() * sizeof(float));
    itsBDF->seek (ix.autoOffset);
    itsBDF->read (itsAutoData.size(), &(itsAutoData[0]));
  }
  // Determine the spw and baseline from the row.
  // The rows are stored in order of spw,baseline; first cross, then auto.
  uInt spw = (rownr - ix.row - ix.nBl*ix.crossNspw) / ix.nAnt;
  uInt bl  = (rownr - ix.row - ix.nBl*ix.crossNspw) - spw*ix.nAnt;
  // Get pointer to the data in the block.
  Float* data = (reinterpret_cast<Float*>(&(itsAutoData[0])) +
                 spw*ix.autoStepSpw + bl*ix.autoStepBl);
  // The autocorr can have 1, 2, 3 or 4 npol.
  // 1 and 2 are XX and/or YY which are real numbers.
  // 3 are all 4 pols with XY a complex number and YX=conj(XY).
  // 4 are all 4 pols with XX,YY real and XY,YX complex.
  if (itsDoSwap) {
    Float valr, vali;
    if (ix.autoNpol == 3) {
      for (uInt i=0; i<ix.autoNchan; ++i) {
        CanonicalConversion::reverse4 (&valr, data++);
        *buf++ = Complex(valr);          // XX
        CanonicalConversion::reverse4 (&valr, data++);
        CanonicalConversion::reverse4 (&vali, data++);
        *buf++ = Complex(valr, vali);    // XY
        *buf++ = Complex(valr, -vali);   // YX
        CanonicalConversion::reverse4 (&valr, data++);
        *buf++ = Complex(valr);          // YY
      }
    } else if (ix.autoNpol == 4) {
      for (uInt i=0; i<ix.autoNchan; ++i) {
        CanonicalConversion::reverse4 (&valr, data++);
        *buf++ = Complex(valr);          // XX
        CanonicalConversion::reverse4 (&valr, data++);
        CanonicalConversion::reverse4 (&vali, data++);
        *buf++ = Complex(valr, vali);    // XY
        CanonicalConversion::reverse4 (&valr, data++);
        CanonicalConversion::reverse4 (&vali, data++);
        *buf++ = Complex(valr, vali);   // YX
        CanonicalConversion::reverse4 (&valr, data++);
        *buf++ = Complex(valr);          // YY
      }
    } else {
      for (uInt i=0; i<ix.autoNchan * ix.autoNpol; ++i) {
        CanonicalConversion::reverse4 (&valr, data++);
        *buf++ = Complex(valr);
      }
    }
  } else {
    // No byte swap needed.
    if (ix.autoNpol == 3) {
      for (uInt i=0; i<ix.autoNchan; ++i) {
        *buf++ = Complex(data[0]);
        *buf++ = Complex(data[1], data[2]);
        *buf++ = Complex(data[1], -data[2]);
        *buf++ = Complex(data[3]);
        data += 4;
      }
    } else if (ix.autoNpol == 4) {
      for (uInt i=0; i<ix.autoNchan; ++i) {
        *buf++ = Complex(data[0]);
        *buf++ = Complex(data[1], data[2]);
        *buf++ = Complex(data[3], data[4]);
        *buf++ = Complex(data[5]);
        data += 6;
      }
    } else {
      for (uInt i=0; i<ix.autoNchan * ix.autoNpol; ++i) {
        *buf++ = Complex(data[i]);
      }
    }
  }
}

IPosition AsdmStMan::getShape (uInt rownr)
{
  // Here determine the shape from the rownr.
  /// For now fill in some shape.
  uInt inx = searchIndex(rownr);
  cout << "inx " << inx << endl;
  const AsdmIndex& ix = itsIndex[inx];

  if (rownr < ix.row + ix.nBl * ix.crossNspw) {
    return IPosition(2, ix.crossNpol, ix.crossNchan);
  }
  // 3 autocorrs means 4 (YX = conj(XY));
  if (ix.autoNpol == 3) {
    return IPosition(2, 4, ix.autoNchan);
  }
  return IPosition(2, ix.autoNpol, ix.autoNchan);
}

void AsdmStMan::getData (uInt rownr, Complex* buf)
{
  /// Differentiate between auto and cross
  /// asdm2ms writes data in order of spw,bl,chan,pol.
  const AsdmIndex& ix = findIndex (rownr);
  // Open the BDF if needed.
  if (Int(ix.fileNr) != itsOpenBDF) {
    closeBDF();
    itsFD  = LargeFiledesIO::open (itsBDFNames[ix.fileNr].c_str(), False);
    itsBDF = new LargeFiledesIO (itsFD, itsBDFNames[ix.fileNr]);
    itsOpenBDF = ix.fileNr;
  }
  // See if we have a cross- or auto-correlation.
  if (rownr < ix.row + ix.nBl * ix.crossNspw) {
    readCross (ix, buf, rownr);
  } else {
    readAuto (ix, buf, rownr);
  }
}

} //# end namespace
