//# Copyright (C) 2004
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
#ifndef SEARCHERLOCAL_H_
#define SEARCHERLOCAL_H_

#include <spectrallines/Splatalogue/Searcher.h>
#include <casa/Arrays/Vector.h>

namespace casa {

/**
 * Searches a local database for molecular lines meeting the specified
 * search criteria.
 */

class SearcherLocal : public Searcher{
public:
	SearcherLocal();

	//Search Paramaters
	virtual void setChemicalNames( const Vector<String>& chemNames );
	virtual void setSpeciesNames( const Vector<String>& speciesNames );
	virtual void setDatabasePath( const String& databasePath );
	virtual void setResultFile( const String& name );
	virtual void setSearchRangeFrequency( double minValue, double maxValue );

	//Astronomical Filters
	virtual void setAstroFilterTop20();
	virtual void setAstroFilterPlanetaryAtmosphere();
	virtual void setAstroFilterHotCores();
	virtual void setAstroFilterDarkClouds();
	virtual void setAstroFilterDiffuseClouds();
	virtual void setAstroFilterComets();
	virtual void setAstroFilterAgbPpnPn();
	virtual void setAstroFilterExtragalactic();

	//Performing the Search
	virtual Record doSearch( String& errorMsg );
	virtual ~SearcherLocal();
private:

	String fileName;
	String databasePath;
	double minValueFreq;
	double maxValueFreq;
	Vector<String> chemicalNames;
	Vector<String> speciesNames;
	Vector<String> top20Names;
	Vector<String> extragalacticNames;
	Vector<String> planetNames;
	Vector<String> hotCoreNames;
	Vector<String> darkCloudNames;
	Vector<String> diffuseCloudNames;
	Vector<String> cometNames;
	Vector<String> agbPpnPnNames;
	Vector<String> filterList;

