//# ColormapDefinition.h: wrapper for colormap function or look-up table
//# Copyright (C) 1998,1999,2000,2001
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

#ifndef TRIALDISPLAY_COLORMAPDEFINITION_H
#define TRIALDISPLAY_COLORMAPDEFINITION_H

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/IO/AipsIO.h>
#include <tables/Tables/Table.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// This class defines a wrapper for a color function or look-up table.
// </summary>

// <use visibility=local>

// <reviewed reviewer="" date="yyyy/mm/dd" test="" demos="">
// </reviewed>

// <etymology>
// "ColormapDefinition" is the thing one uses to define a colormap.
// </etymology>

// <synopsis>
// ColormapDefinition is a class whose principal purpose is, given an
// input value in the range <src>[0.0, 1.0]</src>, to return an RGB
// triplet corresponding to that position in the colormap.  It can
// read definitions from and write definitions to <linkto
// class="Table"> Tables </linkto> on disk, thereby providing a
// mechanism for using custom colormaps for display applications. 
//

// It supplies one built-in colormap <src>Greyscale 1</src>, which will
// always be available. The other colormaps are stored in AIPS++
// tables. The standard colormaps ar in
// <src>/aips++/data/colormaps/default.tbl</src>. These can be
// deactivated by putting <src>display.colormaps.usedefault: no</src> in
// the <src>.aipsrc</src> file. 
// It also supplies user defined colormaps. It looks for the complete
// path to the user table in <src>.aipsrc</src> under
// <src>display.colormaps.usertable:</src>.
//
// ColormapDefinition is used by the <linkto
// class="Colormap">Colormap</linkto> class to generate color values
// for the <linkto class="ColormapManager"> ColormapManager</linkto>.
// </synopsis>

// <example>
// A ColormapDefinition corresponding to the system "rainbow" Colormap
// can be obtained and used as follows:
// <srcblock>
// ColormapDefinition rainbowDefinition(String("rainbow"));
// Float red, green, blue;
// for (uInt i = 0; i <= 100; i++) {
//   rainbowDefinition.getValues((Float)i/100.0, red, green, blue);
//   // ... do something with red, green, blue ...
// }
// </srcblock>
// Or a new ColormapDefinition with a red ramp and green and blue fixed
// at 0.5 could be constructed and saved for later use as follows:
// <srcblock>
// Vector<Float> reds(40), greens(40), blues(40);
// greens = 0.5;
// blues = 0.5;
// for (uInt i = 0; i < 40; i++) {
//   reds(i) = (Float)i / 39.0;
// }
// ColormapDefinition simpleRamp("redRamp", reds, greens, blues);
// Vector<String> synonyms(2);
// synonyms(0) = "RedRamp";synonyms(1) = "redramp";
// simpleRamp.save("mytable.tbl,synonyms);
// </srcblock>
// </example>

// <motivation>
// Needed to separate out ColormapDefinition from Colormap to give the
// programmer a way to over-ride the definition without having to also
// over-ride the shape function.
// </motivation>

// <thrown>
// <li> AipsError: unrecognized map name 
// <li> AipsError: incompatible colormap definition version 
// </thrown>

// <todo asof="1998/12/14">
// <li> add checks for red/green/blue componenets to have same number
//      of elements 
// </todo>

class RegEx;
class String;

class ColormapDefinition {

 public:

  // Construct a single color (white) Colormap
  ColormapDefinition();

  // Construct the known Colormap <src>mapName</src>, first looking for
  // a saved Colormap having this name, then resorting to a built-in
  // Colormap, and if that doesn't exist, throw an error
  explicit ColormapDefinition(const String & mapName);
  
  // Construct a new Colormap, using the supplied name and component
  // vectors
  ColormapDefinition(const String & mapName,
		     const Vector<Float> & reds, 
		     const Vector<Float> & greens,
		     const Vector<Float> & blues);

  // Obtain the Colormap value for the "index" value <src>0 <= t <= 1</src>
  void getValue(const Float t, Float & red, Float & green, Float & blue) const;

  // Change the Colormap values with the provided component vectors
  void setValues(const Vector<Float> & reds, 
		 const Vector<Float> & greens,
		 const Vector<Float> & blues);

  // Write this ColormapDefinition to the named Table in the named
  // directory (default values are obtained from the user's
  // <src>.aipsrc</src> file.  If <src>overwrite</src> is
  // <src>True</src>, then an existing map of the same name in the
  // Table will be over-written.  If the named Table does not exist,
  // it will be created.

  // The table format has to conform with following scheme. 
  // It must have five columns: 
  // <src>CMAP_NAME</src> a String
  // <src>RED</src> a Float array of dim n
  // <src>GREEN</src> a Float array of dim n
  // <src>BLUE</src> a Float array of dim n
  // <src>SYNONYMS</src> a String array of dim m
  Bool save(const String &fullPathName, 
	    const Vector<String> &synonyms,
	    const Bool &overwrite = True) const;

  // Return the names of the built-in colormaps.  If <src>uniqueonly</src>
  // is True (default), only the names of the unique colormaps 
  // are returned, otherwise all colormap names are returned.
  static Vector<String> builtinColormapNames(Bool uniqueonly = True);

  // Load Colormap definitions for a specified colormap<src>name</src>
  Bool loadColormap(const String& name);
  Bool loadBuiltinColormap(const String& name);

  // Write a ColormapDefinition to an ostream in a simple text form.
  friend ostream & operator << (ostream & os, 
				const ColormapDefinition& pcreh);

private:

  // The name of this ColormapDefinition
  String itsName;
  
  // Utility function to look if a colormap name is in a Table
  Bool queryColormapTable(const Table& table, const String& name);

  // load ColormapDefinitions from default location 
  // aips++/data/colormaps/default.tbl
  // and/or location specified in <src>display.colormaps.usertable</src>
  static void loadColormapTable();
  
  //The loaded colormaps (a replacement for the builtins)
  //<group>
  static String ourDefaultColormap;
  static Table ourDefaultColormapTable;
  static Table ourUserColormapTable;
  static String ourTableVersion;
  //</group>

  // The Color component vectors for this ColormapDefinition
  Vector<Float> itsReds;
  Vector<Float> itsGreens;
  Vector<Float> itsBlues;

};


} //# NAMESPACE CASA - END

#endif
