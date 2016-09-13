//# ColormapDefinition.cc: defines colormap function or look-up table
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

#include <unistd.h>
#include <casa/stdlib.h>
#include <casa/iostream.h>
#include <casa/aips.h>
#include <casa/Utilities/Regex.h>
#include <casa/IO/AipsIO.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/System/Aipsrc.h>
#include <casa/Exceptions/Error.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <display/Display/ColormapDefinition.h>


namespace casa { //# NAMESPACE CASA - BEGIN

// statics
	String ColormapDefinition::ourDefaultColormap("");
	String ColormapDefinition::ourTableVersion("1.0");
	Table ColormapDefinition::ourDefaultColormapTable = Table();
	Table ColormapDefinition::ourUserColormapTable = Table();

	ColormapDefinition::ColormapDefinition() :
		itsReds(1u,0.0),
		itsGreens(1u,0.0),
		itsBlues(1u,0.0) {
	}

	ColormapDefinition::ColormapDefinition(const String & mapName) :
		itsName(mapName),
		itsReds(1u,0.0),
		itsGreens(1u,0.0),
		itsBlues(1u,0.0) {

		// static data member
		if (ourDefaultColormap.empty()) {
			Aipsrc::find(ColormapDefinition::ourDefaultColormap,
			             "display.colormaps.defaultcolormap","Greyscale 1");
		}

		if (itsName == "<default>") {
			itsName = ourDefaultColormap;
		}
		Bool ok = loadBuiltinColormap(itsName);
		// if colormap tables haven't been read yet, do this
		if (ourDefaultColormapTable.isNull() && ourUserColormapTable.isNull()) {
			loadColormapTable();
		}

		if (!ok) {
			// not the buitlin colormap
			ok = loadColormap(itsName);
		}
		if ( !ok ) {
			// .aipsrc has wrong entry set everything to builtin
			ourDefaultColormap = "Greyscale 1";
			itsName = "Greyscale 1";
			ok = loadBuiltinColormap(itsName);
		}
		if (!ok) {
			// gee this is a strange colormap name
			throw(AipsError(String("Unrecognized map name '") + itsName +
			                String("'")));
		}
	}

	ColormapDefinition::ColormapDefinition(const String & name,
	                                       const Vector<Float> & reds,
	                                       const Vector<Float> & greens,
	                                       const Vector<Float> & blues)
		: itsName(name) {
		setValues(reds, greens, blues);
	}

// Always succeeds
// t is valid between 0.0 and 1.0, is clamped to between 0 and 1
	void ColormapDefinition::getValue(const Float t, Float & red,
	                                  Float & green, Float & blue) const {
		Float z = t;
		if (z < 0) {
			z = 0;
		} else if (z > 1) {
			z = 1;
		}

		// Interpolate
		Float fndx = (itsReds.nelements()-1)*z;
		uInt ndx = (uInt) fndx;
		if (ndx >= (itsReds.nelements()-1)) ndx--;
		Float offset = fndx - (Float) ndx;

		red = itsReds(ndx)*(1.0-offset) + itsReds(ndx+1)*offset;
		green = itsGreens(ndx)*(1.0-offset) + itsGreens(ndx+1)*offset;
		blue = itsBlues(ndx)*(1.0-offset) + itsBlues(ndx+1)*offset;
	}

	void ColormapDefinition::setValues(const Vector<Float> & reds,
	                                   const Vector<Float> & greens,
	                                   const Vector<Float> & blues) {
		itsReds.resize(reds.nelements());
		itsReds = reds.copy();
		itsGreens.resize(greens.nelements());
		itsGreens = greens.copy();
		itsBlues.resize(blues.nelements());
		itsBlues = blues.copy();
	}

