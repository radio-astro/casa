
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
 * File CalReductionTable.h
 */
 
#ifndef CalReductionTable_CLASS
#define CalReductionTable_CLASS

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;

#include <Angle.h>
#include <AngularRate.h>
#include <ArrayTime.h>
#include <ArrayTimeInterval.h>
#include <Complex.h>
#include <Entity.h>
#include <EntityId.h>
#include <EntityRef.h>
#include <Flux.h>
#include <Frequency.h>
#include <Humidity.h>
#include <Interval.h>
#include <Length.h>
#include <PartId.h>
#include <Pressure.h>
#include <Speed.h>
#include <Tag.h>
#include <Temperature.h>
#include <ConversionException.h>
#include <DuplicateKey.h>
#include <UniquenessViolationException.h>
#include <NoSuchRow.h>
#include <DuplicateKey.h>

/*
#include <Enumerations.h>
using namespace enumerations;
*/




	

	

	

	

	

	

	

	

	

	

	
#include "CInvalidatingCondition.h"
using namespace InvalidatingConditionMod;
	


#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::CalReductionTableIDL;
#endif

using asdm::Angle;
using asdm::AngularRate;
using asdm::ArrayTime;
using asdm::Complex;
using asdm::Entity;
using asdm::EntityId;
using asdm::EntityRef;
using asdm::Flux;
using asdm::Frequency;
using asdm::Humidity;
using asdm::Interval;
using asdm::Length;
using asdm::PartId;
using asdm::Pressure;
using asdm::Speed;
using asdm::Tag;
using asdm::Temperature;

using asdm::DuplicateKey;
using asdm::ConversionException;
using asdm::NoSuchRow;
using asdm::DuplicateKey;

#include <Representable.h>

namespace asdm {

//class asdm::ASDM;
//class asdm::CalReductionRow;

class ASDM;
class CalReductionRow;
/**
 * The CalReductionTable class is an Alma table.
 * 
 * Generated from model's revision "1.46", branch "HEAD"
 *
 * <TABLE BORDER="1">
 * <CAPTION> Attributes of CalReduction </CAPTION>
 * <TR BGCOLOR="#AAAAAA"> <TH> Name </TH> <TH> Type </TH> <TH> Comment </TH></TR>
 
 * <TR> <TH BGCOLOR="#CCCCCC" colspan="3" align="center"> Key </TD></TR>
	
 		
 * <TR>
 * <TD><I> calReductionId </I></TD> 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * </TR>
 		
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="3" valign="center"> Value <br> (Mandarory) </TH></TR>
	
 * <TR>
 * <TD> numApplied </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> numParam </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> timeReduced </TD> 
 * <TD> ArrayTime </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> calAppliedArray </TD> 
 * <TD> vector<string > </TD>
 * <TD>  numApplied </TD> 
 * </TR>
	
 * <TR>
 * <TD> paramSet </TD> 
 * <TD> vector<string > </TD>
 * <TD>  numParam </TD> 
 * </TR>
	
 * <TR>
 * <TD> messages </TD> 
 * <TD> string </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> software </TD> 
 * <TD> string </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> softwareVersion </TD> 
 * <TD> string </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> numInvalidConditions </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * </TR>
	
 * <TR>
 * <TD> invalidConditions </TD> 
 * <TD> vector<InvalidatingConditionMod::InvalidatingCondition > </TD>
 * <TD>  numInvalidConditions </TD> 
 * </TR>
	


 * </TABLE>
 */
class CalReductionTable : public Representable {
	friend class asdm::ASDM;

public:


	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 * @return a vector of string.
	 */	
	static vector<string> getKeyName();


	virtual ~CalReductionTable();
	
	/**
	 * Return the container to which this table belongs.
	 *
	 * @return the ASDM containing this table.
	 */
	ASDM &getContainer() const;
	
	/**
	 * Return the number of rows in the table.
	 *
	 * @return the number of rows in an unsigned int.
	 */
	unsigned int size() ;
	
