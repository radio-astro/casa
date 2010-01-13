
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
 * File CalAtmosphereTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <CalAtmosphereTable.h>
#include <CalAtmosphereRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::CalAtmosphereTable;
using asdm::CalAtmosphereRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "boost/filesystem/operations.hpp"


namespace asdm {

	string CalAtmosphereTable::tableName = "CalAtmosphere";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> CalAtmosphereTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> CalAtmosphereTable::getKeyName() {
		return key;
	}


	CalAtmosphereTable::CalAtmosphereTable(ASDM &c) : container(c) {

	
		key.push_back("antennaName");
	
		key.push_back("receiverBand");
	
		key.push_back("calDataId");
	
		key.push_back("calReductionId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("CalAtmosphereTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for CalAtmosphereTable.
 */
 
	CalAtmosphereTable::~CalAtmosphereTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &CalAtmosphereTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	unsigned int CalAtmosphereTable::size() {
		return privateRows.size();
	}
	

	/**
	 * Return the name of this table.
	 */
	string CalAtmosphereTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity CalAtmosphereTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void CalAtmosphereTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	CalAtmosphereRow *CalAtmosphereTable::newRow() {
		return new CalAtmosphereRow (*this);
	}
	

	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param antennaName 
	
 	 * @param receiverBand 
	
 	 * @param calDataId 
	
 	 * @param calReductionId 
	
 	 * @param startValidTime 
	
 	 * @param endValidTime 
	
 	 * @param numFreq 
	
 	 * @param numLoad 
	
 	 * @param numReceptor 
	
 	 * @param forwardEffSpectrum 
	
 	 * @param frequencyRange 
	
 	 * @param groundPressure 
	
 	 * @param groundRelHumidity 
	
 	 * @param frequencySpectrum 
	
 	 * @param groundTemperature 
	
 	 * @param polarizationTypes 
	
 	 * @param powerSkySpectrum 
	
 	 * @param powerLoadSpectrum 
	
 	 * @param syscalType 
	
 	 * @param tAtmSpectrum 
	
 	 * @param tRecSpectrum 
	
 	 * @param tSysSpectrum 
	
 	 * @param tauSpectrum 
	
 	 * @param tAtm 
	
 	 * @param tRec 
	
 	 * @param tSys 
	
 	 * @param tau 
	
 	 * @param water 
	
 	 * @param waterError 
	
     */
	CalAtmosphereRow* CalAtmosphereTable::newRow(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, int numFreq, int numLoad, int numReceptor, vector<vector<float > > forwardEffSpectrum, vector<Frequency > frequencyRange, Pressure groundPressure, Humidity groundRelHumidity, vector<Frequency > frequencySpectrum, Temperature groundTemperature, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<float > > powerSkySpectrum, vector<vector<vector<float > > > powerLoadSpectrum, SyscalMethodMod::SyscalMethod syscalType, vector<vector<Temperature > > tAtmSpectrum, vector<vector<Temperature > > tRecSpectrum, vector<vector<Temperature > > tSysSpectrum, vector<vector<float > > tauSpectrum, vector<Temperature > tAtm, vector<Temperature > tRec, vector<Temperature > tSys, vector<float > tau, vector<Length > water, vector<Length > waterError){
		CalAtmosphereRow *row = new CalAtmosphereRow(*this);
			
		row->setAntennaName(antennaName);
			
		row->setReceiverBand(receiverBand);
			
		row->setCalDataId(calDataId);
			
		row->setCalReductionId(calReductionId);
			
		row->setStartValidTime(startValidTime);
			
		row->setEndValidTime(endValidTime);
			
		row->setNumFreq(numFreq);
			
		row->setNumLoad(numLoad);
			
		row->setNumReceptor(numReceptor);
			
		row->setForwardEffSpectrum(forwardEffSpectrum);
			
		row->setFrequencyRange(frequencyRange);
			
		row->setGroundPressure(groundPressure);
			
		row->setGroundRelHumidity(groundRelHumidity);
			
		row->setFrequencySpectrum(frequencySpectrum);
			
		row->setGroundTemperature(groundTemperature);
			
		row->setPolarizationTypes(polarizationTypes);
			
		row->setPowerSkySpectrum(powerSkySpectrum);
			
		row->setPowerLoadSpectrum(powerLoadSpectrum);
			
		row->setSyscalType(syscalType);
			
		row->setTAtmSpectrum(tAtmSpectrum);
			
		row->setTRecSpectrum(tRecSpectrum);
			
		row->setTSysSpectrum(tSysSpectrum);
			
		row->setTauSpectrum(tauSpectrum);
			
		row->setTAtm(tAtm);
			
		row->setTRec(tRec);
			
		row->setTSys(tSys);
			
		row->setTau(tau);
			
		row->setWater(water);
			
		row->setWaterError(waterError);
	
		return row;		
	}	
	


CalAtmosphereRow* CalAtmosphereTable::newRow(CalAtmosphereRow* row) {
	return new CalAtmosphereRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	 
	/**
	 * Add a row.
	 * @throws DuplicateKey Thrown if the new row has a key that is already in the table.
	 * @param x A pointer to the row to be added.
	 * @return x
	 */
	CalAtmosphereRow* CalAtmosphereTable::add(CalAtmosphereRow* x) {
		
		if (getRowByKey(
						x->getAntennaName()
						,
						x->getReceiverBand()
						,
						x->getCalDataId()
						,
						x->getCalReductionId()
						))
			//throw DuplicateKey(x.getAntennaName() + "|" + x.getReceiverBand() + "|" + x.getCalDataId() + "|" + x.getCalReductionId(),"CalAtmosphere");
			throw DuplicateKey("Duplicate key exception in ","CalAtmosphereTable");
		
		row.push_back(x);
		privateRows.push_back(x);
		x->isAdded();
		return x;
	}

		





	// 
	// A private method to append a row to its table, used by input conversion
	// methods.
	//

	
	/**
	 * If this table has an autoincrementable attribute then check if *x verifies the rule of uniqueness and throw exception if not.
	 * Check if *x verifies the key uniqueness rule and throw an exception if not.
	 * Append x to its table.
	 * @param x a pointer on the row to be appended.
	 * @returns a pointer on x.
	 * @throws DuplicateKey
	 
	 */
	CalAtmosphereRow*  CalAtmosphereTable::checkAndAdd(CalAtmosphereRow* x)  {
		
		
		if (getRowByKey(
	
			x->getAntennaName()
	,
			x->getReceiverBand()
	,
			x->getCalDataId()
	,
			x->getCalReductionId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "CalAtmosphereTable");
		
		row.push_back(x);
		privateRows.push_back(x);
		x->isAdded();
		return x;	
	}	







	

	//
	// ====> Methods returning rows.
	//	
	/**
	 * Get all rows.
	 * @return Alls rows as an array of CalAtmosphereRow
	 */
	vector<CalAtmosphereRow *> CalAtmosphereTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a CalAtmosphereRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	CalAtmosphereRow* CalAtmosphereTable::getRowByKey(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId)  {
	CalAtmosphereRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->antennaName != antennaName) continue;
			
		
			
				if (aRow->receiverBand != receiverBand) continue;
			
		
			
				if (aRow->calDataId != calDataId) continue;
			
		
			
				if (aRow->calReductionId != calReductionId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param antennaName.
 	 		
 * @param receiverBand.
 	 		
 * @param calDataId.
 	 		
 * @param calReductionId.
 	 		
 * @param startValidTime.
 	 		
 * @param endValidTime.
 	 		
 * @param numFreq.
 	 		
 * @param numLoad.
 	 		
 * @param numReceptor.
 	 		
 * @param forwardEffSpectrum.
 	 		
 * @param frequencyRange.
 	 		
 * @param groundPressure.
 	 		
 * @param groundRelHumidity.
 	 		
 * @param frequencySpectrum.
 	 		
 * @param groundTemperature.
 	 		
 * @param polarizationTypes.
 	 		
 * @param powerSkySpectrum.
 	 		
 * @param powerLoadSpectrum.
 	 		
 * @param syscalType.
 	 		
 * @param tAtmSpectrum.
 	 		
 * @param tRecSpectrum.
 	 		
 * @param tSysSpectrum.
 	 		
 * @param tauSpectrum.
 	 		
 * @param tAtm.
 	 		
 * @param tRec.
 	 		
 * @param tSys.
 	 		
 * @param tau.
 	 		
 * @param water.
 	 		
 * @param waterError.
 	 		 
 */
CalAtmosphereRow* CalAtmosphereTable::lookup(string antennaName, ReceiverBandMod::ReceiverBand receiverBand, Tag calDataId, Tag calReductionId, ArrayTime startValidTime, ArrayTime endValidTime, int numFreq, int numLoad, int numReceptor, vector<vector<float > > forwardEffSpectrum, vector<Frequency > frequencyRange, Pressure groundPressure, Humidity groundRelHumidity, vector<Frequency > frequencySpectrum, Temperature groundTemperature, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<float > > powerSkySpectrum, vector<vector<vector<float > > > powerLoadSpectrum, SyscalMethodMod::SyscalMethod syscalType, vector<vector<Temperature > > tAtmSpectrum, vector<vector<Temperature > > tRecSpectrum, vector<vector<Temperature > > tSysSpectrum, vector<vector<float > > tauSpectrum, vector<Temperature > tAtm, vector<Temperature > tRec, vector<Temperature > tSys, vector<float > tau, vector<Length > water, vector<Length > waterError) {
		CalAtmosphereRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(antennaName, receiverBand, calDataId, calReductionId, startValidTime, endValidTime, numFreq, numLoad, numReceptor, forwardEffSpectrum, frequencyRange, groundPressure, groundRelHumidity, frequencySpectrum, groundTemperature, polarizationTypes, powerSkySpectrum, powerLoadSpectrum, syscalType, tAtmSpectrum, tRecSpectrum, tSysSpectrum, tauSpectrum, tAtm, tRec, tSys, tau, water, waterError)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	




#ifndef WITHOUT_ACS
	// Conversion Methods

	CalAtmosphereTableIDL *CalAtmosphereTable::toIDL() {
		CalAtmosphereTableIDL *x = new CalAtmosphereTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<CalAtmosphereRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void CalAtmosphereTable::fromIDL(CalAtmosphereTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			CalAtmosphereRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	
	string CalAtmosphereTable::toXML()  {
		string buf;

		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<CalAtmosphereTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:clatm=\"http://Alma/XASDM/CalAtmosphereTable\" xsi:schemaLocation=\"http://Alma/XASDM/CalAtmosphereTable http://almaobservatory.org/XML/XASDM/2/CalAtmosphereTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.53\">\n");
	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<CalAtmosphereRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</CalAtmosphereTable> ");
		return buf;
	}

	
	void CalAtmosphereTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<CalAtmosphereTable")) 
			error();
		// cout << "Parsing a CalAtmosphereTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "CalAtmosphereTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		CalAtmosphereRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a CalAtmosphereRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"CalAtmosphereTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"CalAtmosphereTable");	
			}
			catch (...) {
				// cout << "Unexpected error in CalAtmosphereTable::checkAndAdd called from CalAtmosphereTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</CalAtmosphereTable>")) 
			error();
			
		archiveAsBin = false;
		fileAsBin = false;
		
	}

	
	void CalAtmosphereTable::error()  {
		throw ConversionException("Invalid xml document","CalAtmosphere");
	}
	
	
	string CalAtmosphereTable::MIMEXMLPart(const asdm::ByteOrder* byteOrder) {
		string UID = getEntity().getEntityId().toString();
		string withoutUID = UID.substr(6);
		string containerUID = getContainer().getEntity().getEntityId().toString();
		ostringstream oss;
		oss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		oss << "\n";
		oss << "<CalAtmosphereTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:clatm=\"http://Alma/XASDM/CalAtmosphereTable\" xsi:schemaLocation=\"http://Alma/XASDM/CalAtmosphereTable http://almaobservatory.org/XML/XASDM/2/CalAtmosphereTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.53\">\n";
		oss<< "<Entity entityId='"<<UID<<"' entityIdEncrypted='na' entityTypeName='CalAtmosphereTable' schemaVersion='1' documentVersion='1'/>\n";
		oss<< "<ContainerEntity entityId='"<<containerUID<<"' entityIdEncrypted='na' entityTypeName='ASDM' schemaVersion='1' documentVersion='1'/>\n";
		oss << "<BulkStoreRef file_id='"<<withoutUID<<"' byteOrder='"<<byteOrder->toString()<<"' />\n";
		oss << "<Attributes>\n";

		oss << "<antennaName/>\n"; 
		oss << "<receiverBand/>\n"; 
		oss << "<calDataId/>\n"; 
		oss << "<calReductionId/>\n"; 
		oss << "<startValidTime/>\n"; 
		oss << "<endValidTime/>\n"; 
		oss << "<numFreq/>\n"; 
		oss << "<numLoad/>\n"; 
		oss << "<numReceptor/>\n"; 
		oss << "<forwardEffSpectrum/>\n"; 
		oss << "<frequencyRange/>\n"; 
		oss << "<groundPressure/>\n"; 
		oss << "<groundRelHumidity/>\n"; 
		oss << "<frequencySpectrum/>\n"; 
		oss << "<groundTemperature/>\n"; 
		oss << "<polarizationTypes/>\n"; 
		oss << "<powerSkySpectrum/>\n"; 
		oss << "<powerLoadSpectrum/>\n"; 
		oss << "<syscalType/>\n"; 
		oss << "<tAtmSpectrum/>\n"; 
		oss << "<tRecSpectrum/>\n"; 
		oss << "<tSysSpectrum/>\n"; 
		oss << "<tauSpectrum/>\n"; 
		oss << "<tAtm/>\n"; 
		oss << "<tRec/>\n"; 
		oss << "<tSys/>\n"; 
		oss << "<tau/>\n"; 
		oss << "<water/>\n"; 
		oss << "<waterError/>\n"; 

		oss << "<alphaSpectrum/>\n"; 
		oss << "<forwardEfficiency/>\n"; 
		oss << "<forwardEfficiencyError/>\n"; 
		oss << "<sbGain/>\n"; 
		oss << "<sbGainError/>\n"; 
		oss << "<sbGainSpectrum/>\n"; 
		oss << "</Attributes>\n";		
		oss << "</CalAtmosphereTable>\n";

		return oss.str();				
	}
	
	string CalAtmosphereTable::toMIME(const asdm::ByteOrder* byteOrder) {
		EndianOSStream eoss(byteOrder);
		
		string UID = getEntity().getEntityId().toString();
		
		// The MIME Header
		eoss <<"MIME-Version: 1.0";
		eoss << "\n";
		eoss << "Content-Type: Multipart/Related; boundary='MIME_boundary'; type='text/xml'; start= '<header.xml>'";
		eoss <<"\n";
		eoss <<"Content-Description: Correlator";
		eoss <<"\n";
		eoss <<"alma-uid:" << UID;
		eoss <<"\n";
		eoss <<"\n";		
		
		// The MIME XML part header.
		eoss <<"--MIME_boundary";
		eoss <<"\n";
		eoss <<"Content-Type: text/xml; charset='ISO-8859-1'";
		eoss <<"\n";
		eoss <<"Content-Transfer-Encoding: 8bit";
		eoss <<"\n";
		eoss <<"Content-ID: <header.xml>";
		eoss <<"\n";
		eoss <<"\n";
		
		// The MIME XML part content.
		eoss << MIMEXMLPart(byteOrder);

		// The MIME binary part header
		eoss <<"--MIME_boundary";
		eoss <<"\n";
		eoss <<"Content-Type: binary/octet-stream";
		eoss <<"\n";
		eoss <<"Content-ID: <content.bin>";
		eoss <<"\n";
		eoss <<"\n";	
		
		// The MIME binary content
		entity.toBin(eoss);
		container.getEntity().toBin(eoss);
		eoss.writeInt((int) privateRows.size());
		for (unsigned int i = 0; i < privateRows.size(); i++) {
			privateRows.at(i)->toBin(eoss);	
		}
		
		// The closing MIME boundary
		eoss << "\n--MIME_boundary--";
		eoss << "\n";
		
		return eoss.str();	
	}

	
	void CalAtmosphereTable::setFromMIME(const string & mimeMsg) {
    string xmlPartMIMEHeader = "Content-ID: <header.xml>\n\n";
    
    string binPartMIMEHeader = "--MIME_boundary\nContent-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
    
    // Detect the XML header.
    string::size_type loc0 = mimeMsg.find(xmlPartMIMEHeader, 0);
    if ( loc0 == string::npos) {
      throw ConversionException("Failed to detect the beginning of the XML header", "CalAtmosphere");
    }
    loc0 += xmlPartMIMEHeader.size();
    
    // Look for the string announcing the binary part.
    string::size_type loc1 = mimeMsg.find( binPartMIMEHeader, loc0 );
    
    if ( loc1 == string::npos ) {
      throw ConversionException("Failed to detect the beginning of the binary part", "CalAtmosphere");
    }
    
    //
    // Extract the xmlHeader and analyze it to find out what is the byte order and the sequence
    // of attribute names.
    //
    string xmlHeader = mimeMsg.substr(loc0, loc1-loc0);
    xmlDoc *doc;
    doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
    if ( doc == NULL ) 
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "CalAtmosphere");
    
   // This vector will be filled by the names of  all the attributes of the table
   // in the order in which they are expected to be found in the binary representation.
   //
    vector<string> attributesSeq;
      
    xmlNode* root_element = xmlDocGetRootElement(doc);
    if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "CalAtmosphere");
    
    const ByteOrder* byteOrder;
    if ( string("ASDMBinaryTable").compare((const char*) root_element->name) == 0) {
      // Then it's an "old fashioned" MIME file for tables.
      // Just try to deserialize it with Big_Endian for the bytes ordering.
      byteOrder = asdm::ByteOrder::Big_Endian;
      
 	 //
    // Let's consider a  default order for the sequence of attributes.
    //
     
    attributesSeq.push_back("antennaName") ; 
     
    attributesSeq.push_back("receiverBand") ; 
     
    attributesSeq.push_back("calDataId") ; 
     
    attributesSeq.push_back("calReductionId") ; 
     
    attributesSeq.push_back("startValidTime") ; 
     
    attributesSeq.push_back("endValidTime") ; 
     
    attributesSeq.push_back("numFreq") ; 
     
    attributesSeq.push_back("numLoad") ; 
     
    attributesSeq.push_back("numReceptor") ; 
     
    attributesSeq.push_back("forwardEffSpectrum") ; 
     
    attributesSeq.push_back("frequencyRange") ; 
     
    attributesSeq.push_back("groundPressure") ; 
     
    attributesSeq.push_back("groundRelHumidity") ; 
     
    attributesSeq.push_back("frequencySpectrum") ; 
     
    attributesSeq.push_back("groundTemperature") ; 
     
    attributesSeq.push_back("polarizationTypes") ; 
     
    attributesSeq.push_back("powerSkySpectrum") ; 
     
    attributesSeq.push_back("powerLoadSpectrum") ; 
     
    attributesSeq.push_back("syscalType") ; 
     
    attributesSeq.push_back("tAtmSpectrum") ; 
     
    attributesSeq.push_back("tRecSpectrum") ; 
     
    attributesSeq.push_back("tSysSpectrum") ; 
     
    attributesSeq.push_back("tauSpectrum") ; 
     
    attributesSeq.push_back("tAtm") ; 
     
    attributesSeq.push_back("tRec") ; 
     
    attributesSeq.push_back("tSys") ; 
     
    attributesSeq.push_back("tau") ; 
     
    attributesSeq.push_back("water") ; 
     
    attributesSeq.push_back("waterError") ; 
    
     
    attributesSeq.push_back("alphaSpectrum") ; 
     
    attributesSeq.push_back("forwardEfficiency") ; 
     
    attributesSeq.push_back("forwardEfficiencyError") ; 
     
    attributesSeq.push_back("sbGain") ; 
     
    attributesSeq.push_back("sbGainError") ; 
     
    attributesSeq.push_back("sbGainSpectrum") ; 
              
     }
    else if (string("CalAtmosphereTable").compare((const char*) root_element->name) == 0) {
      // It's a new (and correct) MIME file for tables.
      //
      // 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
      //
      xmlNode* bulkStoreRef = 0;
      xmlNode* child = root_element->children;
      
      // Skip the two first children (Entity and ContainerEntity).
      bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
      if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
      	throw ConversionException ("Could not find the element '/CalAtmosphereTable/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", "CalAtmosphere");
      	
      // We found BulkStoreRef, now look for its attribute byteOrder.
      _xmlAttr* byteOrderAttr = 0;
      for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	   byteOrderAttr = attr;
	   break;
	 }
      
      if (byteOrderAttr == 0) 
	     throw ConversionException("Could not find the element '/CalAtmosphereTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", "CalAtmosphere");
      
      string byteOrderValue = string((const char*) byteOrderAttr->children->content);
      if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
		throw ConversionException("No valid value retrieved for the element '/CalAtmosphereTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", "CalAtmosphere");
		
	 //
	 // 2nd) Look for the Attributes element and grab the names of the elements it contains.
	 //
	 xmlNode* attributes = bulkStoreRef->next;
     if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
       	throw ConversionException ("Could not find the element '/CalAtmosphereTable/Attributes'. Invalid XML header '"+ xmlHeader + "'.", "CalAtmosphere");
 
 	xmlNode* childOfAttributes = attributes->children;
 	
 	while ( childOfAttributes != 0 && (childOfAttributes->type == XML_ELEMENT_NODE) ) {
 		attributesSeq.push_back(string((const char*) childOfAttributes->name));
 		childOfAttributes = childOfAttributes->next;
    }
    }
    // Create an EndianISStream from the substring containing the binary part.
    EndianISStream eiss(mimeMsg.substr(loc1+binPartMIMEHeader.size()), byteOrder);
    
    entity = Entity::fromBin(eiss);
    
    // We do nothing with that but we have to read it.
    Entity containerEntity = Entity::fromBin(eiss);
    
    int numRows = eiss.readInt();
    try {
      for (int i = 0; i < numRows; i++) {
	CalAtmosphereRow* aRow = CalAtmosphereRow::fromBin(eiss, *this, attributesSeq);
	checkAndAdd(aRow);
      }
    }
    catch (DuplicateKey e) {
      throw ConversionException("Error while writing binary data , the message was "
				+ e.getMessage(), "CalAtmosphere");
    }
    catch (TagFormatException e) {
      throw ConversionException("Error while reading binary data , the message was "
				+ e.getMessage(), "CalAtmosphere");
    }
    archiveAsBin = true;
    fileAsBin = true;
	}

	
	void CalAtmosphereTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}

		string fileName = directory + "/CalAtmosphere.xml";
		ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not open file " + fileName + " to write ", "CalAtmosphere");
		if (fileAsBin) 
			tableout << MIMEXMLPart();
		else
			tableout << toXML() << endl;
		tableout.close();
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not close file " + fileName, "CalAtmosphere");

		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/CalAtmosphere.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "CalAtmosphere");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "CalAtmosphere");
		}
	}

	
	void CalAtmosphereTable::setFromFile(const string& directory) {
    if (boost::filesystem::exists(boost::filesystem::path(directory + "/CalAtmosphere.xml")))
      setFromXMLFile(directory);
    else if (boost::filesystem::exists(boost::filesystem::path(directory + "/CalAtmosphere.bin")))
      setFromMIMEFile(directory);
    else
      throw ConversionException("No file found for the CalAtmosphere table", "CalAtmosphere");
	}			

	
  void CalAtmosphereTable::setFromMIMEFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/CalAtmosphere.bin";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "CalAtmosphere");
    }
    // Read in a stringstream.
    stringstream ss; ss << tablefile.rdbuf();
    
    if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file " + tablePath,"CalAtmosphere");
    }
    
    // And close.
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file " + tablePath,"CalAtmosphere");
    
    setFromMIME(ss.str());
  }	

	
void CalAtmosphereTable::setFromXMLFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/CalAtmosphere.xml";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "CalAtmosphere");
    }
      // Read in a stringstream.
    stringstream ss;
    ss << tablefile.rdbuf();
    
    if  (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file '" + tablePath + "'", "CalAtmosphere");
    }
    
    // And close
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file '" + tablePath + "'", "CalAtmosphere");

    // Let's make a string out of the stringstream content and empty the stringstream.
    string xmlDocument = ss.str(); ss.str("");

    // Let's make a very primitive check to decide
    // whether the XML content represents the table
    // or refers to it via a <BulkStoreRef element.
    if (xmlDocument.find("<BulkStoreRef") != string::npos)
      setFromMIMEFile(directory);
    else
      fromXML(xmlDocument);
  }

	

	

			
	
	

	
} // End namespace asdm
 
