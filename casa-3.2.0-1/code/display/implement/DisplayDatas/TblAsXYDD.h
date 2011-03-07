//# TblAsXYDD.h: Display Data for xy displays of data from a table
//# Copyright (C) 2000,2001,2002
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
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef TRIALDISPLAY_TBLASXYDD_H
#define TRIALDISPLAY_TBLASXYDD_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h> 
#include <display/Display/DParameterRange.h>
#include <display/Display/DParameterChoice.h>
#include <display/Display/DParameterString.h>   
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <display/DisplayDatas/ActiveCaching2dDD.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class Table;
class TblAsXYDM;
class Regex;

//# Forward Declarations

// <summary>
//Class for displaying data within a table as a xy image.
// </summary>

// <use visibility=local>   or   <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> ActiveCaching2dDD
//   <li> CachingDisplayData
//   <li> Table
// </prerequisite>
//
// <etymology>
// "TblAsXYDD" is a implementation of a <linkto class=ActiveCaching2dDD>
// ActiveCaching2dDD </linkto> which provides for the display of data held 
// within a table to be displayed as a xy image in an environment where
// individual depictions of the data are automatically cached.
// </etymology>
//
// <synopsis>
// This class adds to the interface defined in <linkto
// class=DisplayData>DisplayData </linkto>.  It adds the capability to 
// display data from a <linkto class=Table>Table</linkto>
// column as a xy image.  It is assumed that the Y axis is defined 
// to be either the row number of the table column being displayed or
// the scalar value from the same row number in a different table 
// column (e.g. plotting intensity as a function of frequency against
// row number or time determined from a different column of the table).
// The X axis is assumed to be the data in a scalar table column.
// </synopsis>
//
// <example>
// A TblAsXYDD object could be construed and used as follows:
// <srcblock>
//    TblAsXYDD *tardd1 = 0;
//    tardd1 = new TblAsXYDD("tablename");
//    Colormap cmap1("Hot Metal 2");
//    tardd1->setColormap(&cmap1, 1.0);
//    wcHolder->addDisplayData((DisplayData *)tardd1);
// </srcblock>
// </example>
//
// <motivation>
// To allow the display of data from a table as a xy image.
// </motivation>
//
// <templating arg=T>
// </templating>
//
// <thrown>
// </thrown>
//
// <todo asof="2000/10/30">
//   <li> make sure complex data are handled correctly
//   <li> make sure table column units can be determined properly
//   <li> make sure a scalar table column can be used for y axis
//   <li> extend to n-dimensional arrays in table column
//   <li> when constructed with *table we need to keep table from being deleted
//   <li> handle movie axis once ActiveCachingNDim exists
//   <li> implement showValue()
//   <li> be able to derive xy plot data from columns with arrays
//   <li> choose whether to draw points, lines or both 
// </todo>

class TblAsXYDD : public ActiveCaching2dDD {

 public:

  // constructors
  // given an already constructed table
  TblAsXYDD(Table *table);

  // given a string which gives the full pathname and filename of a table 
  // on disk
  TblAsXYDD(const String tablename);

  // Destructor
  virtual ~TblAsXYDD();

  // format the table value at the give world position 
  virtual String showValue(const Vector<Double> &world);

  // get the data unit 
  virtual const Unit dataUnit(const String column);
  virtual const Unit dataUnit();

  // install the default options for this DisplayData
  virtual void setDefaultOptions();

  // Apply options stored in <src>rec</src> to the DisplayData.  A
  // return value of <src>True</src> means a refresh is needed.
  // <src>recOut</src> contains any fields which were implicitly 
  // changed as a result of the call to this function.  
  virtual Bool setOptions(Record &rec, Record &recOut);

  // Retrieve the current and default options and parameter types.
  virtual Record getOptions();

  // Return the type of this DisplayData.
  virtual Display::DisplayDataType classType()
    { return Display::Vector; }     

  // Create a new TblAsXyDM for drawing on the given
  // WorldCanvas when the AttributeBuffers are suitably matched to the
  // current state of this DisplayData and of the WorldCanvas/Holder.
  // The tag is a unique number used to identify the age of the newly
  // constructed CachingDisplayMethod.
  virtual CachingDisplayMethod *newDisplayMethod(WorldCanvas *worldCanvas,
                                         AttributeBuffer *wchAttributes,
                                         AttributeBuffer *ddAttributes,
                                         CachingDisplayData *dd);
 
  // Return the current options of this DisplayData as an
  // AttributeBuffer.
  virtual AttributeBuffer optionsAsAttributes();

  //provide read-only access to the table
  Table *table();

  // Clean up (ie. delete any existing cached display list).
  virtual void cleanup();

 protected:

  // (Required) default constructor.
  TblAsXYDD();

  // (Required) copy constructor.
  TblAsXYDD(const TblAsXYDD &other);

  // (Required) copy assignment.
  void operator=(const TblAsXYDD &other);

 // Get the value of the named keyword, or the first keyword matching
  // <src>regex</src>, and return it in <src>value</src>.  The return
  // value is <src>True</src> for success, and <src>False</src> for
  // failure, which is the result if the wrong type <src>T</src> is
  // requested.
  // <group>
  template <class T> Bool getTableKeyword(T &value,
                                          const String keyword) const;
  template <class T> Bool getTableKeyword(T &value, const Regex &regex) const;
  // </group>
 
  // Get the value of the named keyword, or the first keyword matching
  // <src>regex</src> for the named column, and return it in
  // <src>value</src>. The return value is <src>True</src> for
  // success, and <src>False</src> for failure, which is the result if           // the wrong type <src>T</src> is requested, or if the keyword
  // doesn't exist.
  // <group>
  template <class T> Bool getColumnKeyword(T &value, const String column,
                                           const String keyword) const;
  template <class T> Bool getColumnKeyword(T &value, const String column,
                                           const Regex &regex) const;
  // </group>                                                                    
private:

  friend class TblAsXYDM;

  // The table to be displayed
  Table *itsTable;

  // The result from a table query
  Table *itsQueryTable;

  // store all the table column names
  Vector<String> itsColumnNames;

  // what columns are we displaying and do we have a movie axis available
  DParameterChoice *itsXColumnName;
  DParameterChoice *itsYColumnName;
  DParameterChoice *itsMColumnName;
  DParameterChoice *itsMColumnSet;

  // options - what is the query string and is it unset?
  String itsOptQueryString;
  Bool itsOptQueryStringUnset;

  // set the default options for this display data
  void installDefaultOptions();

  // Arrange the query table (called after changing an option).
  Bool arrangeQueryTable();
 
  // holder for the current coordinate system
  CoordinateSystem itsCoord;
  Vector<Double> itsLinblc, itsLintrc;

  // update/set the coordinate system 
  void getCoordinateSystem();
  void setCoordinateSystem();

  // get all of the table columnNames
  void getTableColumnNames();

  // get the table column world coordinate range
  Vector<double> columnStatistics(const String& columnName);

  // get all of the table columnNames with a certain data type
  Vector<String> getColumnNamesOfType();

  // Construct and destruct the parameter set.
  // <group>
  void constructParameters();
  void destructParameters();
  // </group>
                   
};



} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/DisplayDatas/TblAsXYDDTemplates.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC
#endif