	ostream& operator << (ostream& ios, const ColormapDefinition& cmap) {
		cout << "[map=";
		for (uInt i = 0; i < cmap.itsReds.nelements(); i++) {
			uInt r = (uInt) (cmap.itsReds(i) * 255.0);
			uInt g = (uInt) (cmap.itsGreens(i) * 255.0);
			uInt b = (uInt) (cmap.itsBlues(i) * 255.0);
			cout << "<" << r << "," << g << "," << b << ">";
		}
		cout << "]";
		return ios;
	}

//static function !!!
	void ColormapDefinition::loadColormapTable() {
		String root = Aipsrc::aipsRoot();
		String defaultpath = root+"/data/gui/colormaps/default.tbl";
		String useSystemCmap;
		String altpath,userpath;
		Aipsrc::find(useSystemCmap,"display.colormaps.usedefault","yes");
		Aipsrc::find(userpath,"display.colormaps.usertable","");
		try {
			if (!useSystemCmap.matches(Regex("[ \t]*(([nN]o)|([fF]alse))[ \t\n]*"))) {
				// default cmaps
				ourDefaultColormapTable = Table(defaultpath);
				if (!userpath.empty()) {
					// default and user cmaps
					ourUserColormapTable = Table(userpath);
				}
			} else {
				if (!userpath.empty()) {
					// user cmaps only
					ourUserColormapTable = Table(userpath);
				}
			}
		} catch (const AipsError &x) {
			fprintf( stderr, "fatal error, could not read default colormaps: %s\n", x.what() );
			exit(1);
		} catch (...) {
			fprintf( stderr, "fatal error, could not read default colormaps...\n" );
			exit(1);
		}
	}

	Bool ColormapDefinition::loadBuiltinColormap(const String& name) {
		Vector<String> names(5);
		names(0) = "Greyscale 1";
		names(1) = "mono";
		names(2) = "greyscale";
		names(3) = "grayscale";
		names(4) = "builtin";

		for (uInt i=0; i<names.nelements(); i++) {
			if (name == names(i)) {
				itsReds.resize(2);
				itsGreens.resize(2);
				itsBlues.resize(2);
				itsReds(0)= 0.0;
				itsReds(1)= 1.0;
				itsGreens(0)= 0.0;
				itsGreens(1)= 1.0;
				itsBlues(0)= 0.0;
				itsBlues(1)= 1.0;
				return True;
			}
		}
		return False;
	}

	Bool ColormapDefinition::queryColormapTable(const Table& table,
	        const String& name) {

		ROScalarColumn<String> nameCol (table, "CMAP_NAME");
		ROArrayColumn<Float> rCol (table, "RED");
		ROArrayColumn<Float> gCol (table, "GREEN");
		ROArrayColumn<Float> bCol (table, "BLUE");
		ROArrayColumn<String> synCol (table, "SYNONYMS");

		Int rowNumber = -1;
		Vector<String> synonyms;
		uInt n = table.nrow();
		Bool foundMatch = False;
		for (uInt i=0; i < n && !foundMatch; i++) {
			// found CMAP_NAME
			if (name == nameCol(i)) {
				rowNumber = Int(i);
				foundMatch = True;
				break;
			}
			synonyms.resize();
			synonyms = Vector<String>(synCol(i));
			for (uInt k=0; k < synonyms.nelements() ; k++) {
				// found SYNONYM
				if (name == synonyms(k)) {
					rowNumber = Int(i);
					foundMatch = True;
					break;
				}
			}
		}
		if (foundMatch) {
			// assume r,g,b have the same length
			uInt n = rCol(rowNumber).nelements();
			itsReds.resize(n);
			itsGreens.resize(n);
			itsBlues.resize(n);
			itsReds = Vector<Float>(rCol(rowNumber));
			itsGreens = Vector<Float>(gCol(rowNumber));
			itsBlues = Vector<Float>(bCol(rowNumber));
			return True;
		}
		return False;
	}

	Bool ColormapDefinition::loadColormap(const String& name) {
		Bool foundName = False;
		if (!ourDefaultColormapTable.isNull()) {
			foundName = queryColormapTable(ourDefaultColormapTable, name);
		}
		if (!foundName && !ourUserColormapTable.isNull()) {
			foundName = queryColormapTable(ourUserColormapTable, name);
		}
		return foundName;
	}

