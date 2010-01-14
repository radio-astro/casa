
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
 * File ExecBlockTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <ExecBlockTable.h>
#include <ExecBlockRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::ExecBlockTable;
using asdm::ExecBlockRow;
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

	string ExecBlockTable::tableName = "ExecBlock";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> ExecBlockTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> ExecBlockTable::getKeyName() {
		return key;
	}


	ExecBlockTable::ExecBlockTable(ASDM &c) : container(c) {

	
		key.push_back("execBlockId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("ExecBlockTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for ExecBlockTable.
 */
 
	ExecBlockTable::~ExecBlockTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &ExecBlockTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */
	unsigned int ExecBlockTable::size() {
		return privateRows.size();
	}
	

	/**
	 * Return the name of this table.
	 */
	string ExecBlockTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity ExecBlockTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void ExecBlockTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	ExecBlockRow *ExecBlockTable::newRow() {
		return new ExecBlockRow (*this);
	}
	

	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param startTime 
	
 	 * @param endTime 
	
 	 * @param execBlockNum 
	
 	 * @param execBlockUID 
	
 	 * @param projectId 
	
 	 * @param configName 
	
 	 * @param telescopeName 
	
 	 * @param observerName 
	
 	 * @param observingLog 
	
 	 * @param sessionReference 
	
 	 * @param sbSummary 
	
 	 * @param schedulerMode 
	
 	 * @param baseRangeMin 
	
 	 * @param baseRangeMax 
	
 	 * @param baseRmsMinor 
	
 	 * @param baseRmsMajor 
	
 	 * @param basePa 
	
 	 * @param siteAltitude 
	
 	 * @param siteLongitude 
	
 	 * @param siteLatitude 
	
 	 * @param aborted 
	
 	 * @param numAntenna 
	
 	 * @param antennaId 
	
 	 * @param sBSummaryId 
	
     */
	ExecBlockRow* ExecBlockTable::newRow(ArrayTime startTime, ArrayTime endTime, int execBlockNum, EntityRef execBlockUID, EntityRef projectId, string configName, string telescopeName, string observerName, string observingLog, string sessionReference, EntityRef sbSummary, string schedulerMode, Length baseRangeMin, Length baseRangeMax, Length baseRmsMinor, Length baseRmsMajor, Angle basePa, Length siteAltitude, Angle siteLongitude, Angle siteLatitude, bool aborted, int numAntenna, vector<Tag>  antennaId, Tag sBSummaryId){
		ExecBlockRow *row = new ExecBlockRow(*this);
			
		row->setStartTime(startTime);
			
		row->setEndTime(endTime);
			
		row->setExecBlockNum(execBlockNum);
			
		row->setExecBlockUID(execBlockUID);
			
		row->setProjectId(projectId);
			
		row->setConfigName(configName);
			
		row->setTelescopeName(telescopeName);
			
		row->setObserverName(observerName);
			
		row->setObservingLog(observingLog);
			
		row->setSessionReference(sessionReference);
			
		row->setSbSummary(sbSummary);
			
		row->setSchedulerMode(schedulerMode);
			
		row->setBaseRangeMin(baseRangeMin);
			
		row->setBaseRangeMax(baseRangeMax);
			
		row->setBaseRmsMinor(baseRmsMinor);
			
		row->setBaseRmsMajor(baseRmsMajor);
			
		row->setBasePa(basePa);
			
		row->setSiteAltitude(siteAltitude);
			
		row->setSiteLongitude(siteLongitude);
			
		row->setSiteLatitude(siteLatitude);
			
		row->setAborted(aborted);
			
		row->setNumAntenna(numAntenna);
			
		row->setAntennaId(antennaId);
			
		row->setSBSummaryId(sBSummaryId);
	
		return row;		
	}	
	


ExecBlockRow* ExecBlockTable::newRow(ExecBlockRow* row) {
	return new ExecBlockRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	 
	
	/** 
 	 * Look up the table for a row whose noautoincrementable attributes are matching their
 	 * homologues in *x.  If a row is found  this row else autoincrement  *x.execBlockId, 
 	 * add x to its table and returns x.
 	 *  
 	 * @returns a pointer on a ExecBlockRow.
 	 * @param x. A pointer on the row to be added.
 	 */ 
 		
			
	ExecBlockRow* ExecBlockTable::add(ExecBlockRow* x) {
			 
		ExecBlockRow* aRow = lookup(
				
		x->getStartTime()
				,
		x->getEndTime()
				,
		x->getExecBlockNum()
				,
		x->getExecBlockUID()
				,
		x->getProjectId()
				,
		x->getConfigName()
				,
		x->getTelescopeName()
				,
		x->getObserverName()
				,
		x->getObservingLog()
				,
		x->getSessionReference()
				,
		x->getSbSummary()
				,
		x->getSchedulerMode()
				,
		x->getBaseRangeMin()
				,
		x->getBaseRangeMax()
				,
		x->getBaseRmsMinor()
				,
		x->getBaseRmsMajor()
				,
		x->getBasePa()
				,
		x->getSiteAltitude()
				,
		x->getSiteLongitude()
				,
		x->getSiteLatitude()
				,
		x->getAborted()
				,
		x->getNumAntenna()
				,
		x->getAntennaId()
				,
		x->getSBSummaryId()
				
		);
		if (aRow) return aRow;
			

			
		// Autoincrement execBlockId
		x->setExecBlockId(Tag(size(), TagType::ExecBlock));
						
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
	 
	 * @throws UniquenessViolationException
	 
	 */
	ExecBlockRow*  ExecBlockTable::checkAndAdd(ExecBlockRow* x)  {
	 
		 
		if (lookup(
			
			x->getStartTime()
		,
			x->getEndTime()
		,
			x->getExecBlockNum()
		,
			x->getExecBlockUID()
		,
			x->getProjectId()
		,
			x->getConfigName()
		,
			x->getTelescopeName()
		,
			x->getObserverName()
		,
			x->getObservingLog()
		,
			x->getSessionReference()
		,
			x->getSbSummary()
		,
			x->getSchedulerMode()
		,
			x->getBaseRangeMin()
		,
			x->getBaseRangeMax()
		,
			x->getBaseRmsMinor()
		,
			x->getBaseRmsMajor()
		,
			x->getBasePa()
		,
			x->getSiteAltitude()
		,
			x->getSiteLongitude()
		,
			x->getSiteLatitude()
		,
			x->getAborted()
		,
			x->getNumAntenna()
		,
			x->getAntennaId()
		,
			x->getSBSummaryId()
		
		)) throw UniquenessViolationException("Uniqueness violation exception in table ExecBlockTable");
		
		
		
		if (getRowByKey(
	
			x->getExecBlockId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "ExecBlockTable");
		
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
	 * @return Alls rows as an array of ExecBlockRow
	 */
	vector<ExecBlockRow *> ExecBlockTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a ExecBlockRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	ExecBlockRow* ExecBlockTable::getRowByKey(Tag execBlockId)  {
	ExecBlockRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->execBlockId != execBlockId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes  except the autoincrementable one 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param startTime.
 	 		
 * @param endTime.
 	 		
 * @param execBlockNum.
 	 		
 * @param execBlockUID.
 	 		
 * @param projectId.
 	 		
 * @param configName.
 	 		
 * @param telescopeName.
 	 		
 * @param observerName.
 	 		
 * @param observingLog.
 	 		
 * @param sessionReference.
 	 		
 * @param sbSummary.
 	 		
 * @param schedulerMode.
 	 		
 * @param baseRangeMin.
 	 		
 * @param baseRangeMax.
 	 		
 * @param baseRmsMinor.
 	 		
 * @param baseRmsMajor.
 	 		
 * @param basePa.
 	 		
 * @param siteAltitude.
 	 		
 * @param siteLongitude.
 	 		
 * @param siteLatitude.
 	 		
 * @param aborted.
 	 		
 * @param numAntenna.
 	 		
 * @param antennaId.
 	 		
 * @param sBSummaryId.
 	 		 
 */
ExecBlockRow* ExecBlockTable::lookup(ArrayTime startTime, ArrayTime endTime, int execBlockNum, EntityRef execBlockUID, EntityRef projectId, string configName, string telescopeName, string observerName, string observingLog, string sessionReference, EntityRef sbSummary, string schedulerMode, Length baseRangeMin, Length baseRangeMax, Length baseRmsMinor, Length baseRmsMajor, Angle basePa, Length siteAltitude, Angle siteLongitude, Angle siteLatitude, bool aborted, int numAntenna, vector<Tag>  antennaId, Tag sBSummaryId) {
		ExecBlockRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(startTime, endTime, execBlockNum, execBlockUID, projectId, configName, telescopeName, observerName, observingLog, sessionReference, sbSummary, schedulerMode, baseRangeMin, baseRangeMax, baseRmsMinor, baseRmsMajor, basePa, siteAltitude, siteLongitude, siteLatitude, aborted, numAntenna, antennaId, sBSummaryId)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	




#ifndef WITHOUT_ACS
	// Conversion Methods

	ExecBlockTableIDL *ExecBlockTable::toIDL() {
		ExecBlockTableIDL *x = new ExecBlockTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<ExecBlockRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void ExecBlockTable::fromIDL(ExecBlockTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			ExecBlockRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	
	string ExecBlockTable::toXML()  {
		string buf;

		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<ExecBlockTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:excblk=\"http://Alma/XASDM/ExecBlockTable\" xsi:schemaLocation=\"http://Alma/XASDM/ExecBlockTable http://almaobservatory.org/XML/XASDM/2/ExecBlockTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.53\">\n");
	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<ExecBlockRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</ExecBlockTable> ");
		return buf;
	}

	
	void ExecBlockTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<ExecBlockTable")) 
			error();
		// cout << "Parsing a ExecBlockTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "ExecBlockTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		ExecBlockRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a ExecBlockRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"ExecBlockTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"ExecBlockTable");	
			}
			catch (...) {
				// cout << "Unexpected error in ExecBlockTable::checkAndAdd called from ExecBlockTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</ExecBlockTable>")) 
			error();
			
		archiveAsBin = false;
		fileAsBin = false;
		
	}

	
	void ExecBlockTable::error()  {
		throw ConversionException("Invalid xml document","ExecBlock");
	}
	
	
	string ExecBlockTable::MIMEXMLPart(const asdm::ByteOrder* byteOrder) {
		string UID = getEntity().getEntityId().toString();
		string withoutUID = UID.substr(6);
		string containerUID = getContainer().getEntity().getEntityId().toString();
		ostringstream oss;
		oss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		oss << "\n";
		oss << "<ExecBlockTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:excblk=\"http://Alma/XASDM/ExecBlockTable\" xsi:schemaLocation=\"http://Alma/XASDM/ExecBlockTable http://almaobservatory.org/XML/XASDM/2/ExecBlockTable.xsd\" schemaVersion=\"2\" schemaRevision=\"1.53\">\n";
		oss<< "<Entity entityId='"<<UID<<"' entityIdEncrypted='na' entityTypeName='ExecBlockTable' schemaVersion='1' documentVersion='1'/>\n";
		oss<< "<ContainerEntity entityId='"<<containerUID<<"' entityIdEncrypted='na' entityTypeName='ASDM' schemaVersion='1' documentVersion='1'/>\n";
		oss << "<BulkStoreRef file_id='"<<withoutUID<<"' byteOrder='"<<byteOrder->toString()<<"' />\n";
		oss << "<Attributes>\n";

		oss << "<execBlockId/>\n"; 
		oss << "<startTime/>\n"; 
		oss << "<endTime/>\n"; 
		oss << "<execBlockNum/>\n"; 
		oss << "<execBlockUID/>\n"; 
		oss << "<projectId/>\n"; 
		oss << "<configName/>\n"; 
		oss << "<telescopeName/>\n"; 
		oss << "<observerName/>\n"; 
		oss << "<observingLog/>\n"; 
		oss << "<sessionReference/>\n"; 
		oss << "<sbSummary/>\n"; 
		oss << "<schedulerMode/>\n"; 
		oss << "<baseRangeMin/>\n"; 
		oss << "<baseRangeMax/>\n"; 
		oss << "<baseRmsMinor/>\n"; 
		oss << "<baseRmsMajor/>\n"; 
		oss << "<basePa/>\n"; 
		oss << "<siteAltitude/>\n"; 
		oss << "<siteLongitude/>\n"; 
		oss << "<siteLatitude/>\n"; 
		oss << "<aborted/>\n"; 
		oss << "<numAntenna/>\n"; 
		oss << "<antennaId/>\n"; 
		oss << "<sBSummaryId/>\n"; 

		oss << "<releaseDate/>\n"; 
		oss << "<flagRow/>\n"; 
		oss << "</Attributes>\n";		
		oss << "</ExecBlockTable>\n";

		return oss.str();				
	}
	
	string ExecBlockTable::toMIME(const asdm::ByteOrder* byteOrder) {
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

	
	void ExecBlockTable::setFromMIME(const string & mimeMsg) {
    string xmlPartMIMEHeader = "Content-ID: <header.xml>\n\n";
    
    string binPartMIMEHeader = "--MIME_boundary\nContent-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
    
    // Detect the XML header.
    string::size_type loc0 = mimeMsg.find(xmlPartMIMEHeader, 0);
    if ( loc0 == string::npos) {
      throw ConversionException("Failed to detect the beginning of the XML header", "ExecBlock");
    }
    loc0 += xmlPartMIMEHeader.size();
    
    // Look for the string announcing the binary part.
    string::size_type loc1 = mimeMsg.find( binPartMIMEHeader, loc0 );
    
    if ( loc1 == string::npos ) {
      throw ConversionException("Failed to detect the beginning of the binary part", "ExecBlock");
    }
    
    //
    // Extract the xmlHeader and analyze it to find out what is the byte order and the sequence
    // of attribute names.
    //
    string xmlHeader = mimeMsg.substr(loc0, loc1-loc0);
    xmlDoc *doc;
    doc = xmlReadMemory(xmlHeader.data(), xmlHeader.size(), "BinaryTableHeader.xml", NULL, XML_PARSE_NOBLANKS);
    if ( doc == NULL ) 
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "ExecBlock");
    
   // This vector will be filled by the names of  all the attributes of the table
   // in the order in which they are expected to be found in the binary representation.
   //
    vector<string> attributesSeq;
      
    xmlNode* root_element = xmlDocGetRootElement(doc);
    if ( root_element == NULL || root_element->type != XML_ELEMENT_NODE )
      throw ConversionException("Failed to parse the xmlHeader into a DOM structure.", "ExecBlock");
    
    const ByteOrder* byteOrder;
    if ( string("ASDMBinaryTable").compare((const char*) root_element->name) == 0) {
      // Then it's an "old fashioned" MIME file for tables.
      // Just try to deserialize it with Big_Endian for the bytes ordering.
      byteOrder = asdm::ByteOrder::Big_Endian;
      
 	 //
    // Let's consider a  default order for the sequence of attributes.
    //
     
    attributesSeq.push_back("execBlockId") ; 
     
    attributesSeq.push_back("startTime") ; 
     
    attributesSeq.push_back("endTime") ; 
     
    attributesSeq.push_back("execBlockNum") ; 
     
    attributesSeq.push_back("execBlockUID") ; 
     
    attributesSeq.push_back("projectId") ; 
     
    attributesSeq.push_back("configName") ; 
     
    attributesSeq.push_back("telescopeName") ; 
     
    attributesSeq.push_back("observerName") ; 
     
    attributesSeq.push_back("observingLog") ; 
     
    attributesSeq.push_back("sessionReference") ; 
     
    attributesSeq.push_back("sbSummary") ; 
     
    attributesSeq.push_back("schedulerMode") ; 
     
    attributesSeq.push_back("baseRangeMin") ; 
     
    attributesSeq.push_back("baseRangeMax") ; 
     
    attributesSeq.push_back("baseRmsMinor") ; 
     
    attributesSeq.push_back("baseRmsMajor") ; 
     
    attributesSeq.push_back("basePa") ; 
     
    attributesSeq.push_back("siteAltitude") ; 
     
    attributesSeq.push_back("siteLongitude") ; 
     
    attributesSeq.push_back("siteLatitude") ; 
     
    attributesSeq.push_back("aborted") ; 
     
    attributesSeq.push_back("numAntenna") ; 
     
    attributesSeq.push_back("antennaId") ; 
     
    attributesSeq.push_back("sBSummaryId") ; 
    
     
    attributesSeq.push_back("releaseDate") ; 
     
    attributesSeq.push_back("flagRow") ; 
              
     }
    else if (string("ExecBlockTable").compare((const char*) root_element->name) == 0) {
      // It's a new (and correct) MIME file for tables.
      //
      // 1st )  Look for a BulkStoreRef element with an attribute byteOrder.
      //
      xmlNode* bulkStoreRef = 0;
      xmlNode* child = root_element->children;
      
      // Skip the two first children (Entity and ContainerEntity).
      bulkStoreRef = (child ==  0) ? 0 : ( (child->next) == 0 ? 0 : child->next->next );
      
      if ( bulkStoreRef == 0 || (bulkStoreRef->type != XML_ELEMENT_NODE)  || (string("BulkStoreRef").compare((const char*) bulkStoreRef->name) != 0))
      	throw ConversionException ("Could not find the element '/ExecBlockTable/BulkStoreRef'. Invalid XML header '"+ xmlHeader + "'.", "ExecBlock");
      	
      // We found BulkStoreRef, now look for its attribute byteOrder.
      _xmlAttr* byteOrderAttr = 0;
      for (struct _xmlAttr* attr = bulkStoreRef->properties; attr; attr = attr->next) 
	  if (string("byteOrder").compare((const char*) attr->name) == 0) {
	   byteOrderAttr = attr;
	   break;
	 }
      
      if (byteOrderAttr == 0) 
	     throw ConversionException("Could not find the element '/ExecBlockTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader +"'.", "ExecBlock");
      
      string byteOrderValue = string((const char*) byteOrderAttr->children->content);
      if (!(byteOrder = asdm::ByteOrder::fromString(byteOrderValue)))
		throw ConversionException("No valid value retrieved for the element '/ExecBlockTable/BulkStoreRef/@byteOrder'. Invalid XML header '" + xmlHeader + "'.", "ExecBlock");
		
	 //
	 // 2nd) Look for the Attributes element and grab the names of the elements it contains.
	 //
	 xmlNode* attributes = bulkStoreRef->next;
     if ( attributes == 0 || (attributes->type != XML_ELEMENT_NODE)  || (string("Attributes").compare((const char*) attributes->name) != 0))	 
       	throw ConversionException ("Could not find the element '/ExecBlockTable/Attributes'. Invalid XML header '"+ xmlHeader + "'.", "ExecBlock");
 
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
	ExecBlockRow* aRow = ExecBlockRow::fromBin(eiss, *this, attributesSeq);
	checkAndAdd(aRow);
      }
    }
    catch (DuplicateKey e) {
      throw ConversionException("Error while writing binary data , the message was "
				+ e.getMessage(), "ExecBlock");
    }
    catch (TagFormatException e) {
      throw ConversionException("Error while reading binary data , the message was "
				+ e.getMessage(), "ExecBlock");
    }
    archiveAsBin = true;
    fileAsBin = true;
	}

	
	void ExecBlockTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}

		string fileName = directory + "/ExecBlock.xml";
		ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not open file " + fileName + " to write ", "ExecBlock");
		if (fileAsBin) 
			tableout << MIMEXMLPart();
		else
			tableout << toXML() << endl;
		tableout.close();
		if (tableout.rdstate() == ostream::failbit)
			throw ConversionException("Could not close file " + fileName, "ExecBlock");

		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/ExecBlock.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "ExecBlock");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "ExecBlock");
		}
	}

	
	void ExecBlockTable::setFromFile(const string& directory) {
    if (boost::filesystem::exists(boost::filesystem::path(directory + "/ExecBlock.xml")))
      setFromXMLFile(directory);
    else if (boost::filesystem::exists(boost::filesystem::path(directory + "/ExecBlock.bin")))
      setFromMIMEFile(directory);
    else
      throw ConversionException("No file found for the ExecBlock table", "ExecBlock");
	}			

	
  void ExecBlockTable::setFromMIMEFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/ExecBlock.bin";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "ExecBlock");
    }
    // Read in a stringstream.
    stringstream ss; ss << tablefile.rdbuf();
    
    if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file " + tablePath,"ExecBlock");
    }
    
    // And close.
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file " + tablePath,"ExecBlock");
    
    setFromMIME(ss.str());
  }	

	
