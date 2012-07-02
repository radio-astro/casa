//# SkyCatOverlay.h: sky catalogue overlay displaydata
//# Copyright (C) 1999,2000,2001,2002
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
//# $Id$

#ifndef TRIALDISPLAY_SKYCATOVERLAYDD_H
#define TRIALDISPLAY_SKYCATOVERLAYDD_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <display/Display/DParameterRange.h>
#include <display/Display/DParameterChoice.h>
#include <display/Display/DParameterColorChoice.h>
#include <display/Display/DParameterString.h>
#include <display/Display/DParameterMapKeyChoice.h>
#include <display/DisplayDatas/PassiveTableDD.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class Table;
class SkyCatOverlayDM;

// <summary>
// Simple implementation of a cached DisplayData for sky catalogue overlays.
// </summary>

class SkyCatOverlayDD : public PassiveTableDD {

 public:

  // Constructor taking a pointer to an already constructed Table.
  SkyCatOverlayDD(Table* table);

  // Constructor taking a String which describes the full pathname
  // of a Table on disk.
  SkyCatOverlayDD(const String tablename);
  
  // Destructor.
  virtual ~SkyCatOverlayDD();

  // Install the default options for this DisplayData.
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
    { return Display::CanvasAnnotation; }
  
  // Create a new AxesDisplayMethod for drawing on the given
  // WorldCanvas when the AttributeBuffers are suitably matched to the
  // current state of this DisplayData and of the WorldCanvas/Holder.
  // The tag is a unique number used to identify the age of the newly
  // constructed CachingDisplayMethod.
  virtual CachingDisplayMethod* newDisplayMethod(WorldCanvas* worldCanvas,
					       AttributeBuffer* wchAttributes,
					       AttributeBuffer* ddAttributes,
					       CachingDisplayData* dd);
 
  // Return the current options of this DisplayData as an
  // AttributeBuffer.
  virtual AttributeBuffer optionsAsAttributes();


  // Get the column name for text labelling.
  virtual String nameColumn() const
    { return itsParamNameColumn->value(); }
  
  // Get the line width for labelling.
  virtual Float lineWidth() const
    { return itsParamLineWidth->value(); }

  // Get the marker type, size and color.
  // <group>
  virtual Display::Marker markerType() const
  { return static_cast<Display::Marker>(itsParamMarkerType->keyValue()); }
  virtual Int markerSize() const
    { return itsParamMarkerSize->value(); }
  virtual String markerColor() const
    { return itsParamMarkerColor->value(); }
  // </group>
  // Get the column name for value-size mapping.
  virtual String mapColumn() const
    { return itsParamMapColumn->value(); }


  // Get the character font, size, color and angle for labelling.
  // <group>
  virtual String charFont() const
  { return itsParamCharacterFont->value(); }
  virtual Float charSize() const
  { return itsParamCharacterSize->value(); }
  virtual String charColor() const
    { return itsParamCharacterColor->value(); }
  virtual Int charAngle() const
    { return itsParamCharacterAngle->value(); }
  // </group>

  // Get the label x and y offsets (in character height).
  // <group>
  virtual Float labelXOffset() const
    { return itsParamLabelXOffset->value(); }
  virtual Float labelYOffset() const
    { return itsParamLabelYOffset->value(); }
  // </group>

  // Determine ability to draw, given current canvas coordinate state.
  // This DD requires direction coordinates in both X and Y axis codes.
  virtual Bool conformsToCS(const WorldCanvasHolder& wch);
  
protected:

  // (Required) default constructor.
  SkyCatOverlayDD();

  // (Required) copy constructor.
  SkyCatOverlayDD(const SkyCatOverlayDD &other);

  // (Required) copy assignment.
  void operator=(const SkyCatOverlayDD &other);

  // Attempt to determine which columns map to longitude, latitude,
  // and type.
  virtual Bool determineDirectionColumnMapping();

  // Return the unit of a given column in the table.  This is
  // extracted from the first column keyword matching the pattern
  // <src>^[uU][nN][iI][tT]$</src>.  If no unit keyword is available,
  // the returned unit is a null unit.
  Unit columnUnit(const String columnName) const;

private:

  // Store the column names here.
  Vector<String> itsColumnNames;

  // Option: what type of coordinates are provided?
  //String itsCoordinateType;

  // Which column is mapped to the longitude coordinate, and what is
  // its unit?
  String itsLongitudeColumn;
  Unit itsLongitudeUnit;

  // Which column is mapped to the latitude coordinate, and what is
  // its unit?
  String itsLatitudeColumn;
  Unit itsLatitudeUnit;

  // Which column is mapped to epoch
  String itsDirectionTypeColumn;

  // Option: which column is mapped to the name?
  DParameterChoice* itsParamNameColumn;

  // plot line width
  DParameterRange<Int>* itsParamLineWidth;

  // Marker type, size and color.
  DParameterMapKeyChoice* itsParamMarkerType;
  DParameterRange<Int>* itsParamMarkerSize;
  DParameterColorChoice* itsParamMarkerColor;
  DParameterChoice* itsParamMapColumn;


  // Labelling character font, size, color and angle.
  DParameterChoice* itsParamCharacterFont;
  DParameterRange<Float>* itsParamCharacterSize;
  DParameterColorChoice* itsParamCharacterColor;
  DParameterRange<Int>* itsParamCharacterAngle;

  // Labelling character offsets in character height units.
  DParameterRange<Float>* itsParamLabelXOffset;
  DParameterRange<Float>* itsParamLabelYOffset;

  // Allow the DisplayMethod to access the private data.
  friend class SkyCatOverlayDM;

  // Construct and destruct the parameter set.
  // <group>
  void constructParameters();
  void destructParameters();
  // </group>

};


} //# NAMESPACE CASA - END

#endif  
