
/*
 * ALMA - Atacama Large Millimeter Array
 * (c) European Southern Observatory, 2002
 * (c) Associated Universities Inc., 2002
 * Copyright by ESO (in the framework of the ALMA collaboration),
 * Copyright by AUI (in the framework of the ALMA collaboration),
 * All rights reserved.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307  USA
 *
 * Warning!
 *  -------------------------------------------------------------------- 
 * | This is generated code!  Do not modify this file.                  |
 * | If you do, all changes will be lost when the file is re-generated. |
 *  --------------------------------------------------------------------
 *
 * File TagType.cpp
 */
 #include "TagType.h"

namespace asdm {
const TagType* TagType::NoType = new TagType("NoType") ;

	

	
const TagType* TagType:: AlmaRadiometer = new TagType("AlmaRadiometer") ;
	

	
const TagType* TagType:: Annotation = new TagType("Annotation") ;
	

	
const TagType* TagType:: Antenna = new TagType("Antenna") ;
	

	
const TagType* TagType:: Beam = new TagType("Beam") ;
	

	

	

	

	

	
const TagType* TagType:: CalData = new TagType("CalData") ;
	

	

	

	

	

	

	

	

	

	

	

	

	

	
const TagType* TagType:: CalReduction = new TagType("CalReduction") ;
	

	

	

	
const TagType* TagType:: ConfigDescription = new TagType("ConfigDescription") ;
	

	
const TagType* TagType:: CorrelatorMode = new TagType("CorrelatorMode") ;
	

	
const TagType* TagType:: DataDescription = new TagType("DataDescription") ;
	

	
const TagType* TagType:: Doppler = new TagType("Doppler") ;
	

	
const TagType* TagType:: Ephemeris = new TagType("Ephemeris") ;
	

	
const TagType* TagType:: ExecBlock = new TagType("ExecBlock") ;
	

	
const TagType* TagType:: Feed = new TagType("Feed") ;
	

	
const TagType* TagType:: Field = new TagType("Field") ;
	

	

	

	
const TagType* TagType:: FocusModel = new TagType("FocusModel") ;
	

	

	

	

	
const TagType* TagType:: Holography = new TagType("Holography") ;
	

	
const TagType* TagType:: Observation = new TagType("Observation") ;
	

	

	
const TagType* TagType:: PointingModel = new TagType("PointingModel") ;
	

	
const TagType* TagType:: Polarization = new TagType("Polarization") ;
	

	
const TagType* TagType:: Processor = new TagType("Processor") ;
	

	
const TagType* TagType:: Receiver = new TagType("Receiver") ;
	

	

	

	

	
const TagType* TagType:: Source = new TagType("Source") ;
	

	
const TagType* TagType:: SourceParameter = new TagType("SourceParameter") ;
	

	
const TagType* TagType:: SpectralWindow = new TagType("SpectralWindow") ;
	

	
const TagType* TagType:: SquareLawDetector = new TagType("SquareLawDetector") ;
	

	
const TagType* TagType:: State = new TagType("State") ;
	

	
const TagType* TagType:: Station = new TagType("Station") ;
	

	

	
const TagType* TagType:: SwitchCycle = new TagType("SwitchCycle") ;
	

	

	

	

	


TagType::TagTypeMgr TagType::tagTypeMgr;
map<string, const TagType*> TagType::name2TagType;

TagType::TagType(string name) {
  this->name = name;
}

string TagType::toString() const {
  return this->name;
}

const TagType* TagType::getTagType(string name) {
  if ( name2TagType.size() == 0) {
	TagType::name2TagType["NoType"] = TagType::NoType;
 
	

	
	TagType::name2TagType["AlmaRadiometer"] = TagType::AlmaRadiometer ;
	

	
	TagType::name2TagType["Annotation"] = TagType::Annotation ;
	

	
	TagType::name2TagType["Antenna"] = TagType::Antenna ;
	

	
	TagType::name2TagType["Beam"] = TagType::Beam ;
	

	

	

	

	

	
	TagType::name2TagType["CalData"] = TagType::CalData ;
	

	

	

	

	

	

	

	

	

	

	

	

	

	
	TagType::name2TagType["CalReduction"] = TagType::CalReduction ;
	

	

	

	
	TagType::name2TagType["ConfigDescription"] = TagType::ConfigDescription ;
	

	
	TagType::name2TagType["CorrelatorMode"] = TagType::CorrelatorMode ;
	

	
	TagType::name2TagType["DataDescription"] = TagType::DataDescription ;
	

	
	TagType::name2TagType["Doppler"] = TagType::Doppler ;
	

	
	TagType::name2TagType["Ephemeris"] = TagType::Ephemeris ;
	

	
	TagType::name2TagType["ExecBlock"] = TagType::ExecBlock ;
	

	
	TagType::name2TagType["Feed"] = TagType::Feed ;
	

	
	TagType::name2TagType["Field"] = TagType::Field ;
	

	

	

	
	TagType::name2TagType["FocusModel"] = TagType::FocusModel ;
	

	

	

	

	
	TagType::name2TagType["Holography"] = TagType::Holography ;
	

	
	TagType::name2TagType["Observation"] = TagType::Observation ;
	

	

	
	TagType::name2TagType["PointingModel"] = TagType::PointingModel ;
	

	
	TagType::name2TagType["Polarization"] = TagType::Polarization ;
	

	
	TagType::name2TagType["Processor"] = TagType::Processor ;
	

	
	TagType::name2TagType["Receiver"] = TagType::Receiver ;
	

	

	

	

	
	TagType::name2TagType["Source"] = TagType::Source ;
	

	
	TagType::name2TagType["SourceParameter"] = TagType::SourceParameter ;
	

	
	TagType::name2TagType["SpectralWindow"] = TagType::SpectralWindow ;
	

	
	TagType::name2TagType["SquareLawDetector"] = TagType::SquareLawDetector ;
	

	
	TagType::name2TagType["State"] = TagType::State ;
	

	
	TagType::name2TagType["Station"] = TagType::Station ;
	

	

	
	TagType::name2TagType["SwitchCycle"] = TagType::SwitchCycle ;
	

	

	

	

	

  }
  return TagType::name2TagType[name];
} 
} // end namespace asdm
