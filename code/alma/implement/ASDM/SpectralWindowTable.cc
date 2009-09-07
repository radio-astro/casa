
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
 * File SpectralWindowTable.cpp
 */
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <OutOfBoundsException.h>

using asdm::ConversionException;
using asdm::DuplicateKey;
using asdm::OutOfBoundsException;

#include <ASDM.h>
#include <SpectralWindowTable.h>
#include <SpectralWindowRow.h>
#include <Parser.h>

using asdm::ASDM;
using asdm::SpectralWindowTable;
using asdm::SpectralWindowRow;
using asdm::Parser;

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <Misc.h>
using namespace asdm;


namespace asdm {

	string SpectralWindowTable::tableName = "SpectralWindow";
	

	/**
	 * The list of field names that make up key key.
	 * (Initialization is in the constructor.)
	 */
	vector<string> SpectralWindowTable::key;

	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 */	
	vector<string> SpectralWindowTable::getKeyName() {
		return key;
	}


	SpectralWindowTable::SpectralWindowTable(ASDM &c) : container(c) {

	
		key.push_back("spectralWindowId");
	


		// Define a default entity.
		entity.setEntityId(EntityId("uid://X0/X0/X0"));
		entity.setEntityIdEncrypted("na");
		entity.setEntityTypeName("SpectralWindowTable");
		entity.setEntityVersion("1");
		entity.setInstanceVersion("1");
		
		// Archive XML
		archiveAsBin = false;
		
		// File XML
		fileAsBin = false;
	}
	
/**
 * A destructor for SpectralWindowTable.
 */
 
	SpectralWindowTable::~SpectralWindowTable() {
		for (unsigned int i = 0; i < privateRows.size(); i++) 
			delete(privateRows.at(i));
	}

	/**
	 * Container to which this table belongs.
	 */
	ASDM &SpectralWindowTable::getContainer() const {
		return container;
	}

	/**
	 * Return the number of rows in the table.
	 */

	unsigned int SpectralWindowTable::size() {
		return row.size();
	}	
	
	
	/**
	 * Return the name of this table.
	 */
	string SpectralWindowTable::getName() const {
		return tableName;
	}

	/**
	 * Return this table's Entity.
	 */
	Entity SpectralWindowTable::getEntity() const {
		return entity;
	}

	/**
	 * Set this table's Entity.
	 */
	void SpectralWindowTable::setEntity(Entity e) {
		this->entity = e; 
	}
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row.
	 */
	SpectralWindowRow *SpectralWindowTable::newRow() {
		return new SpectralWindowRow (*this);
	}
	
	SpectralWindowRow *SpectralWindowTable::newRowEmpty() {
		return newRow ();
	}


	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param basebandName. 
	
 	 * @param netSideband. 
	
 	 * @param numChan. 
	
 	 * @param refFreq. 
	
 	 * @param sidebandProcessingMode. 
	
 	 * @param totBandwidth. 
	
 	 * @param windowFunction. 
	
     */
	SpectralWindowRow* SpectralWindowTable::newRow(BasebandNameMod::BasebandName basebandName, NetSidebandMod::NetSideband netSideband, int numChan, Frequency refFreq, SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode, Frequency totBandwidth, WindowFunctionMod::WindowFunction windowFunction){
		SpectralWindowRow *row = new SpectralWindowRow(*this);
			
		row->setBasebandName(basebandName);
			
		row->setNetSideband(netSideband);
			
		row->setNumChan(numChan);
			
		row->setRefFreq(refFreq);
			
		row->setSidebandProcessingMode(sidebandProcessingMode);
			
		row->setTotBandwidth(totBandwidth);
			
		row->setWindowFunction(windowFunction);
	
		return row;		
	}	

