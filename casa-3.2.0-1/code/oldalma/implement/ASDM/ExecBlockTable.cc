
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
		return row.size();
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
	
	ExecBlockRow *ExecBlockTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param antennaId. 
	
 	 * @param execBlockNum. 
	
 	 * @param telescopeName. 
	
 	 * @param configName. 
	
 	 * @param numAntenna. 
	
 	 * @param baseRangeMin. 
	
 	 * @param baseRangeMax. 
	
 	 * @param baseRmsMinor. 
	
 	 * @param baseRmsMajor. 
	
 	 * @param basePa. 
	
 	 * @param timeInterval. 
	
 	 * @param observerName. 
	
 	 * @param observingLog. 
	
 	 * @param schedulerMode. 
	
 	 * @param projectId. 
	
 	 * @param siteLongitude. 
	
 	 * @param siteLatitude. 
	
 	 * @param siteAltitude. 
	
 	 * @param flagRow. 
	
 	 * @param execBlockUID. 
	
 	 * @param aborted. 
	
     */
	ExecBlockRow* ExecBlockTable::newRow(vector<Tag>  antennaId, int execBlockNum, string telescopeName, string configName, int numAntenna, Length baseRangeMin, Length baseRangeMax, Length baseRmsMinor, Length baseRmsMajor, Angle basePa, ArrayTimeInterval timeInterval, string observerName, vector<string > observingLog, vector<string > schedulerMode, EntityRef projectId, Angle siteLongitude, Angle siteLatitude, Length siteAltitude, bool flagRow, EntityRef execBlockUID, bool aborted){
		ExecBlockRow *row = new ExecBlockRow(*this);
			
		row->setAntennaId(antennaId);
			
		row->setExecBlockNum(execBlockNum);
			
		row->setTelescopeName(telescopeName);
			
		row->setConfigName(configName);
			
		row->setNumAntenna(numAntenna);
			
		row->setBaseRangeMin(baseRangeMin);
			
		row->setBaseRangeMax(baseRangeMax);
			
		row->setBaseRmsMinor(baseRmsMinor);
			
		row->setBaseRmsMajor(baseRmsMajor);
			
		row->setBasePa(basePa);
			
		row->setTimeInterval(timeInterval);
			
		row->setObserverName(observerName);
			
		row->setObservingLog(observingLog);
			
		row->setSchedulerMode(schedulerMode);
			
		row->setProjectId(projectId);
			
		row->setSiteLongitude(siteLongitude);
			
		row->setSiteLatitude(siteLatitude);
			
		row->setSiteAltitude(siteAltitude);
			
		row->setFlagRow(flagRow);
			
		row->setExecBlockUID(execBlockUID);
			
		row->setAborted(aborted);
	
		return row;		
	}	

	ExecBlockRow* ExecBlockTable::newRowFull(vector<Tag>  antennaId, int execBlockNum, string telescopeName, string configName, int numAntenna, Length baseRangeMin, Length baseRangeMax, Length baseRmsMinor, Length baseRmsMajor, Angle basePa, ArrayTimeInterval timeInterval, string observerName, vector<string > observingLog, vector<string > schedulerMode, EntityRef projectId, Angle siteLongitude, Angle siteLatitude, Length siteAltitude, bool flagRow, EntityRef execBlockUID, bool aborted)	{
		ExecBlockRow *row = new ExecBlockRow(*this);
			
		row->setAntennaId(antennaId);
			
		row->setExecBlockNum(execBlockNum);
			
		row->setTelescopeName(telescopeName);
			
		row->setConfigName(configName);
			
		row->setNumAntenna(numAntenna);
			
		row->setBaseRangeMin(baseRangeMin);
			
		row->setBaseRangeMax(baseRangeMax);
			
		row->setBaseRmsMinor(baseRmsMinor);
			
		row->setBaseRmsMajor(baseRmsMajor);
			
		row->setBasePa(basePa);
			
		row->setTimeInterval(timeInterval);
			
		row->setObserverName(observerName);
			
		row->setObservingLog(observingLog);
			
		row->setSchedulerMode(schedulerMode);
			
		row->setProjectId(projectId);
			
		row->setSiteLongitude(siteLongitude);
			
		row->setSiteLatitude(siteLatitude);
			
		row->setSiteAltitude(siteAltitude);
			
		row->setFlagRow(flagRow);
			
		row->setExecBlockUID(execBlockUID);
			
		row->setAborted(aborted);
	
		return row;				
	}
	


