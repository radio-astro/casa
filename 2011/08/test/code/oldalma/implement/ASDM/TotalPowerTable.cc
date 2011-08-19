
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
 * File TotalPowerTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <TotalPowerTable.h>
#include <TotalPowerRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::TotalPowerTable;
using asdm::TotalPowerRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>

#include <Misc.h>

using namespace std;
using namespace asdm;


namespace asdm {

	string TotalPowerTable::tableName = "TotalPower";


	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> TotalPowerTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> TotalPowerTable::getKeyName() {
		return key;
	}


	TotalPowerTable::TotalPowerTable(ASDM &c) : container(c) {

	
		key.push_back("time");
	
		key.push_back("configDescriptionId");
	
		key.push_back("fieldId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("TotalPowerTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
				
		// Archive binary
		archiveAsBin = true;
		
		// File binary
		fileAsBin = true;	
	}
	
/**
 * A destructor for TotalPowerTable.
 */	
	
	TotalPowerTable::~TotalPowerTable() {
		for (unsigned int i = 0; i < privateRows.size(); ++i) {
			delete privateRows[i];
		}
	}


	/**
	 * Container to which this table belongs.
	 */
	ASDM &TotalPowerTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int TotalPowerTable::size() {
		return privateRows.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string TotalPowerTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity TotalPowerTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void TotalPowerTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	TotalPowerRow *TotalPowerTable::newRow() {
		return new TotalPowerRow (*this);
	}

	TotalPowerRow *TotalPowerTable::newRowEmpty() {
		return new TotalPowerRow (*this);
	}

	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param configDescriptionId. 
	
 	 * @param fieldId. 
	
 	 * @param time. 
	
 	 * @param execBlockId. 
	
 	 * @param stateId. 
	
 	 * @param scanNumber. 
	
 	 * @param subscanNumber. 
	
 	 * @param integrationNumber. 
	
 	 * @param uvw. 
	
 	 * @param exposure. 
	
 	 * @param timeCentroid. 
	
 	 * @param floatData. 
	
 	 * @param flagAnt. 
	
 	 * @param flagPol. 
	
 	 * @param flagRow. 
	
 	 * @param interval. 
	
     */
	TotalPowerRow* TotalPowerTable::newRow(Tag configDescriptionId, Tag fieldId, ArrayTime time, Tag execBlockId, vector<Tag>  stateId, int scanNumber, int subscanNumber, int integrationNumber, vector<vector<Length > > uvw, vector<vector<Interval > > exposure, vector<vector<ArrayTime > > timeCentroid, vector<vector<vector<float > > > floatData, vector<int > flagAnt, vector<vector<int > > flagPol, bool flagRow, Interval interval){
		TotalPowerRow *row = new TotalPowerRow(*this);
			
		row->setConfigDescriptionId(configDescriptionId);
			
		row->setFieldId(fieldId);
			
		row->setTime(time);
			
		row->setExecBlockId(execBlockId);
			
		row->setStateId(stateId);
			
		row->setScanNumber(scanNumber);
			
		row->setSubscanNumber(subscanNumber);
			
		row->setIntegrationNumber(integrationNumber);
			
		row->setUvw(uvw);
			
		row->setExposure(exposure);
			
		row->setTimeCentroid(timeCentroid);
			
		row->setFloatData(floatData);
			
		row->setFlagAnt(flagAnt);
			
		row->setFlagPol(flagPol);
			
		row->setFlagRow(flagRow);
			
		row->setInterval(interval);
	
		return row;		
	}	

	TotalPowerRow* TotalPowerTable::newRowFull(Tag configDescriptionId, Tag fieldId, ArrayTime time, Tag execBlockId, vector<Tag>  stateId, int scanNumber, int subscanNumber, int integrationNumber, vector<vector<Length > > uvw, vector<vector<Interval > > exposure, vector<vector<ArrayTime > > timeCentroid, vector<vector<vector<float > > > floatData, vector<int > flagAnt, vector<vector<int > > flagPol, bool flagRow, Interval interval){
		TotalPowerRow *row = new TotalPowerRow(*this);
			
		row->setConfigDescriptionId(configDescriptionId);
			
		row->setFieldId(fieldId);
			
		row->setTime(time);
			
		row->setExecBlockId(execBlockId);
			
		row->setStateId(stateId);
			
		row->setScanNumber(scanNumber);
			
		row->setSubscanNumber(subscanNumber);
			
		row->setIntegrationNumber(integrationNumber);
			
		row->setUvw(uvw);
			
		row->setExposure(exposure);
			
		row->setTimeCentroid(timeCentroid);
			
		row->setFloatData(floatData);
			
		row->setFlagAnt(flagAnt);
			
		row->setFlagPol(flagPol);
			
		row->setFlagRow(flagRow);
			
		row->setInterval(interval);
	
		return row;		
	}	
	
TotalPowerRow* TotalPowerTable::newRow(TotalPowerRow* row) {
	return new TotalPowerRow(*this, *row);
}

TotalPowerRow* TotalPowerTable::newRowCopy(TotalPowerRow* row) {
	return new TotalPowerRow(*this, *row);
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
	TotalPowerRow* TotalPowerTable::add(TotalPowerRow* x) {
		string keystr = Key(x->configDescriptionId, x->fieldId); 
		if (context.find(keystr) == context.end()) {
			vector<TotalPowerRow *> v;
			context[keystr] = v;
		}
		return insertByTime(x, context[keystr]);		
	}


	// 
	// A private method to append a row to its table, used by input conversion
	// methods.
	//

	TotalPowerRow * TotalPowerTable::insertByTime(TotalPowerRow* x, vector<TotalPowerRow *>&row) {
		ArrayTime start = x->getTime();
		
		// Is the vector empty ?
		if (row.size() == 0) {
			row.push_back(x);
			privateRows.push_back(x);
			x->isAdded();
			return x;
		}
		
		// Optimization for the case of insertion by ascending time.
		TotalPowerRow* last = row.at(row.size()-1);
		
		if (start.get() > last->getTime().get()) {
			row.push_back(x);
			privateRows.push_back(x);
			x->isAdded();
			return x;
		}
		
		// Optimization for the case of insertion by descending time.
		TotalPowerRow* first = row.at(0);
		
		if (start.get() < first->getTime().get()) {
			row.insert(row.begin(), x);
			privateRows.push_back(x);
			x->isAdded();
			return x;
		}
		
		// Case where x has to be inserted inside row; let's use a dichotomy
		// method to find the insertion index.		
		int k0 = 0;
		int k1 = row.size() - 1;    	  
		while (k0 != (k1 - 1)) {
			if (start.get() == row.at(k0)->getTime().get()) {
				if (row.at(k0)->equalByRequiredValue(x))
					return row.at(k0);
				else
					throw new DuplicateKey("DuplicateKey exception in ", "TotalPowerTable");	
			}
			else if (start.get() == row.at(k1)->getTime().get()) {
				if (row.at(k1)->equalByRequiredValue(x))
					return row.at(k1);
				else
					throw new DuplicateKey("DuplicateKey exception in ", "TotalPowerTable");	
			}
			else {
				if (start.get() <= row.at((k0+k1)/2)->getTime().get())
					k1 = (k0 + k1) / 2;
				else
					k0 = (k0 + k1) / 2;				
			} 	
		}
		row.insert(row.begin()+(k0+1), x);
		privateRows.push_back(x);
		x->isAdded();
		return x; 						
	}
	
	/**
	 * If this table has an autoincrementable attribute then check if *x verifies the rule of uniqueness and throw exception if not.
	 * Check if *x verifies the key uniqueness rule and throw an exception if not.
	 * Append x to its table.
	 * @param x a pointer on the row to be appended.
	 * @returns a pointer on x.
	 */
	TotalPowerRow*  TotalPowerTable::checkAndAdd(TotalPowerRow* x) throw (DuplicateKey) {
		string keystr = Key(x->configDescriptionId, x->fieldId); 
		if (context.find(keystr) == context.end()) {
			vector<TotalPowerRow *> v;
			context[keystr] = v;
		}
		
		vector<TotalPowerRow*>& found = context.find(keystr)->second;
//		return insertByTime(context[keystr], x);
		return insertByTime(x, found);	
	}	

	//
	// ====> Methods returning rows.
	//	
	/**
	 * Get all rows.
	 * @return Alls rows as an array of TotalPowerRow
	 */
	vector<TotalPowerRow *> TotalPowerTable::get() {
		return privateRows;
	}


	 vector<TotalPowerRow *> *TotalPowerTable::getByContext(Tag configDescriptionId, Tag fieldId) {
	  	string k = Key(configDescriptionId, fieldId);
 
	    if (context.find(k) == context.end()) return 0;
 	   else return &(context[k]);		
	}		
/*
 ** Returns a TotalPowerRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	TotalPowerRow* TotalPowerTable::getRowByKey(Tag configDescriptionId, Tag fieldId, ArrayTime time)  {
 		string keystr = Key(configDescriptionId, fieldId);
 		
 		if (context.find(keystr) == context.end()) return 0;
 		
 		vector<TotalPowerRow* > row = context[keystr];
 		
 		// Is the vector empty...impossible in principle !
		if (row.size() == 0) return 0;
		
		// Only one element in the vector
		if (row.size() == 1) {
			if (time.get() == row.at(0)->getTime().get())
				return row.at(0);
			else
				return 0;	
		}
		
		// Optimizations 
		TotalPowerRow* last = row.at(row.size()-1);		
		if (time.get() > last->getTime().get()) return 0;
		TotalPowerRow* first = row.at(0);
		if (time.get() < first->getTime().get()) return 0;
		
		// More than one row
		// let's use a dichotomy method for the general case..		
		int k0 = 0;
		int k1 = row.size() - 1;    	  
		while (k0 !=  k1 ) {
			if (time.get() == row.at(k0)->getTime().get()) {
				return row.at(k0);
			}
			else if (time.get() == row.at(k1)->getTime().get()) {
				return row.at(k1);
			}
			else {
				if (time.get() <= row.at((k0+k1)/2)->getTime().get())
					k1 = (k0 + k1) / 2;
				else
					k0 = (k0 + k1) / 2;				
			} 	
		}
		return 0; 			
	}
	

	
/**
 * Look up the table for a row whose all attributes 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param configDescriptionId.
 	 		
 * @param fieldId.
 	 		
 * @param time.
 	 		
 * @param execBlockId.
 	 		
 * @param stateId.
 	 		
 * @param scanNumber.
 	 		
 * @param subscanNumber.
 	 		
 * @param integrationNumber.
 	 		
 * @param uvw.
 	 		
 * @param exposure.
 	 		
 * @param timeCentroid.
 	 		
 * @param floatData.
 	 		
 * @param flagAnt.
 	 		
 * @param flagPol.
 	 		
 * @param flagRow.
 	 		
 * @param interval.
 	 		 
 */
TotalPowerRow* TotalPowerTable::lookup(Tag configDescriptionId, Tag fieldId, ArrayTime time, Tag execBlockId, vector<Tag>  stateId, int scanNumber, int subscanNumber, int integrationNumber, vector<vector<Length > > uvw, vector<vector<Interval > > exposure, vector<vector<ArrayTime > > timeCentroid, vector<vector<vector<float > > > floatData, vector<int > flagAnt, vector<vector<int > > flagPol, bool flagRow, Interval interval) {
		TotalPowerRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow =privateRows.at(i); 
			if (aRow->compareNoAutoInc(configDescriptionId, fieldId, time, execBlockId, stateId, scanNumber, subscanNumber, integrationNumber, uvw, exposure, timeCentroid, floatData, flagAnt, flagPol, flagRow, interval)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	





#ifndef WITHOUT_ACS
	// Conversion Methods

	TotalPowerTableIDL *TotalPowerTable::toIDL() {
		TotalPowerTableIDL *x = new TotalPowerTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<TotalPowerRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void TotalPowerTable::fromIDL(TotalPowerTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			TotalPowerRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *TotalPowerTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","TotalPower");
	}

	void TotalPowerTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","TotalPower");
	}

	string TotalPowerTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","TotalPower");
	}

	void TotalPowerTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","TotalPower");
	}

	string TotalPowerTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<TotalPowerTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/TotalPowerTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<TotalPowerTable> ");
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<TotalPowerRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</TotalPowerTable> ");
		return buf;
	}
	
	void TotalPowerTable::fromXML(string xmlDoc) throw(ConversionException) {
		Parser xml(xmlDoc);
		if (!xml.isStr("<TotalPowerTable")) 
			error();
		// cout << "Parsing a TotalPowerTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "TotalPowerTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		TotalPowerRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a TotalPowerRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"TotalPowerTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"TotalPowerTable");	
			}
			catch (...) {
				// cout << "Unexpected error in TotalPowerTable::checkAndAdd called from TotalPowerTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</TotalPowerTable>")) 
			error();
	}

	string TotalPowerTable::toMIME() {		
		EndianOSStream eoss;
		
		string UID = getEntity().getEntityId().toString();
		string execBlockUID = getContainer().getEntity().getEntityId().toString();
		
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
		eoss << "<?xml version='1.0'  encoding='ISO-8859-1'?>";
		eoss << "\n";
		eoss<< "<ASDMBinaryTable  xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'  xsi:noNamespaceSchemaLocation='ASDMBinaryTable.xsd' ID='None'  version='1.0'>\n";
		eoss << "<ExecBlockUID>\n";
		eoss << execBlockUID  << "\n";
		eoss << "</ExecBlockUID>\n";
		eoss << "</ASDMBinaryTable>\n";		

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
	
	void TotalPowerTable::setFromMIME(const string & mimeMsg) {
	 	string terminator = "Content-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
	 	
	 	// Look for the string announcing the binary part.
	 	string::size_type loc = mimeMsg.find( terminator, 0 );
	 	
	 	if ( loc == string::npos ) {
	 		throw ConversionException("Failed to detect the beginning of the binary part", "TotalPower");
	 	}
	 	
	 	// Create an EndianISStream from the substring containing the binary part.
	 	EndianISStream eiss(mimeMsg.substr(loc+terminator.size()));
	 	
	 	entity = Entity::fromBin(eiss);

	 	// We do nothing with that but we have to read it.
	 	Entity containerEntity = Entity::fromBin(eiss);
	 	
	 	int numRows = eiss.readInt();
	 	try {
	 		for (int i = 0; i < numRows; i++) {
	 			TotalPowerRow* aRow = TotalPowerRow::fromBin(eiss, *this);
	 			checkAndAdd(aRow);
	 		}
	 	}
	 	catch (DuplicateKey e) {
	 		throw ConversionException("Error while writing binary data , the message was "
	 					+ e.getMessage(), "TotalPower");
	 	}
		catch (TagFormatException e) {
			throw ConversionException("Error while reading binary data , the message was "
					+ e.getMessage(), "TotalPower");
		}	 		 	
	 }	

	void TotalPowerTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","TotalPower");
	}
	
	void TotalPowerTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/TotalPower.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "TotalPower");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "TotalPower");
		}
		else {
			// write the XML
			string fileName = directory + "/TotalPower.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "TotalPower");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "TotalPower");
		}
	}
	
	void TotalPowerTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/TotalPower.bin";
		else
			tablename = directory + "/TotalPower.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg();
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "TotalPower");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"TotalPower");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"TotalPower");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());			
	}		
	
	string TotalPowerTable:: Key(const Tag& configDescriptionId, const Tag& fieldId) {
		ostringstream result;
		result << configDescriptionId.toString() << "_" << fieldId.toString() ;
		return result.str();
	}
} // End namespace asdm
 
