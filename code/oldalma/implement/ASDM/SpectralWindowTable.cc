
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
	
 	 * @param numChan. 
	
 	 * @param refFreq. 
	
 	 * @param chanFreq. 
	
 	 * @param chanWidth. 
	
 	 * @param effectiveBw. 
	
 	 * @param resolution. 
	
 	 * @param totBandwidth. 
	
 	 * @param netSideband. 
	
 	 * @param sidebandProcessingMode. 
	
 	 * @param quantization. 
	
 	 * @param windowFunction. 
	
 	 * @param oversampling. 
	
 	 * @param correlationBit. 
	
 	 * @param flagRow. 
	
     */
	SpectralWindowRow* SpectralWindowTable::newRow(int numChan, Frequency refFreq, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<Frequency > effectiveBw, vector<Frequency > resolution, Frequency totBandwidth, NetSidebandMod::NetSideband netSideband, SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode, bool quantization, WindowFunctionMod::WindowFunction windowFunction, bool oversampling, CorrelationBitMod::CorrelationBit correlationBit, bool flagRow){
		SpectralWindowRow *row = new SpectralWindowRow(*this);
			
		row->setNumChan(numChan);
			
		row->setRefFreq(refFreq);
			
		row->setChanFreq(chanFreq);
			
		row->setChanWidth(chanWidth);
			
		row->setEffectiveBw(effectiveBw);
			
		row->setResolution(resolution);
			
		row->setTotBandwidth(totBandwidth);
			
		row->setNetSideband(netSideband);
			
		row->setSidebandProcessingMode(sidebandProcessingMode);
			
		row->setQuantization(quantization);
			
		row->setWindowFunction(windowFunction);
			
		row->setOversampling(oversampling);
			
		row->setCorrelationBit(correlationBit);
			
		row->setFlagRow(flagRow);
	
		return row;		
	}	

	SpectralWindowRow* SpectralWindowTable::newRowFull(int numChan, Frequency refFreq, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<Frequency > effectiveBw, vector<Frequency > resolution, Frequency totBandwidth, NetSidebandMod::NetSideband netSideband, SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode, bool quantization, WindowFunctionMod::WindowFunction windowFunction, bool oversampling, CorrelationBitMod::CorrelationBit correlationBit, bool flagRow)	{
		SpectralWindowRow *row = new SpectralWindowRow(*this);
			
		row->setNumChan(numChan);
			
		row->setRefFreq(refFreq);
			
		row->setChanFreq(chanFreq);
			
		row->setChanWidth(chanWidth);
			
		row->setEffectiveBw(effectiveBw);
			
		row->setResolution(resolution);
			
		row->setTotBandwidth(totBandwidth);
			
		row->setNetSideband(netSideband);
			
		row->setSidebandProcessingMode(sidebandProcessingMode);
			
		row->setQuantization(quantization);
			
		row->setWindowFunction(windowFunction);
			
		row->setOversampling(oversampling);
			
		row->setCorrelationBit(correlationBit);
			
		row->setFlagRow(flagRow);
	
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
				
		x->getNumChan()
				,
		x->getRefFreq()
				,
		x->getChanFreq()
				,
		x->getChanWidth()
				,
		x->getEffectiveBw()
				,
		x->getResolution()
				,
		x->getTotBandwidth()
				,
		x->getNetSideband()
				,
		x->getSidebandProcessingMode()
				,
		x->getQuantization()
				,
		x->getWindowFunction()
				,
		x->getOversampling()
				,
		x->getCorrelationBit()
				,
		x->getFlagRow()
				
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
	 */
	SpectralWindowRow*  SpectralWindowTable::checkAndAdd(SpectralWindowRow* x) throw (DuplicateKey, UniquenessViolationException) {
	 
		 
		if (lookup(
			
			x->getNumChan()
		,
			x->getRefFreq()
		,
			x->getChanFreq()
		,
			x->getChanWidth()
		,
			x->getEffectiveBw()
		,
			x->getResolution()
		,
			x->getTotBandwidth()
		,
			x->getNetSideband()
		,
			x->getSidebandProcessingMode()
		,
			x->getQuantization()
		,
			x->getWindowFunction()
		,
			x->getOversampling()
		,
			x->getCorrelationBit()
		,
			x->getFlagRow()
		
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
			
 * @param numChan.
 	 		
 * @param refFreq.
 	 		
 * @param chanFreq.
 	 		
 * @param chanWidth.
 	 		
 * @param effectiveBw.
 	 		
 * @param resolution.
 	 		
 * @param totBandwidth.
 	 		
 * @param netSideband.
 	 		
 * @param sidebandProcessingMode.
 	 		
 * @param quantization.
 	 		
 * @param windowFunction.
 	 		
 * @param oversampling.
 	 		
 * @param correlationBit.
 	 		
 * @param flagRow.
 	 		 
 */
SpectralWindowRow* SpectralWindowTable::lookup(int numChan, Frequency refFreq, vector<Frequency > chanFreq, vector<Frequency > chanWidth, vector<Frequency > effectiveBw, vector<Frequency > resolution, Frequency totBandwidth, NetSidebandMod::NetSideband netSideband, SidebandProcessingModeMod::SidebandProcessingMode sidebandProcessingMode, bool quantization, WindowFunctionMod::WindowFunction windowFunction, bool oversampling, CorrelationBitMod::CorrelationBit correlationBit, bool flagRow) {
		SpectralWindowRow* aRow;
		for (unsigned int i = 0; i < size(); i++) {
			aRow = row.at(i); 
			if (aRow->compareNoAutoInc(numChan, refFreq, chanFreq, chanWidth, effectiveBw, resolution, totBandwidth, netSideband, sidebandProcessingMode, quantization, windowFunction, oversampling, correlationBit, flagRow)) return aRow;
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
	void SpectralWindowTable::fromIDL(SpectralWindowTableIDL x) throw(DuplicateKey,ConversionException) {
		unsigned int nrow = x.row.length();
		for (unsigned int i = 0; i < nrow; ++i) {
			SpectralWindowRow *tmp = newRow();
			tmp->setFromIDL(x.row[i]);
			// checkAndAdd(tmp);
			add(tmp);
		}
	}
#endif

	char *SpectralWindowTable::toFITS() const throw(ConversionException) {
		throw ConversionException("Not implemented","SpectralWindow");
	}

	void SpectralWindowTable::fromFITS(char *fits) throw(ConversionException) {
		throw ConversionException("Not implemented","SpectralWindow");
	}

	string SpectralWindowTable::toVOTable() const throw(ConversionException) {
		throw ConversionException("Not implemented","SpectralWindow");
	}

	void SpectralWindowTable::fromVOTable(string vo) throw(ConversionException) {
		throw ConversionException("Not implemented","SpectralWindow");
	}

	string SpectralWindowTable::toXML()  throw(ConversionException) {
		string buf;
		buf.append("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?> ");
//		buf.append("<SpectralWindowTable xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"../../idl/SpectralWindowTable.xsd\"> ");
		buf.append("<?xml-stylesheet type=\"text/xsl\" href=\"../asdm2html/table2html.xsl\"?> ");		
		buf.append("<SpectralWindowTable> ");
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
	
	void SpectralWindowTable::fromXML(string xmlDoc) throw(ConversionException) {
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

	void SpectralWindowTable::error() throw(ConversionException) {
		throw ConversionException("Invalid xml document","SpectralWindow");
	}
	
	string SpectralWindowTable::toMIME() {
	 // To be implemented
		return "";
	}
	
	void SpectralWindowTable::setFromMIME(const string & mimeMsg) {
		// To be implemented
		;
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
 