	SpectralWindowRow* SpectralWindowTable::newRowFull(BasebandNameMod::BasebandName basebandName, NetSidebandMod::NetSideband netSideband, int numChan, Frequency refFreq, SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode, Frequency totBandwidth, WindowFunctionMod::WindowFunction windowFunction)	{
		SpectralWindowRow *row = new SpectralWindowRow(*this);
			
		row->setBasebandName(basebandName);
			
		row->setNetSideband(netSideband);
			
		row->setNumChan(numChan);
			
		row->setRefFreq(refFreq);
			
		row->setSidebandProcessingMode(sidebandProcessingMode);
			
		row->setTotBandwidth(totBandwidth);
			
		row->setWindowFunction(windowFunction);
	
		return row;				
	}
	


SpectralWindowRow* SpectralWindowTable::newRow(SpectralWindowRow* row) {
	return new SpectralWindowRow(*this, *row);
}

SpectralWindowRow* SpectralWindowTable::newRowCopy(SpectralWindowRow* row) {
	return new SpectralWindowRow(*this, *row);
}

	//
	// Append a row to its table.
	//

	
	 
	
	/** 
 	 * Look up the table for a row whose noautoincrementable attributes are matching their
 	 * homologues in *x.  If a row is found  this row else autoincrement  *x.spectralWindowId, 
 	 * add x to its table and returns x.
 	 *  
 	 * @returns a pointer on a SpectralWindowRow.
 	 * @param x. A pointer on the row to be added.
 	 */ 
 		
			
	SpectralWindowRow* SpectralWindowTable::add(SpectralWindowRow* x) {
			 
		SpectralWindowRow* aRow = lookup(
				
		x->getBasebandName()
				,
		x->getNetSideband()
				,
		x->getNumChan()
				,
		x->getRefFreq()
				,
		x->getSidebandProcessingMode()
				,
		x->getTotBandwidth()
				,
		x->getWindowFunction()
				
		);
		if (aRow) return aRow;
			

			
		// Autoincrement spectralWindowId
		x->setSpectralWindowId(Tag(size(), TagType::SpectralWindow));
						
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
	SpectralWindowRow*  SpectralWindowTable::checkAndAdd(SpectralWindowRow* x)  {
	 
		 
		if (lookup(
			
			x->getBasebandName()
		,
			x->getNetSideband()
		,
			x->getNumChan()
		,
			x->getRefFreq()
		,
			x->getSidebandProcessingMode()
		,
			x->getTotBandwidth()
		,
			x->getWindowFunction()
		
		)) throw UniquenessViolationException("Uniqueness violation exception in table SpectralWindowTable");
		
		
		
		if (getRowByKey(
	
			x->getSpectralWindowId()
			
		)) throw DuplicateKey("Duplicate key exception in ", "SpectralWindowTable");
		
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
	 * @return Alls rows as an array of SpectralWindowRow
	 */
	vector<SpectralWindowRow *> SpectralWindowTable::get() {
		return privateRows;
		// return row;
	}

	
/*
 ** Returns a SpectralWindowRow* given a key.
 ** @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 ** no row exists for that key.
 **
 */
 	SpectralWindowRow* SpectralWindowTable::getRowByKey(Tag spectralWindowId)  {
	SpectralWindowRow* aRow = 0;
	for (unsigned int i = 0; i < row.size(); i++) {
		aRow = row.at(i);
		
			
				if (aRow->spectralWindowId != spectralWindowId) continue;
			
		
		return aRow;
	}
	return 0;		
}
	

	
/**
 * Look up the table for a row whose all attributes  except the autoincrementable one 
 * are equal to the corresponding parameters of the method.
 * @return a pointer on this row if any, 0 otherwise.
 *
			
 * @param basebandName.
 	 		
 * @param netSideband.
 	 		
 * @param numChan.
 	 		
 * @param refFreq.
 	 		
 * @param sidebandProcessingMode.
 	 		
 * @param totBandwidth.
 	 		
 * @param windowFunction.
 	 		 
 */
SpectralWindowRow* SpectralWindowTable::lookup(BasebandNameMod::BasebandName basebandName, NetSidebandMod::NetSideband netSideband, int numChan, Frequency refFreq, SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode, Frequency totBandwidth, WindowFunctionMod::WindowFunction windowFunction) {
		SpectralWindowRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(basebandName, netSideband, numChan, refFreq, sidebandProcessingMode, totBandwidth, windowFunction)) return aRow;
		}			
		return 0;	
} 
	
 	 	

	




#ifndef WITHOUT_ACS
	// Conversion Methods