void ExecBlockTable::setFromXMLFile(const string& directory) {
    string tablePath ;
    
    tablePath = directory + "/ExecBlock.xml";
    ifstream tablefile(tablePath.c_str(), ios::in|ios::binary);
    if (!tablefile.is_open()) { 
      throw ConversionException("Could not open file " + tablePath, "ExecBlock");
    }
      // Read in a stringstream.
    stringstream ss;
    ss << tablefile.rdbuf();
    
    if  (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
      throw ConversionException("Error reading file '" + tablePath + "'", "ExecBlock");
    }
    
    // And close
    tablefile.close();
    if (tablefile.rdstate() == istream::failbit)
      throw ConversionException("Could not close file '" + tablePath + "'", "ExecBlock");

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

	

	

			
	
	

	
	void ExecBlockTable::autoIncrement(string key, ExecBlockRow* x) {
		map<string, int>::iterator iter;
		if ((iter=noAutoIncIds.find(key)) == noAutoIncIds.end()) {
			// There is not yet a combination of the non autoinc attributes values in the hashtable
			
			// Initialize  execBlockId to Tag(0).
			x->setExecBlockId(Tag(0,  TagType::ExecBlock));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, 0));			
		} 
		else {
			// There is already a combination of the non autoinc attributes values in the hashtable
			// Increment its value.
			int n = iter->second + 1; 
			
			// Initialize  execBlockId to Tag(n).
			x->setExecBlockId(Tag(n, TagType::ExecBlock));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, n));				
		}		
	}
	
} // End namespace asdm
 
