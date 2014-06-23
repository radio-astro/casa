//# AsdmColumn.h: A column in the ASDM Storage Manager
//# Copyright (C) 2012
//# Associated Universities, Inc. Washington DC, USA.
//# (c) European Southern Observatory, 2012
//# Copyright by ESO (in the framework of the ALMA collaboration)
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
//# $Id: AsdmColumn.h 19324 2011-11-21 07:29:55Z diepen $

#ifndef ASDM_ASDMCOLUMN_H
#define ASDM_ASDMCOLUMN_H


//# Includes
#include <asdmstman/AsdmStMan.h> 
#include <tables/Tables/StManColumn.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Containers/Block.h>
#include <casa/OS/Conversion.h>

namespace casa {

// <summary>
// A column in the ASDM Storage Manager.
// </summary>

// <use visibility=local>

// <reviewed reviewer="UNKNOWN" date="before2004/08/25" tests="tAsdmStMan.cc">
// </reviewed>

// <prerequisite>
//# Classes you should understand before using this one.
//   <li> <linkto class=AsdmStMan>AsdmStMan</linkto>
// </prerequisite>

// <synopsis>
// For each column a specific Column class exists.
// </synopsis>

class AsdmColumn : public StManColumn
{
public:
  explicit AsdmColumn (AsdmStMan* parent, int dtype)
    : StManColumn (dtype),
      itsParent   (parent)
  {}
  virtual ~AsdmColumn();
  // All columns are not writable.
  // However, we let AsdmColumn::isWritable() return True. If an actual write is done, 
  // an exception will be thrown. This ensures that the StMan will work with MSMainColumns.
  virtual Bool isWritable() const;
  // Set column shape of fixed shape columns; it does nothing.
  virtual void setShapeColumn (const IPosition& shape);
  // Prepare the column. By default it does nothing.
  virtual void prepareCol();
protected:
  AsdmStMan* itsParent;
};

// <summary>DATA column in the ASDM Storage Manager.</summary>
// <use visibility=local>
class AsdmDataColumn : public AsdmColumn
{
public:
  explicit AsdmDataColumn (AsdmStMan* parent, int dtype)
    : AsdmColumn(parent, dtype) {}
  virtual ~AsdmDataColumn();
  virtual IPosition shape (uInt rownr);
  virtual void getArrayComplexV (uInt rowNr, Array<Complex>* dataPtr);
};

// <summary>FLAG column in the ASDM Storage Manager.</summary>
// <use visibility=local>
class AsdmFlagColumn : public AsdmColumn
{
public:
  explicit AsdmFlagColumn (AsdmStMan* parent, int dtype)
    : AsdmColumn(parent, dtype) {}
  virtual ~AsdmFlagColumn();
  virtual casa::IPosition shape (casa::uInt rownr);
  virtual void getArrayBoolV (casa::uInt rowNr,
                              casa::Array<casa::Bool>* dataPtr);
};

// <summary>WEIGHT column in the ASDM Storage Manager.</summary>
// <use visibility=local>
class AsdmWeightColumn : public AsdmColumn
{
public:
  explicit AsdmWeightColumn (AsdmStMan* parent, int dtype)
    : AsdmColumn(parent, dtype) {}
  virtual ~AsdmWeightColumn();
  virtual casa::IPosition shape (casa::uInt rownr);
  virtual void getArrayfloatV (casa::uInt rowNr,
                               casa::Array<casa::Float>* dataPtr);
};

// <summary>SIGMA column in the ASDM Storage Manager.</summary>
// <use visibility=local>
class AsdmSigmaColumn : public AsdmColumn
{
public:
  explicit AsdmSigmaColumn (AsdmStMan* parent, int dtype)
    : AsdmColumn(parent, dtype) {}
  virtual ~AsdmSigmaColumn();
  virtual casa::IPosition shape (casa::uInt rownr);
  virtual void getArrayfloatV (casa::uInt rowNr,
                               casa::Array<casa::Float>* dataPtr);
};


} //# end namespace

#endif