	SpectralWindowTableIDL *SpectralWindowTable::toIDL() {
		SpectralWindowTableIDL *x = new SpectralWindowTableIDL ();
		unsigned int nrow = size();
		x->row.length(nrow);
		vector<SpectralWindowRow*> v = get();
		for (unsigned int i = 0; i < nrow; ++i) {
			x->row[i] = *(v[i]->toIDL());
		}
		return x;
	}
#endif
	
#ifndef WITHOUT_ACS
	void SpectralWindowTable::fromIDL(SpectralWindowTableIDL x) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			SpectralWindowRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *SpectralWindowTable::toFITS() const  {
		throw ConversionException("Not implemented","SpectralWindow");
	}

	void SpectralWindowTable::fromFITS(char *fits)  {
		throw ConversionException("Not implemented","SpectralWindow");
	}

	string SpectralWindowTable::toVOTable() const {
		throw ConversionException("Not implemented","SpectralWindow");
	}

	void SpectralWindowTable::fromVOTable(string vo) {
		throw ConversionException("Not implemented","SpectralWindow");
	}

	
	string SpectralWindowTable::toXML()  {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
		buf.append("<SpectralWindowTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://Alma/XASDM/SpectralWindowTable\" xsi:schemaLocation=\"http://Alma/XASDM/SpectralWindowTable http://almaobservatory.org/XML/XASDM/2/SpectralWindowTable.xsd\"> ");	
		buf.append(entity.toXML());
		string s = container.getEntity().toXML();
		// Change the "Entity" tag to "ContainerEntity".
		buf.append("<Container" + s.substr(1,s.length() - 1)+" ");
		vector<SpectralWindowRow*> v = get();
		for (unsigned int i = 0; i < v.size(); ++i) {
			try {
				buf.append(v[i]->toXML());
			} catch (NoSuchRow e) {
			}
			buf.append("  ");
		}		
		buf.append("</SpectralWindowTable> ");
		return buf;
	}

	
	void SpectralWindowTable::fromXML(string xmlDoc)  {
		Parser xml(xmlDoc);
		if (!xml.isStr("<SpectralWindowTable")) 
			error();
		// cout << "Parsing a SpectralWindowTable" << endl;
		string s = xml.getElement("<Entity","/>");
		if (s.length() == 0) 
			error();
		Entity e;
		e.setFromXML(s);
		if (e.getEntityTypeName() != "SpectralWindowTable")
			error();
		setEntity(e);
		// Skip the container's entity; but, it has to be there.
		s = xml.getElement("<ContainerEntity","/>");
		if (s.length() == 0) 
			error();

		// Get each row in the table.
		s = xml.getElementContent("<row>","</row>");
		SpectralWindowRow *row;
		while (s.length() != 0) {
			// cout << "Parsing a SpectralWindowRow" << endl; 
			row = newRow();
			row->setFromXML(s);
			try {
				checkAndAdd(row);
			} catch (DuplicateKey e1) {
				throw ConversionException(e1.getMessage(),"SpectralWindowTable");
			} 
			catch (UniquenessViolationException e1) {
				throw ConversionException(e1.getMessage(),"SpectralWindowTable");	
			}
			catch (...) {
				// cout << "Unexpected error in SpectralWindowTable::checkAndAdd called from SpectralWindowTable::fromXML " << endl;
			}
			s = xml.getElementContent("<row>","</row>");
		}
		if (!xml.isStr("</SpectralWindowTable>")) 
			error();
	}

	
	void SpectralWindowTable::error()  {
		throw ConversionException("Invalid xml document","SpectralWindow");
	}
	
	
	string SpectralWindowTable::toMIME() {
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

	
	void SpectralWindowTable::setFromMIME(const string & mimeMsg) {
		// cout << "Entering setFromMIME" << endl;
	 	string terminator = "Content-Type: binary/octet-stream\nContent-ID: <content.bin>\n\n";
	 	
	 	// Look for the string announcing the binary part.
	 	string::size_type loc = mimeMsg.find( terminator, 0 );
	 	
	 	if ( loc == string::npos ) {
	 		throw ConversionException("Failed to detect the beginning of the binary part", "SpectralWindow");
	 	}
	
	 	// Create an EndianISStream from the substring containing the binary part.
	 	EndianISStream eiss(mimeMsg.substr(loc+terminator.size()));
	 	
	 	entity = Entity::fromBin(eiss);
	 	
	 	// We do nothing with that but we have to read it.
	 	Entity containerEntity = Entity::fromBin(eiss);
	 		 	
	 	int numRows = eiss.readInt();
	 	try {
	 		for (int i = 0; i < numRows; i++) {
	 			SpectralWindowRow* aRow = SpectralWindowRow::fromBin(eiss, *this);
	 			checkAndAdd(aRow);
	 		}
	 	}
	 	catch (DuplicateKey e) {
	 		throw ConversionException("Error while writing binary data , the message was "
	 					+ e.getMessage(), "SpectralWindow");
	 	}
		catch (TagFormatException e) {
			throw ConversionException("Error while reading binary data , the message was "
					+ e.getMessage(), "SpectralWindow");
		} 		 	
	}

	
	void SpectralWindowTable::toFile(string directory) {
		if (!directoryExists(directory.c_str()) &&
			!createPath(directory.c_str())) {
			throw ConversionException("Could not create directory " , directory);
		}
		
		if (fileAsBin) {
			// write the bin serialized
			string fileName = directory + "/SpectralWindow.bin";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "SpectralWindow");
			tableout << toMIME() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "SpectralWindow");
		}
		else {
			// write the XML
			string fileName = directory + "/SpectralWindow.xml";
			ofstream tableout(fileName.c_str(),ios::out|ios::trunc);
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not open file " + fileName + " to write ", "SpectralWindow");
			tableout << toXML() << endl;
			tableout.close();
			if (tableout.rdstate() == ostream::failbit)
				throw ConversionException("Could not close file " + fileName, "SpectralWindow");
		}
	}

	
	void SpectralWindowTable::setFromFile(const string& directory) {
		string tablename;
		if (fileAsBin)
			tablename = directory + "/SpectralWindow.bin";
		else
			tablename = directory + "/SpectralWindow.xml";
			
		// Determine the file size.
		ifstream::pos_type size;
		ifstream tablefile(tablename.c_str(), ios::in|ios::binary|ios::ate);

 		if (tablefile.is_open()) { 
  				size = tablefile.tellg(); 
  		}
		else {
				throw ConversionException("Could not open file " + tablename, "SpectralWindow");
		}
		
		// Re position to the beginning.
		tablefile.seekg(0);
		
		// Read in a stringstream.
		stringstream ss;
		ss << tablefile.rdbuf();

		if (tablefile.rdstate() == istream::failbit || tablefile.rdstate() == istream::badbit) {
			throw ConversionException("Error reading file " + tablename,"SpectralWindow");
		}

		// And close
		tablefile.close();
		if (tablefile.rdstate() == istream::failbit)
			throw ConversionException("Could not close file " + tablename,"SpectralWindow");
					
		// And parse the content with the appropriate method
		if (fileAsBin) 
			setFromMIME(ss.str());
		else
			fromXML(ss.str());	
	}			

	

	

			
	
	

	
	void SpectralWindowTable::autoIncrement(string key, SpectralWindowRow* x) {
		map<string, int>::iterator iter;
		if ((iter=noAutoIncIds.find(key)) == noAutoIncIds.end()) {
			// There is not yet a combination of the non autoinc attributes values in the hashtable
			
			// Initialize  spectralWindowId to Tag(0).
			x->setSpectralWindowId(Tag(0,  TagType::SpectralWindow));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, 0));			
		} 
		else {
			// There is already a combination of the non autoinc attributes values in the hashtable
			// Increment its value.
			int n = iter->second + 1; 
			
			// Initialize  spectralWindowId to Tag(n).
			x->setSpectralWindowId(Tag(n, TagType::SpectralWindow));
			
			// Record it in the map.		
			noAutoIncIds.insert(make_pair(key, n));				
		}		
	}
	
} // End namespace asdm
 
