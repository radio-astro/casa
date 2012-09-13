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
#include "SearcherTable.h"
#include <spectrallines/Splatalogue/SearchEngine.h>


#include <iostream>
using namespace std;

namespace casa {

const String SearcherTable::ACETALDEHYDE = "Acetaldehyde";
const String SearcherTable::ACETONE = "Acetone";
const String SearcherTable::ACETYLENE = "Acetylene";
const String SearcherTable::ALUMINUM_ISOCYANITRILE = "Aluminum Isocyanitrile";
const String SearcherTable::ALUMINUM_MONOCHLORIDE = "Aluminum Monochloride";
const String SearcherTable::ALUMINUM_MONOFLUORIDE = "Aluminum Monofluoride";
const String SearcherTable::AMIDOGEN = "Amidogen";
const String SearcherTable::AMINOACETONITRILE = "Aminoacetonitrile";
const String SearcherTable::AMMONIA = "Ammonia";
const String SearcherTable::ATOMIC_CARBON = "Atomic Carbon";
const String SearcherTable::ATOMIC_HYDROGEN = "Atomic Hydrogen";
const String SearcherTable::BUTADIYNYL13_ANION = "1,3-Butadiynyl anion";
const String SearcherTable::BUTADIYNYL13_RADICAL = "1,3-Butadiynyl radical";
const String SearcherTable::BUTATRIENYLIDENE = "Butatrienylidene";
const String SearcherTable::BUTYNENITRILE2 = "2-Butynenitrile";
const String SearcherTable::CARBON_MONOPHOSPHIDE = "Carbon Monophosphide";
const String SearcherTable::CARBON_MONOSULFIDE = "Carbon Monosulfide";
const String SearcherTable::CARBON_MONOXIDE = "Carbon Monoxide";
const String SearcherTable::CARBON_MONOXIDE_ION = "Carbon Monoxide Ion";
const String SearcherTable::CARBONYL_SULFIDE = "Carbonyl Sulfide";
const String SearcherTable::CYANAMIDE = "Cyanamide";
const String SearcherTable::CYANIC_ACID = "Cyanic acid";
const String SearcherTable::CYANIDE_RADICAL = "Cyanide Radical";
const String SearcherTable::CYANOACETYLENE = "Cyanoacetylene";
const String SearcherTable::CYANOETHYNYL = "Cyanoethynyl";
const String SearcherTable::CYANOALLENE = "Cyanoallene";
const String SearcherTable::CYANOBUTADIYNE = "Cyanobutadiyne";
const String SearcherTable::CYANOBUTADIYNYLIDE_ANION = "Cyanobutadiynylide anion";
const String SearcherTable::CYANO4_BUTADIYNYL13 = "4-Cyano-1,3-Butadiynyl";
const String SearcherTable::CYANOETHYNYLIDE_ION = "Cyanoethynylide ion";
const String SearcherTable::CYANOFORMALDEHYDE = "Cyanoformaldehye";
const String SearcherTable::CYANOMETHYL = "Cyanomethyl";
const String SearcherTable::CYANOMETHYLENE = "Cyanomethylene";
const String SearcherTable::CYANO3_PROPADIENYLIDENE12 = "3-Cyano-1,2-propadienylidene";
const String SearcherTable::CYCLOPROPENONE = "Cyclopropenone";
const String SearcherTable::CYCLOPROPENYLIDENE = "Cyclopropenylidene";
const String SearcherTable::DIAZENYLIUM = "Diazenylium";
const String SearcherTable::DIMETHYL_ETHER = "Dimethyl ether";
const String SearcherTable::ETHYL_CYANIDE = "Ethyl Cyanide";
const String SearcherTable::ETHYLENE_GLYCOL = "Ethylene Glycol";
const String SearcherTable::ETHYLENE_OXIDE = "Ethylene Oxide";
const String SearcherTable::ETHYNE_ISOCYANIDE = "Ethyne Isocyanide";
const String SearcherTable::ETHYNYL = "Ethynyl";
const String SearcherTable::FLUOROMETHYLIUMYLIDENE = "Fluoromethyliumylidene";
const String SearcherTable::FORMALDEHYDE = "Formaldehyde";
const String SearcherTable::FORMAMIDE = "Formamide";
const String SearcherTable::FORMIC_ACID = "Formic Acid";
const String SearcherTable::FORMYL_RADICAL = "Formyl Radical";
const String SearcherTable::FORMYLIUM = "Formylium";
const String SearcherTable::FULMINIC_ACID = "Fulminic acid";
const String SearcherTable::GAUCHE_ETHANOL = "gauche-Ethanol";
const String SearcherTable::GLYCOLALDEHYDE = "Glycolaldehyde";
const String SearcherTable::HEPTATRIYNENITRILE246 = "2,4,6-Heptatriynenitrile";
const String SearcherTable::HEPTATRIYNYLIDYNE246 = "2,4,6-Heptatriynylidyne";
const String SearcherTable::HEXATRIENYL = "Hexatrienyl";
const String SearcherTable::HEXATRIYNYL135 = "1,3,5-Hexatriynyl";
const String SearcherTable::HEXATRIYNYL135_ANION = "1,3,5-Hexatriynyl anion";
const String SearcherTable::HYDROCHLORIC_ACID = "Hydrochloric acid";
const String SearcherTable::HYDROGEN_CYANIDE = "Hydrogen Cyanide";
const String SearcherTable::HYDROGEN_FLUORIDE = "Hydrogen fluoride";
const String SearcherTable::HYDROGEN_ISOCYANIDE = "Hydrogen Isocyanide";
const String SearcherTable::HYDROGEN_SULFIDE = "Hydrogen sulfide";
const String SearcherTable::HYDROXYL = "Hydroxyl";
const String SearcherTable::HYDROXYMETHYLIUM_ION = "Hydroxymethylium ion";
const String SearcherTable::IMINOMETHYLIUM = "Iminomethylium";
const String SearcherTable::IMINO3_PROPADIENYLIDENE12 = "3-Imino-1,2 Propadienylidene";
const String SearcherTable::IMIDOGEN = "Imidogen";
const String SearcherTable::ISOCYANIC_ACID = "Isocyanic Acid";
const String SearcherTable::ISOTHIOCYANIC_ACID = "Isothiocyanic acid";
const String SearcherTable::KETENE = "Ketene";
const String SearcherTable::KETENIMINE = "Ketenimine";
const String SearcherTable::MAGNESIUM_CYANIDE = "Magnesium Cyanide";
const String SearcherTable::MAGNESIUM_ISOCYANIDE = "Magnesium Isocyanide";
const String SearcherTable::METHANIMINE = "Methanimine";
const String SearcherTable::METHANOL = "Methanol";
const String SearcherTable::METHYL_ACETYLENE = "Methyl Acetylene";
const String SearcherTable::METHYL_CYANIDE = "Methyl Cyanide";
const String SearcherTable::METHYL_DIACETYLENE = "Methyl diacetylene";
const String SearcherTable::METHYL_FORMATE = "Methyl Formate";
const String SearcherTable::METHYL_ISOCYANIDE = "Methyl isocyanide";
const String SearcherTable::METHYLCYANODIACETYLENE = "Methylcyanodiacetylene";
const String SearcherTable::METHYLAMINE = "Methylamine";
const String SearcherTable::METHYLENE = "Methylene";
const String SearcherTable::METHYLENE_AMIDOGEN = "Methylene amidogen";
const String SearcherTable::METHYLIDYNE = "Methylidyne";
const String SearcherTable::METHYLIUMYLIDENE = "Methyliumylidene";
const String SearcherTable::METHYLTRIACETYLENE="Methyltriacetylene";
const String SearcherTable::NITRIC_SULFIDE = "Nitric sulfide";
const String SearcherTable::NITRIC_OXIDE = "Nitric oxide";
const String SearcherTable::NITROSYL_HYDRIDE = "Nitrosyl hydride";
const String SearcherTable::NITROUS_OXIDE = "Nitrous Oxide";
const String SearcherTable::NONATETRAYNENITRILE2468 = "2,4,6,8-Nonatetraynenitrile";
const String SearcherTable::OCTATETRAYNYL1357 = "1,3,5,7-Octatetraynyl";
const String SearcherTable::OCTATETRAYNYL1357_ANION="1,3,5,7-Octatetraynyl anion";
const String SearcherTable::OXOETHENYLIDENE = "Oxoethenylidene";
const String SearcherTable::OXONIUM_HYDRIDE = "Oxonium hydride";
const String SearcherTable::OXO3_PROPADIENYLIDENE12="3-Oxo-1,2-Propadienylidene";
const String SearcherTable::PENTADIYNYLIDYNE24 = "2,4-Pentadiynylidyne";
const String SearcherTable::PHOSPHAETHYNE = "Phosphaethyne";
const String SearcherTable::PHOSPHAPROPYNYLIDYNE = "Phosphapropynylidyne";
const String SearcherTable::PHOSPHOROUS_MONOXIDE = "Phosphorus Monoxide";
const String SearcherTable::PHOSPHOROUS_NITRIDE = "Phosphorous nitride";
const String SearcherTable::PROPADIENYLIDENE = "Propadienylidene";
const String SearcherTable::PROPENAL = "Propenal";
const String SearcherTable::PROPYNAL2 = "2-Propynal";
const String SearcherTable::PROPYNYLIDYNE = "Propynylidyne";
const String SearcherTable::POTASSIUM_CHLORIDE = "Potassium chloride";
const String SearcherTable::PROTONATE_2_PROYNENITRILE = "Protonate 2-proynenitrile";
const String SearcherTable::PROTONATED_CARBON_DIOXIDE = "Protonated Carbon Dioxide";
const String SearcherTable::SILICON_CARBIDE = "Silicon Carbide";
const String SearcherTable::SILICON_CYANIDE = "Silicon Cyanide";
const String SearcherTable::SILICON_ISOCYANIDE = "Silicon Isocyanide";
const String SearcherTable::SILICON_MONOCARBIDE = "Silicon Monocarbide";
const String SearcherTable::SILICON_MONONITRIDE = "Silicon Mononitride";
const String SearcherTable::SILICON_MONOSULFIDE = "Silicon monosulfide";
const String SearcherTable::SILICON_MONOXIDE = "Silicon Monoxide";
const String SearcherTable::SILICON_TETRACARBIDE = "Silicon tetracarbide";
const String SearcherTable::SILANETETRAYL3_PROPADIENYLIDENE12 = "3-Silanetetrayl-1,2-Propadienylidene";
const String SearcherTable::SODIUM_CHLORIDE = "Sodium Chloride";
const String SearcherTable::SODIUM_CYANIDE = "Sodium Cyanide";
const String SearcherTable::SULFUR_DIOXIDE = "Sulfur dioxide";
const String SearcherTable::SULFUR_MONOXIDE = "Sulfur Monoxide";
const String SearcherTable::SULFUR_MONOXIDE_ION = "Sulfur Monoxide ion";
const String SearcherTable::TRIHYDROGEN_ION = "Trihydrogen ion";
const String SearcherTable::THIOCYANIC_ACID = "thiocyanic acid";
const String SearcherTable::THIOFORMALDEHYDE = "Thioformaldehyde";
const String SearcherTable::THIOFORMYLIUM = "Thioformylium";
const String SearcherTable::THIOXOETHENYLIDENE = "Thioxoethenylidene";
const String SearcherTable::THIOXO_12_PROPADIEYLIDENE3="3-Thioxo-1,2-Propadieylidene";
const String SearcherTable::UNDECAPENTAYNENITRILE246810 = "2,4,6,8,10-Undecapentaynenitrile";
const String SearcherTable::VINYL_ALCOHOL = "Vinyl Alcohol";
const String SearcherTable::VINYL_CYANIDE = "Vinyl Cyanide";
const String SearcherTable::WATER = "Water";



SearcherTable::SearcherTable() :top20Names(12), extragalacticNames(36),
		planetNames(19), hotCoreNames(81), darkCloudNames(51),
		diffuseCloudNames(20), cometNames(30), agbPpnPnNames(69){
	top20Names[0] = FORMALDEHYDE;
	top20Names[1] = HYDROXYL;
	top20Names[2] = AMMONIA;
	top20Names[3] = SULFUR_DIOXIDE;
	top20Names[4] = METHANOL;
	top20Names[5] = WATER;
	top20Names[6] = CARBON_MONOSULFIDE;
	top20Names[7] = FORMYLIUM;
	top20Names[8] = HYDROGEN_CYANIDE;
	top20Names[9] = SILICON_MONOXIDE;
	top20Names[10] = FORMYLIUM;
	top20Names[11] = CARBON_MONOXIDE;

	extragalacticNames[0] = AMMONIA;
	extragalacticNames[1] = ATOMIC_CARBON;
	extragalacticNames[2] = ATOMIC_HYDROGEN;
	extragalacticNames[3] = BUTADIYNYL13_RADICAL;
	extragalacticNames[4] = CARBON_MONOXIDE;
	extragalacticNames[5] = CARBON_MONOSULFIDE;
	extragalacticNames[6] = CARBONYL_SULFIDE;
	extragalacticNames[7] = CYANIDE_RADICAL;
	extragalacticNames[8] = CYANAMIDE;
	extragalacticNames[9] = CYANOACETYLENE;
	extragalacticNames[10] = CYCLOPROPENYLIDENE;
	extragalacticNames[11] = DIAZENYLIUM;
	extragalacticNames[12] = ETHYNYL;
	extragalacticNames[13] = FORMALDEHYDE;
	extragalacticNames[14] = FORMYLIUM;
	extragalacticNames[15] = HYDROGEN_CYANIDE;
	extragalacticNames[16] = HYDROGEN_ISOCYANIDE;
	extragalacticNames[17] = HYDROGEN_SULFIDE;
	extragalacticNames[18] = HYDROXYL;
	extragalacticNames[19] = ISOCYANIC_ACID;
	extragalacticNames[20] = METHANIMINE;
	extragalacticNames[21] = METHANOL;
	extragalacticNames[22] = METHYL_ACETYLENE;
	extragalacticNames[23] = METHYL_CYANIDE;
	extragalacticNames[24] = METHYLIDYNE;
	extragalacticNames[25] = METHYLIUMYLIDENE;
	extragalacticNames[26] = NITRIC_OXIDE;
	extragalacticNames[27] = NITRIC_SULFIDE;
	extragalacticNames[28] = PROTONATED_CARBON_DIOXIDE;
	extragalacticNames[29] = SILICON_MONOXIDE;
	extragalacticNames[30] = SODIUM_CHLORIDE;
	extragalacticNames[31] = SULFUR_DIOXIDE;
	extragalacticNames[32] = SULFUR_MONOXIDE;
	extragalacticNames[33] = THIOFORMALDEHYDE;
	extragalacticNames[34] = THIOXOETHENYLIDENE;
	extragalacticNames[35] = WATER;

	planetNames[0] = ACETYLENE;
	planetNames[1] = ATOMIC_CARBON;
	planetNames[2] = ATOMIC_HYDROGEN;
	planetNames[3] = BUTATRIENYLIDENE;
	planetNames[4] = CARBON_MONOXIDE;
	planetNames[5] = CARBONYL_SULFIDE;
	planetNames[6] = CYANOACETYLENE;
	planetNames[7] = FORMYLIUM;
	planetNames[8] = HYDROCHLORIC_ACID;
	planetNames[9] = HYDROGEN_CYANIDE;
	planetNames[10] = HYDROGEN_FLUORIDE;
	planetNames[11] = IMINOMETHYLIUM;
	planetNames[12] = METHANOL;
	planetNames[13] = NITRIC_OXIDE;
	planetNames[14] = NITROUS_OXIDE;
	planetNames[15] = SODIUM_CHLORIDE;
	planetNames[16] = SULFUR_DIOXIDE;
	planetNames[17] = SULFUR_MONOXIDE;
	planetNames[18] = WATER;

	hotCoreNames[0] = ACETALDEHYDE;
	hotCoreNames[1] = ACETONE;
	hotCoreNames[2] = AMINOACETONITRILE;
	hotCoreNames[3] = AMMONIA;
	hotCoreNames[4] = ATOMIC_CARBON;
	hotCoreNames[5] = ATOMIC_HYDROGEN;
	hotCoreNames[6] = BUTADIYNYL13_RADICAL;
	hotCoreNames[7] = BUTYNENITRILE2;
	hotCoreNames[8] = CARBON_MONOSULFIDE;
	hotCoreNames[9] = CARBON_MONOXIDE;
	hotCoreNames[10] = CARBONYL_SULFIDE;
	hotCoreNames[11] = CYANIC_ACID;
	hotCoreNames[12] = CYANIDE_RADICAL;
	hotCoreNames[13] = CYANOACETYLENE;
	hotCoreNames[14] = CYANOBUTADIYNE;
	hotCoreNames[15] = CYANOFORMALDEHYDE;
	hotCoreNames[16] = CYANOMETHYL;
	hotCoreNames[17] = CYANOMETHYLENE;
	hotCoreNames[18] = CYCLOPROPENONE;
	hotCoreNames[19] = CYCLOPROPENYLIDENE;
	hotCoreNames[20] = DIMETHYL_ETHER;
	hotCoreNames[21] = DIAZENYLIUM;
	hotCoreNames[22] = ETHYNYL;
	hotCoreNames[23] = ETHYL_CYANIDE;
	hotCoreNames[24] = ETHYLENE_GLYCOL;
	hotCoreNames[25] = ETHYLENE_OXIDE;
	hotCoreNames[26] = ETHYNE_ISOCYANIDE;
	hotCoreNames[27] = FLUOROMETHYLIUMYLIDENE;
	hotCoreNames[28] = FORMALDEHYDE;
	hotCoreNames[29] = FORMAMIDE;
	hotCoreNames[30] = FORMIC_ACID;
	hotCoreNames[31] = FORMYL_RADICAL;
	hotCoreNames[32] = FORMYLIUM;
	hotCoreNames[33] = FULMINIC_ACID;
	hotCoreNames[34] = GAUCHE_ETHANOL;
	hotCoreNames[35] = GLYCOLALDEHYDE;
	hotCoreNames[36] = HEPTATRIYNENITRILE246;
	hotCoreNames[37] = HYDROCHLORIC_ACID;
	hotCoreNames[38] = HYDROGEN_CYANIDE;
	hotCoreNames[39] = HYDROGEN_ISOCYANIDE;
	hotCoreNames[40] = HYDROGEN_SULFIDE;
	hotCoreNames[41] = HYDROXYL;
	hotCoreNames[42] = HYDROXYMETHYLIUM_ION;
	hotCoreNames[43] = IMINOMETHYLIUM;
	hotCoreNames[44] = ISOCYANIC_ACID;
	hotCoreNames[45] = ISOTHIOCYANIC_ACID;
	hotCoreNames[46] = KETENE;
	hotCoreNames[47] = KETENIMINE;
	hotCoreNames[48] = METHANIMINE;
	hotCoreNames[49] = METHANOL;
	hotCoreNames[50] = METHYLAMINE;
	hotCoreNames[51] = METHYL_ACETYLENE;
	hotCoreNames[52] = METHYL_CYANIDE;
	hotCoreNames[53] = METHYL_FORMATE;
	hotCoreNames[54] = METHYL_ISOCYANIDE;
	hotCoreNames[55] = METHYLENE;
	hotCoreNames[56] = METHYLIDYNE;
	hotCoreNames[57] = METHYLIUMYLIDENE;
	hotCoreNames[58] = NITROSYL_HYDRIDE;
	hotCoreNames[59] = NITROUS_OXIDE;
	hotCoreNames[60] = NITRIC_OXIDE;
	hotCoreNames[61] = NITRIC_SULFIDE;
	hotCoreNames[62] = OXONIUM_HYDRIDE;
	hotCoreNames[63] = PHOSPHOROUS_NITRIDE;
	hotCoreNames[64] = PROPADIENYLIDENE;
	hotCoreNames[65] = PROPENAL;
	hotCoreNames[66] = PROPYNAL2;
	hotCoreNames[67] = PROTONATED_CARBON_DIOXIDE;
	hotCoreNames[68] = PROTONATE_2_PROYNENITRILE;
	hotCoreNames[69] = SILICON_MONOXIDE;
	hotCoreNames[70] = SULFUR_DIOXIDE;
	hotCoreNames[71] = SULFUR_MONOXIDE;
	hotCoreNames[72] = SULFUR_MONOXIDE_ION;
	hotCoreNames[73] = THIOCYANIC_ACID;
	hotCoreNames[74] = THIOFORMALDEHYDE;
	hotCoreNames[75] = THIOXOETHENYLIDENE;
	hotCoreNames[76] = THIOXO_12_PROPADIEYLIDENE3;
	hotCoreNames[77] = TRIHYDROGEN_ION;
	hotCoreNames[78] = VINYL_ALCOHOL;
	hotCoreNames[79] = VINYL_CYANIDE;
	hotCoreNames[80] = WATER;

	darkCloudNames[0] = AMMONIA;
	darkCloudNames[1] = ATOMIC_CARBON;
	darkCloudNames[2] = ATOMIC_HYDROGEN;
	darkCloudNames[3] = CARBON_MONOSULFIDE;
	darkCloudNames[4] = CARBON_MONOXIDE;
	darkCloudNames[5] = CYANIDE_RADICAL;
	darkCloudNames[6] = CYANOACETYLENE;
	darkCloudNames[7] = CYANOALLENE;
	darkCloudNames[8] = CYANOBUTADIYNE;
	darkCloudNames[9] = CYANOMETHYL;
	darkCloudNames[10] = CYANO4_BUTADIYNYL13;
	darkCloudNames[11] = CYCLOPROPENYLIDENE;
	darkCloudNames[12] = DIAZENYLIUM;
	darkCloudNames[13] = ETHYL_CYANIDE;
	darkCloudNames[14] = FORMALDEHYDE;
	darkCloudNames[15] = FORMIC_ACID;
	darkCloudNames[16] = FORMYLIUM;
	darkCloudNames[17] = HEXATRIENYL;
	darkCloudNames[18] = HEPTATRIYNENITRILE246;
	darkCloudNames[19] = HEXATRIYNYL135_ANION;
	darkCloudNames[20] = HYDROCHLORIC_ACID;
	darkCloudNames[21] = HYDROGEN_CYANIDE;
	darkCloudNames[22] = HYDROGEN_ISOCYANIDE;
	darkCloudNames[23] = HYDROGEN_SULFIDE;
	darkCloudNames[24] = HYDROXYL;
	darkCloudNames[25] = KETENE;
	darkCloudNames[26] = METHANOL;
	darkCloudNames[27] = METHYL_ACETYLENE;
	darkCloudNames[28] = METHYL_CYANIDE;
	darkCloudNames[29] = METHYLCYANODIACETYLENE;
	darkCloudNames[30] = METHYL_DIACETYLENE;
	darkCloudNames[31] = METHYLAMINE;
	darkCloudNames[32] = METHYLIDYNE;
	darkCloudNames[33] = METHYLENE_AMIDOGEN;
	darkCloudNames[34] = METHYLIUMYLIDENE;
	darkCloudNames[35] = METHYLTRIACETYLENE;
	darkCloudNames[36] = NITRIC_OXIDE;
	darkCloudNames[37] = NITRIC_SULFIDE;
	darkCloudNames[38] = NONATETRAYNENITRILE2468;
	darkCloudNames[39] = OCTATETRAYNYL1357;
	darkCloudNames[40] = OXOETHENYLIDENE;
	darkCloudNames[41] = OXONIUM_HYDRIDE;
	darkCloudNames[42] = SILICON_MONOXIDE;
	darkCloudNames[43] = SULFUR_MONOXIDE;
	darkCloudNames[44] = THIOFORMALDEHYDE;
	darkCloudNames[45] = THIOXOETHENYLIDENE;
	darkCloudNames[46] = THIOXO_12_PROPADIEYLIDENE3;
	darkCloudNames[47] = TRIHYDROGEN_ION;
	darkCloudNames[48] = UNDECAPENTAYNENITRILE246810;
	darkCloudNames[49] = VINYL_CYANIDE;
	darkCloudNames[50] = WATER;

	diffuseCloudNames[0] = ATOMIC_CARBON;
	diffuseCloudNames[1] = ATOMIC_HYDROGEN;
	diffuseCloudNames[2] = CARBON_MONOSULFIDE;
	diffuseCloudNames[3] = CARBON_MONOXIDE;
	diffuseCloudNames[4] = CYANIDE_RADICAL;
	diffuseCloudNames[5] = CYCLOPROPENYLIDENE;
	diffuseCloudNames[6] = FORMALDEHYDE;
	diffuseCloudNames[7] = FORMYLIUM;
	diffuseCloudNames[8] = HYDROGEN_CYANIDE;
	diffuseCloudNames[9] = HYDROGEN_ISOCYANIDE;
	diffuseCloudNames[10] = HYDROGEN_SULFIDE;
	diffuseCloudNames[11] = HYDROXYL;
	diffuseCloudNames[12] = IMIDOGEN;
	diffuseCloudNames[13] = METHYLIDYNE;
	diffuseCloudNames[14] = METHYLIUMYLIDENE;
	diffuseCloudNames[15] = NITRIC_OXIDE;
	diffuseCloudNames[16] = SILICON_MONOXIDE;
	diffuseCloudNames[17] = SULFUR_DIOXIDE;
	diffuseCloudNames[18] = THIOFORMYLIUM;
	diffuseCloudNames[19] = WATER;

	cometNames[0] = ACETALDEHYDE;
	cometNames[1] = AMIDOGEN;
	cometNames[2] = AMMONIA;
	cometNames[3] = ATOMIC_HYDROGEN;
	cometNames[4] = CARBON_MONOXIDE;
	cometNames[5] = CARBON_MONOXIDE_ION;
	cometNames[6] = CARBONYL_SULFIDE;
	cometNames[7] = CYANIDE_RADICAL;
	cometNames[8] = CYANOACETYLENE;
	cometNames[9] = CYCLOPROPENYLIDENE;
	cometNames[10] = ETHYLENE_GLYCOL;
	cometNames[11] = FORMALDEHYDE;
	cometNames[12] = FORMIC_ACID;
	cometNames[13] = FORMYLIUM;
	cometNames[14] = HYDROGEN_CYANIDE;
	cometNames[15] = HYDROGEN_ISOCYANIDE;
	cometNames[16] = HYDROGEN_SULFIDE;
	cometNames[17] = HYDROXYL;
	cometNames[18] = IMIDOGEN;
	cometNames[19] = ISOCYANIC_ACID;
	cometNames[20] = METHANOL;
	cometNames[21] = METHYL_CYANIDE;
	cometNames[22] = METHYLIDYNE;
	cometNames[23] = METHYLIUMYLIDENE;
	cometNames[24] = NITRIC_OXIDE;
	cometNames[25] = NITRIC_SULFIDE;
	cometNames[26] = SULFUR_DIOXIDE;
	cometNames[27] = SULFUR_MONOXIDE_ION;
	cometNames[28] = THIOFORMALDEHYDE;
	cometNames[29] = WATER;

	agbPpnPnNames[0] = ATOMIC_CARBON;
	agbPpnPnNames[1] = ATOMIC_HYDROGEN;
	agbPpnPnNames[2] = ALUMINUM_ISOCYANITRILE;
	agbPpnPnNames[3] = ALUMINUM_MONOCHLORIDE;
	agbPpnPnNames[4] = ALUMINUM_MONOFLUORIDE;
	agbPpnPnNames[5] = BUTADIYNYL13_ANION;
	agbPpnPnNames[6] = BUTADIYNYL13_RADICAL;
	agbPpnPnNames[7] = BUTATRIENYLIDENE;
	agbPpnPnNames[8] = CARBON_MONOPHOSPHIDE;
	agbPpnPnNames[9] = CARBON_MONOSULFIDE;
	agbPpnPnNames[10] = CARBON_MONOXIDE;
	agbPpnPnNames[11] = CARBON_MONOXIDE_ION;
	agbPpnPnNames[12] = CYANIDE_RADICAL;
	agbPpnPnNames[13] = CYANOACETYLENE;
	agbPpnPnNames[14] = CYANOBUTADIYNE;
	agbPpnPnNames[15] = CYANOBUTADIYNYLIDE_ANION;
	agbPpnPnNames[16] = CYANO3_PROPADIENYLIDENE12;
	agbPpnPnNames[17] = CYANO4_BUTADIYNYL13;
	agbPpnPnNames[18] = CYANOMETHYLENE;
	agbPpnPnNames[19] = CYANOETHYNYLIDE_ION;
	agbPpnPnNames[20] = CYCLOPROPENYLIDENE;
	agbPpnPnNames[21] = ETHYNE_ISOCYANIDE;
	agbPpnPnNames[22] = ETHYNYL;
	agbPpnPnNames[23] = FORMALDEHYDE;
	agbPpnPnNames[24] = FORMYLIUM;
	agbPpnPnNames[25] = HEPTATRIYNYLIDYNE246;
	agbPpnPnNames[26] = HEXATRIENYL;
	agbPpnPnNames[27] = HEXATRIYNYL135;
	agbPpnPnNames[28] = HEXATRIYNYL135_ANION;
	agbPpnPnNames[29] = HYDROGEN_CYANIDE;
	agbPpnPnNames[30] = HYDROGEN_ISOCYANIDE;
	agbPpnPnNames[31] = HYDROGEN_SULFIDE;
	agbPpnPnNames[32] = HYDROXYL;
	agbPpnPnNames[33] = IMINO3_PROPADIENYLIDENE12;
	agbPpnPnNames[34] = MAGNESIUM_CYANIDE;
	agbPpnPnNames[35] = MAGNESIUM_ISOCYANIDE;
	agbPpnPnNames[36] = METHANIMINE;
	agbPpnPnNames[37] = METHYL_ACETYLENE;
	agbPpnPnNames[38] = METHYL_CYANIDE;
	agbPpnPnNames[39] = METHYLIDYNE;
	agbPpnPnNames[40] = METHYLIUMYLIDENE;
	agbPpnPnNames[41] = NITRIC_SULFIDE;
	agbPpnPnNames[42] = OCTATETRAYNYL1357_ANION;
	agbPpnPnNames[43] = OXO3_PROPADIENYLIDENE12;
	agbPpnPnNames[44] = PENTADIYNYLIDYNE24;
	agbPpnPnNames[45] = PHOSPHAPROPYNYLIDYNE;
	agbPpnPnNames[46] = PHOSPHOROUS_MONOXIDE;
	agbPpnPnNames[47] = PHOSPHOROUS_NITRIDE;
	agbPpnPnNames[48] = POTASSIUM_CHLORIDE;
	agbPpnPnNames[49] = PROPYNYLIDYNE;
	agbPpnPnNames[50] = SILANETETRAYL3_PROPADIENYLIDENE12;
	agbPpnPnNames[51] = SILICON_CARBIDE;
	agbPpnPnNames[52] = SILICON_CYANIDE;
	agbPpnPnNames[53] = SILICON_ISOCYANIDE;
	agbPpnPnNames[54] = SILICON_MONOCARBIDE;
	agbPpnPnNames[55] = SILICON_MONONITRIDE;
	agbPpnPnNames[56] = SILICON_MONOSULFIDE;
	agbPpnPnNames[57] = SILICON_MONOXIDE;
	agbPpnPnNames[58] = SILICON_TETRACARBIDE;
	agbPpnPnNames[59] = SODIUM_CHLORIDE;
	agbPpnPnNames[60] = SODIUM_CYANIDE;
	agbPpnPnNames[61] = SULFUR_DIOXIDE;
	agbPpnPnNames[62] = SULFUR_MONOXIDE;
	agbPpnPnNames[63] = TRIHYDROGEN_ION;
	agbPpnPnNames[64] = THIOFORMALDEHYDE;
	agbPpnPnNames[65] = THIOXOETHENYLIDENE;
	agbPpnPnNames[66] = THIOXO_12_PROPADIEYLIDENE3;
	agbPpnPnNames[67] = VINYL_CYANIDE;
	agbPpnPnNames[68] = WATER;
}

void SearcherTable::setAstroFilterTop20( bool filter ){
	if ( filter ){
		filterList = top20Names;
	}
	else {
		filterList.resize(0);
	}
}

void SearcherTable::setAstroFilterPlanetaryAtmosphere( bool filter ){
	if ( filter ){
		filterList = planetNames;
	}
	else {
		filterList.resize(0);
	}
}
void SearcherTable::setAstroFilterHotCores( bool filter ){
	if ( filter ){
		filterList = hotCoreNames;
	}
	else {
		filterList.resize(0);
	}
}
void SearcherTable::setAstroFilterDarkClouds( bool filter ){
	if ( filter ){
		filterList = darkCloudNames;
	}
	else {
		filterList.resize(0);
	}
}
void SearcherTable::setAstroFilterDiffuseClouds( bool filter ){
	if ( filter ){
		filterList = diffuseCloudNames;
	}
	else {
		filterList.resize(0);
	}
}

void SearcherTable::setAstroFilterComets( bool filter ){
	if ( filter ){
		filterList = cometNames;
	}
	else {
		filterList.resize(0);
	}
}

void SearcherTable::setAstroFilterAgbPpnPn( bool filter ){
	if ( filter ){
		filterList = agbPpnPnNames;
	}
	else {
		filterList.resize(0);
	}
}

void SearcherTable::setAstroFilterExtragalactic( bool filter ){
	if ( filter ){
		filterList = extragalacticNames;
	}
	else {
		filterList.resize(0);
	}
}

vector<SplatResult> SearcherTable::doSearch( string& errorMsg ){
	Vector<String> qns;
	Double intensityLow = -1;
	Double intensityHigh = -1;
	Double smu2Low = -1;
	Double smu2High = -1;
	Double logaLow = -1;
	Double logaHigh = -1;
	Double elLow = -1;
	Double elHigh = -1;
	Double euLow = -1;
	Double euHigh = -1;
	Bool includeRRLs = true;
	Bool onlyRRLs = false;
	Record resultsRecord;
	try {
		SplatalogueTable splatalogueTable( databasePath );
		const SearchEngine searcher( &splatalogueTable, false, "", false );
		if ( filterList.size() > 0 ){
			int start = chemicalNames.size();
			chemicalNames.resize( filterList.size() + chemicalNames.size() );
			int end = start + filterList.size();
			for ( int i = start; i < end; i++ ){
				chemicalNames[i] = filterList[i - start];
			}
		}

		bool recommended = false;
		//Change from std to casa types for the search engine.
		Vector<String> speciesVector (speciesNames.size());
		for ( int i = 0; i < static_cast<int>(speciesNames.size()); i++  ){
			speciesVector[i]= speciesNames[i].c_str();
		}
		Vector<String> chemVector( chemicalNames.size());
		for ( int i = 0; i < static_cast<int>(chemicalNames.size()); i++ ){
			chemVector[i] = chemicalNames[i].c_str();
		}

		SplatalogueTable *resTable = searcher.search( fileName, minValueFreq, maxValueFreq,
					speciesVector, recommended, chemVector, qns, intensityLow, intensityHigh,
					smu2Low, smu2High, logaLow, logaHigh,
					elLow, elHigh, euLow, euHigh,
					includeRRLs, onlyRRLs );
		resultsRecord = resTable->toRecord();


		//Clean up
		filterList.resize( 0 );
		delete resTable;
	}
	catch( AipsError& err ){
		errorMsg = "Search was unsuccessful.\nPlease check that a valid local database has been specified.";
	}
	return toSplatResult(resultsRecord);
}

vector<SplatResult> SearcherTable::toSplatResult( const Record& record ){
	int resultCount = record.size();
	vector<SplatResult> results;
	for ( int i = 0; i < resultCount; i++ ){
		Record line = record.asRecord("*" + String::toString(i) );

		//Species
		String species = line.asString( SplatalogueTable::RECORD_SPECIES );

		//Chemical Name
		String chemName = line.asString( SplatalogueTable::RECORD_CHEMNAME );

		//Frequency
		double freq = line.asDouble( SplatalogueTable::RECORD_FREQUENCY );
		pair<double,string> freqResult( freq, "GHz");

		//QNS
		String qns = line.asString( SplatalogueTable::RECORD_QNS );

		//Intensity
		float intensity = line.asFloat( SplatalogueTable::RECORD_QNS );
		pair<double,string> emptyPair;
		SplatResult result( -1, species.c_str(), chemName.c_str(),
					qns.c_str(), "", false , -1, freqResult,
					emptyPair, emptyPair, emptyPair, -1, -1, intensity );
		results.push_back(result);
	}
	return results;
}

void SearcherTable::setChemicalNames( const vector<string>& chemNames ){
	chemicalNames = chemNames;
}

void SearcherTable::setSpeciesNames( const vector<string>& species ){
	speciesNames = species;
}

void SearcherTable::setResultFile( const string& name ){
	fileName = name;
}

void SearcherTable::setSearchRangeFrequency( double minVal, double maxVal ){
	minValueFreq = minVal;
	maxValueFreq = maxVal;
}

void SearcherTable::setDatabasePath( const string& path ){
	databasePath = path;
}
SearcherTable::~SearcherTable() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