	Bool ColormapDefinition::save(const String &fullPathName,
	                              const Vector<String> &synonyms,
	                              const Bool &/*overwrite*/) const {
		Table *tbl = 0;
		try {
			tbl = new Table(fullPathName, Table::Update);
		} catch (const AipsError &x) {
			if (&x) { };
		}
		if (!tbl) {
			// probably table doesn't exist, so let's try to create it:
			try {
				TableDesc td("ColormapTable", ourTableVersion, TableDesc::Scratch);
				td.comment() = "Colormap definitions";
				td.addColumn(ScalarColumnDesc<String>("CMAP_NAME", "Colormap name"));
				td.addColumn(ArrayColumnDesc<Float>("RED", "Red components", 1));
				td.addColumn(ArrayColumnDesc<Float>("GREEN", "Green components", 1));
				td.addColumn(ArrayColumnDesc<Float>("BLUE", "Blue components", 1));
				td.addColumn(ArrayColumnDesc<String>("SYNONYMS", "Synonyms", 1));
				SetupNewTable newtab(fullPathName, td, Table::New);
				tbl = new Table(newtab, 0);
			} catch (const AipsError &x) {
				if (&x) {
					throw(AipsError("Couldn't create table for saving colormap"));
				}
			}
		}

		// check that four required columns are present:
		Vector<String> cnames = tbl->tableDesc().columnNames();
		if (cnames.nelements() < 5) {
			throw(AipsError("Named table is an invalid colormap store"));
		}
		if (!((cnames(0) == String("CMAP_NAME")) &&
		        (cnames(1) == String("RED")) &&
		        (cnames(2) == String("GREEN")) &&
		        (cnames(3) == String("BLUE")) &&
		        (cnames(4) == String("SYNONYMS")))) {
			throw(AipsError("Cannot save colormap to named table"));
		}

		// see if named colormap already exists:
		ScalarColumn<String> namecol(*tbl, "CMAP_NAME");
		Vector<String> names = namecol.getColumn();
		uInt rownr = names.nelements();;
		for (uInt i = 0; i < names.nelements(); i++) {
			if (names(i) == itsName) {
				rownr = i;
			}
		}
		if (rownr == names.nelements()) {
			// add a row to the table:
			tbl->addRow();
		}

		namecol.put(rownr, itsName);
		ArrayColumn<Float> acred(*tbl, "RED");
		acred.put(rownr, itsReds);
		ArrayColumn<Float> acgreen(*tbl, "GREEN");
		acgreen.put(rownr, itsGreens);
		ArrayColumn<Float> acblue(*tbl, "BLUE");
		acblue.put(rownr, itsBlues);
		ArrayColumn<String> acsyn(*tbl, "SYNONYMS");
		acsyn.put(rownr, synonyms);

		tbl->flush();
		delete tbl;
		tbl=0;

		return True;
	}


	std::map<String,bool> ColormapDefinition::builtinColormapNames(Bool /*uniqueonly*/) {
		// uniqueonly is not used at the moment
		// static data members
		if (ourDefaultColormap.empty()) {
			Aipsrc::find(ColormapDefinition::ourDefaultColormap,
			             "display.colormaps.defaultcolormap","Greyscale 1");
		}
		if (ourDefaultColormapTable.isNull() && ourUserColormapTable.isNull()) {
			loadColormapTable();
		}
		//
		uInt defaultLength = 0;
		uInt userLength = 0;

		colormapnamemap vec;
		ROScalarColumn<String>* name1Col(0);
		ROScalarColumn<String>* name2Col(0);
		if (!ourDefaultColormapTable.isNull()) {
			name1Col = new ROScalarColumn<String>(ourDefaultColormapTable, "CMAP_NAME");
			defaultLength = ourDefaultColormapTable.nrow();
		}
		if (!ourUserColormapTable.isNull()) {
			name2Col = new ROScalarColumn<String>(ourUserColormapTable, "CMAP_NAME");
			userLength = ourUserColormapTable.nrow();
		}

		vec.insert(colormapnamemap::value_type("Greyscale 1",true));

		// add colormap table entries
		for (uInt i=0; i <defaultLength; i++) {
			vec.insert(colormapnamemap::value_type(name1Col->operator()(i),true));
		}
		for (uInt i=0; i <userLength; i++) {
			vec.insert(colormapnamemap::value_type(name2Col->operator()(i),true));
		}
		if (name1Col) {
			delete name1Col;
		}
		if (name2Col) {
			delete name2Col;
		}
		return vec;
	}

} //# NAMESPACE CASA - END