	/**
	 * Return the name of this table.
	 *
	 * @return the name of this table in a string.
	 */
	string getName() const;

	/**
	 * Return this table's Entity.
	 */
	Entity getEntity() const;

	/**
	 * Set this table's Entity.
	 * @param e An entity. 
	 */
	void setEntity(Entity e);

	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row with default values.
	 * @return a pointer on a CalReductionRow
	 */
	CalReductionRow *newRow();
	
	/**
	  * Has the same definition than the newRow method with the same signature.
	  * Provided to facilitate the call from Python, otherwise the newRow method will be preferred.
	  */
	CalReductionRow* newRowEmpty();

	
	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param numApplied. 
	
 	 * @param numParam. 
	
 	 * @param timeReduced. 
	
 	 * @param calAppliedArray. 
	
 	 * @param paramSet. 
	
 	 * @param messages. 
	
 	 * @param software. 
	
 	 * @param softwareVersion. 
	
 	 * @param numInvalidConditions. 
	
 	 * @param invalidConditions. 
	
     */
	CalReductionRow *newRow(int numApplied, int numParam, ArrayTime timeReduced, vector<string > calAppliedArray, vector<string > paramSet, string messages, string software, string softwareVersion, int numInvalidConditions, vector<InvalidatingConditionMod::InvalidatingCondition > invalidConditions);
	
	/**
	  * Has the same definition than the newRow method with the same signature.
	  * Provided to facilitate the call from Python, otherwise the newRow method will be preferred.
	  */
	CalReductionRow *newRowFull(int numApplied, int numParam, ArrayTime timeReduced, vector<string > calAppliedArray, vector<string > paramSet, string messages, string software, string softwareVersion, int numInvalidConditions, vector<InvalidatingConditionMod::InvalidatingCondition > invalidConditions);


	/**
	 * Create a new row using a copy constructor mechanism.
	 * 
	 * The method creates a new CalReductionRow owned by this. Each attribute of the created row 
	 * is a (deep) copy of the corresponding attribute of row. The method does not add 
	 * the created row to this, its simply parents it to this, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a new CalReductionRow with default values for its attributes. 
	 *
	 * @param row the row which is to be copied.
	 */
	 CalReductionRow *newRow(CalReductionRow *row); 

	/**
	  * Has the same definition than the newRow method with the same signature.
	  * Provided to facilitate the call from Python, otherwise the newRow method will be preferred.
	  */
	 CalReductionRow *newRowCopy(CalReductionRow *row); 

	//
	// ====> Append a row to its table.
	//

	
	
	
	/** 
	 * Add a row.
	 * If there table contains a row whose key's fields are equal
	 * to x's ones then return a pointer on this row (i.e. no actual insertion is performed) 
	 * otherwise add x to the table and return x.
	 * @param x. A pointer on the row to be added.
 	 * @returns a CalReductionRow pointer.
	 */	 
	 
 	 CalReductionRow* add(CalReductionRow* x) ;



	//
	// ====> Methods returning rows.
	//
		
	/**
	 * Get all rows.
	 * @return Alls rows as a vector of pointers of CalReductionRow. The elements of this vector are stored in the order 
	 * in which they have been added to the CalReductionTable.
	 */
	vector<CalReductionRow *> get() ;
	


 
	
	/**
 	 * Returns a CalReductionRow* given a key.
 	 * @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 	 * no row exists for that key.
	
	 * @param calReductionId. 
	
 	 *
	 */
 	CalReductionRow* getRowByKey(Tag calReductionId);

 	 	