ExecBlockRow* ExecBlockTable::newRow(ExecBlockRow* row) {
	return new ExecBlockRow(*this, *row);
}

ExecBlockRow* ExecBlockTable::newRowCopy(ExecBlockRow* row) {
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
				
		x->getAntennaId()
				,
		x->getExecBlockNum()
				,
		x->getTelescopeName()
				,
		x->getConfigName()
				,
		x->getNumAntenna()
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
		x->getTimeInterval()
				,
		x->getObserverName()
				,
		x->getObservingLog()
				,
		x->getSchedulerMode()
				,
		x->getProjectId()
				,
		x->getSiteLongitude()
				,
		x->getSiteLatitude()
				,
		x->getSiteAltitude()
				,
		x->getFlagRow()
				,
		x->getExecBlockUID()
				,
		x->getAborted()
				
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
	 */
	ExecBlockRow*  ExecBlockTable::checkAndAdd(ExecBlockRow* x) throw (DuplicateKey, UniquenessViolationException) {
	 
		 
		if (lookup(
			
			x->getAntennaId()
		,
			x->getExecBlockNum()
		,
			x->getTelescopeName()
		,
			x->getConfigName()
		,
			x->getNumAntenna()
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
			x->getTimeInterval()
		,
			x->getObserverName()
		,
			x->getObservingLog()
		,
			x->getSchedulerMode()
		,
			x->getProjectId()
		,
			x->getSiteLongitude()
		,
			x->getSiteLatitude()
		,
			x->getSiteAltitude()
		,
			x->getFlagRow()
		,
			x->getExecBlockUID()
		,
			x->getAborted()
		
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
			
 * @param antennaId.
 	 		
 * @param execBlockNum.
 	 		
 * @param telescopeName.
 	 		
 * @param configName.
 	 		
 * @param numAntenna.
 	 		
 * @param baseRangeMin.
 	 		
 * @param baseRangeMax.
 	 		
 * @param baseRmsMinor.
 	 		
 * @param baseRmsMajor.
 	 		
 * @param basePa.
 	 		
 * @param timeInterval.
 	 		
 * @param observerName.
 	 		
 * @param observingLog.
 	 		
 * @param schedulerMode.
 	 		
 * @param projectId.
 	 		
 * @param siteLongitude.
 	 		
 * @param siteLatitude.
 	 		
 * @param siteAltitude.
 	 		
 * @param flagRow.
 	 		
 * @param execBlockUID.
 	 		
 * @param aborted.
 	 		 
 */
ExecBlockRow* ExecBlockTable::lookup(vector<Tag>  antennaId, int execBlockNum, string telescopeName, string configName, int numAntenna, Length baseRangeMin, Length baseRangeMax, Length baseRmsMinor, Length baseRmsMajor, Angle basePa, ArrayTimeInterval timeInterval, string observerName, vector<string > observingLog, vector<string > schedulerMode, EntityRef projectId, Angle siteLongitude, Angle siteLatitude, Length siteAltitude, bool flagRow, EntityRef execBlockUID, bool aborted) {
		ExecBlockRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(antennaId, execBlockNum, telescopeName, configName, numAntenna, baseRangeMin, baseRangeMax, baseRmsMinor, baseRmsMajor, basePa, timeInterval, observerName, observingLog, schedulerMode, projectId, siteLongitude, siteLatitude, siteAltitude, flagRow, execBlockUID, aborted)) return aRow;
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
	void ExecBlockTable::fromIDL(ExecBlockTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			ExecBlockRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *ExecBlockTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","ExecBlock");
	}

	void ExecBlockTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","ExecBlock");
	}

	string ExecBlockTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","ExecBlock");
	}

	void ExecBlockTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","ExecBlock");
	}

	string ExecBlockTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<ExecBlockTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/ExecBlockTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<ExecBlockTable> ");
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
	
	void ExecBlockTable::fromXML(string xmlDoc) throw(ConversionException) {
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
	}

	void ExecBlockTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","ExecBlock");
	}
	
	string ExecBlockTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void ExecBlockTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
	}
	
	
	void ExecBlockTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
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
		else {
			// write the XML
			string fileName = directory + "/ExecBlock.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "ExecBlock");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "ExecBlock");
		}
	}
	
	void ExecBlockTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/ExecBlock.bin";
		else
			tablename = directory + "/ExecBlock.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "ExecBlock");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"ExecBlock");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"ExecBlock");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
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
 