	const static String ACETALDEHYDE;
	const static String ACETONE;
	const static String ACETYLENE;
	const static String ALUMINUM_ISOCYANITRILE;
	const static String ALUMINUM_MONOCHLORIDE;
	const static String ALUMINUM_MONOFLUORIDE;
	const static String AMIDOGEN;
	const static String AMINOACETONITRILE;
	const static String AMMONIA;
	const static String ATOMIC_CARBON;
	const static String ATOMIC_HYDROGEN;
	const static String BUTADIYNYL13_ANION;
	const static String BUTADIYNYL13_RADICAL;
	const static String BUTATRIENYLIDENE;
	const static String BUTYNENITRILE2;
	const static String CARBON_MONOPHOSPHIDE;
	const static String CARBON_MONOSULFIDE;
	const static String CARBON_MONOXIDE;
	const static String CARBON_MONOXIDE_ION;
	const static String CARBONYL_SULFIDE;
	const static String CYANAMIDE;
	const static String CYANIC_ACID;
	const static String CYANIDE_RADICAL;
	const static String CYANOETHYNYL;
	const static String CYANOALLENE;
	const static String CYANOBUTADIYNE;
	const static String CYANOBUTADIYNYLIDE_ANION;
	const static String CYANO4_BUTADIYNYL13;
	const static String CYANOACETYLENE;
	const static String CYANOETHYNYLIDE_ION;
	const static String CYANOFORMALDEHYDE;
	const static String CYANOMETHYL;
	const static String CYANOMETHYLENE;
	const static String CYANO3_PROPADIENYLIDENE12;
	const static String CYCLOPROPENONE;
	const static String CYCLOPROPENYLIDENE;
	const static String DIAZENYLIUM;
	const static String DIMETHYL_ETHER;
	const static String ETHYL_CYANIDE;
	const static String ETHYLENE_GLYCOL;
	const static String ETHYLENE_OXIDE;
	const static String ETHYNE_ISOCYANIDE;
	const static String ETHYNYL;
	const static String FLUOROMETHYLIUMYLIDENE;
	const static String FORMALDEHYDE;
	const static String FORMAMIDE;
	const static String FORMIC_ACID;
	const static String FORMYL_RADICAL;
	const static String FORMYLIUM;
	const static String FULMINIC_ACID;
	const static String GAUCHE_ETHANOL;
	const static String GLYCOLALDEHYDE;
	const static String HEPTATRIYNENITRILE246;
	const static String HEPTATRIYNYLIDYNE246;
	const static String HEXATRIENYL;
	const static String HEXATRIYNYL135;
	const static String HEXATRIYNYL135_ANION;
	const static String HYDROCHLORIC_ACID;
	const static String HYDROGEN_CYANIDE;
	const static String HYDROGEN_FLUORIDE;
	const static String HYDROGEN_ISOCYANIDE;
	const static String HYDROGEN_SULFIDE;
	const static String HYDROXYL;
	const static String HYDROXYMETHYLIUM_ION;
	const static String IMINOMETHYLIUM;
	const static String IMINO3_PROPADIENYLIDENE12;
	const static String IMIDOGEN;
	const static String ISOCYANIC_ACID;
	const static String ISOTHIOCYANIC_ACID;
	const static String KETENE;
	const static String KETENIMINE;
	const static String MAGNESIUM_CYANIDE;
	const static String MAGNESIUM_ISOCYANIDE;
	const static String METHANIMINE;
	const static String METHANOL;
	const static String METHYL_ACETYLENE;
	const static String METHYL_CYANIDE;
	const static String METHYL_DIACETYLENE;
	const static String METHYL_FORMATE;
	const static String METHYL_ISOCYANIDE;
	const static String METHYLCYANODIACETYLENE;
	const static String METHYLAMINE;
	const static String METHYLENE;
	const static String METHYLENE_AMIDOGEN;
	const static String METHYLIDYNE;
	const static String METHYLIUMYLIDENE;
	const static String METHYLTRIACETYLENE;
	const static String NITRIC_SULFIDE;
	const static String NITRIC_OXIDE;
	const static String NITROSYL_HYDRIDE;
	const static String NITROUS_OXIDE;
	const static String NONATETRAYNENITRILE2468;
	const static String OCTATETRAYNYL1357;
	const static String OCTATETRAYNYL1357_ANION;
	const static String OXOETHENYLIDENE;
	const static String OXONIUM_HYDRIDE;
	const static String OXO3_PROPADIENYLIDENE12;
	const static String PENTADIYNYLIDYNE24;
	const static String PHOSPHAETHYNE;
	const static String PHOSPHAPROPYNYLIDYNE;
	const static String PHOSPHOROUS_MONOXIDE;
	const static String PHOSPHOROUS_NITRIDE;
	const static String PROPADIENYLIDENE;
	const static String PROPENAL;
	const static String PROPYNAL2;
	const static String PROPYNYLIDYNE;
	const static String POTASSIUM_CHLORIDE;
	const static String PROTONATE_2_PROYNENITRILE;
	const static String PROTONATED_CARBON_DIOXIDE;
	const static String SILICON_CARBIDE;
	const static String SILICON_CYANIDE;
	const static String SILICON_ISOCYANIDE;
	const static String SILICON_MONOCARBIDE;
	const static String SILICON_MONONITRIDE;
	const static String SILICON_MONOSULFIDE;
	const static String SILICON_MONOXIDE;
	const static String SILICON_TETRACARBIDE;
	const static String SILANETETRAYL3_PROPADIENYLIDENE12;
	const static String SODIUM_CHLORIDE;
	const static String SODIUM_CYANIDE;
	const static String SULFUR_DIOXIDE;
	const static String SULFUR_MONOXIDE;
	const static String SULFUR_MONOXIDE_ION;
	const static String TRIHYDROGEN_ION;
	const static String THIOCYANIC_ACID;
	const static String THIOFORMALDEHYDE;
	const static String THIOFORMYLIUM;
	const static String THIOXOETHENYLIDENE;
	const static String THIOXO_12_PROPADIEYLIDENE3;
	const static String UNDECAPENTAYNENITRILE246810;
	const static String VINYL_ALCOHOL;
	const static String VINYL_CYANIDE;
	const static String WATER;
};

} /* namespace casa */
#endif /* SEARCHLOCAL_H_ */