	/**
 	 * Look up the table for a row whose all attributes  except the autoincrementable one 
 	 * are equal to the corresponding parameters of the method.
 	 * @return a pointer on this row if any, null otherwise.
 	 *
			
 	 * @param numApplied.
 	 		
 	 * @param numParam.
 	 		
 	 * @param timeReduced.
 	 		
 	 * @param calAppliedArray.
 	 		
 	 * @param paramSet.
 	 		
 	 * @param messages.
 	 		
 	 * @param software.
 	 		
 	 * @param softwareVersion.
 	 		
 	 * @param numInvalidConditions.
 	 		
 	 * @param invalidConditions.
 	 		 
 	 */
	CalReductionRow* lookup(int numApplied, int numParam, ArrayTime timeReduced, vector<string > calAppliedArray, vector<string > paramSet, string messages, string software, string softwareVersion, int numInvalidConditions, vector<InvalidatingConditionMod::InvalidatingCondition > invalidConditions); 


#ifndef WITHOUT_ACS
	// Conversion Methods
	/**
	 * Convert this table into a CalReductionTableIDL CORBA structure.
	 *
	 * @return a pointer to a CalReductionTableIDL
	 */
	CalReductionTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a CalReductionTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(CalReductionTableIDL x) throw(DuplicateKey,ConversionException);
#endif

	/**
	 * To be implemented
	 */
	char *toFITS() const throw(ConversionException);

	/**
	 * To be implemented
	 */
	void fromFITS(char *fits) throw(ConversionException);

	/**
	 * To be implemented
	 */
	string toVOTable() const throw(ConversionException);

	/**
	 * To be implemented
	 */
	void fromVOTable(string vo) throw(ConversionException);

	/**
	 * Translate this table to an XML representation conform
	 * to the schema defined for CalReduction (CalReductionTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 */
	string toXML()  throw(ConversionException);
	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a CalReduction (CalReductionTable.xsd).
	 * 
	 */
	void fromXML(string xmlDoc) throw(ConversionException);
	
   /**
	 * Serialize this into a stream of bytes and encapsulates that stream into a MIME message.
	 * @returns a string containing the MIME message.
	 * 
	 */
	string toMIME();
	
   /** 
     * Extracts the binary part of a MIME message and deserialize its content
	 * to fill this with the result of the deserialization. 
	 * @param mimeMsg the string containing the MIME message.
	 * @throws ConversionException
	 */
	 void setFromMIME(const string & mimeMsg);
	
	/**
	  * Stores a representation (binary or XML) of this table into a file.
	  *
	  * Depending on the boolean value of its private field fileAsBin a binary serialization  of this (fileAsBin==true)  
	  * will be saved in a file "CalReduction.bin" or an XML representation (fileAsBin==false) will be saved in a file "CalReduction.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(string directory);
	  
	/**
	 * Reads and parses a file containing a representation of a CalReductionTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const string& directory);	

private:

	/**
	 * Create a CalReductionTable.
	 * <p>
	 * This constructor is private because only the
	 * container can create tables.  All tables must know the container
	 * to which they belong.
	 * @param container The container to which this table belongs.
	 */ 
	CalReductionTable (ASDM & container);

	ASDM & container;
	
	bool archiveAsBin; // If true archive binary else archive XML
	bool fileAsBin ; // If true file binary else file XML	
	
	Entity entity;
	

	// A map for the autoincrementation algorithm
	map<string,int>  noAutoIncIds;
	void autoIncrement(string key, CalReductionRow* x);


	/**
	 * The name of this table.
	 */
	static string tableName;


	/**
	 * The list of field names that make up key key.
	 */
	static vector<string> key;


	/**
	 * If this table has an autoincrementable attribute then check if *x verifies the rule of uniqueness and throw exception if not.
	 * Check if *x verifies the key uniqueness rule and throw an exception if not.
	 * Append x to its table.
	 */
	CalReductionRow* checkAndAdd(CalReductionRow* x) throw (DuplicateKey, UniquenessViolationException);



// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private ArrayList of CalReductionRow s.
   vector<CalReductionRow * > privateRows;
   

			
	vector<CalReductionRow *> row;


	void error() throw(ConversionException);

};

} // End namespace asdm

#endif /* CalReductionTable_CLASS */
