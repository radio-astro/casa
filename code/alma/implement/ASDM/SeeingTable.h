
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
 * File SeeingTable.h
 */
 
#ifndef SeeingTable_CLASS
#define SeeingTable_CLASS

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;



#include <Angle.h>
using  asdm::Angle;

#include <Length.h>
using  asdm::Length;

#include <ArrayTimeInterval.h>
using  asdm::ArrayTimeInterval;




	

	

	

	

	

	



#include <ConversionException.h>
#include <DuplicateKey.h>
#include <UniquenessViolationException.h>
#include <NoSuchRow.h>
#include <DuplicateKey.h>
using asdm::DuplicateKey;
using asdm::ConversionException;
using asdm::NoSuchRow;
using asdm::DuplicateKey;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::SeeingTableIDL;
#endif

#include <Representable.h>

namespace asdm {

//class asdm::ASDM;
//class asdm::SeeingRow;

class ASDM;
class SeeingRow;
/**
 * The SeeingTable class is an Alma table.
 * <BR>
 * 
 * \par Role
 * Seeing information.
 * <BR>
 
 * Generated from model's revision "1.53", branch "HEAD"
 *
 * <TABLE BORDER="1">
 * <CAPTION> Attributes of Seeing </CAPTION>
 * <TR BGCOLOR="#AAAAAA"> <TH> Name </TH> <TH> Type </TH> <TH> Expected shape  </TH> <TH> Comment </TH></TR>
 
 * <TR> <TH BGCOLOR="#CCCCCC" colspan="4" align="center"> Key </TD></TR>
	
 * <TR>
 		
 * <TD> timeInterval </TD>
 		 
 * <TD> ArrayTimeInterval</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp; </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Mandarory) </TH></TR>
	
 * <TR>
 * <TD> numBaseLength </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp; </TD>
 * </TR>
	
 * <TR>
 * <TD> baseLength </TD> 
 * <TD> vector<Length > </TD>
 * <TD>  numBaseLength </TD> 
 * <TD> &nbsp; </TD>
 * </TR>
	
 * <TR>
 * <TD> phaseRms </TD> 
 * <TD> vector<Angle > </TD>
 * <TD>  numBaseLength </TD> 
 * <TD> &nbsp; </TD>
 * </TR>
	
 * <TR>
 * <TD> seeing </TD> 
 * <TD> float </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp; </TD>
 * </TR>
	
 * <TR>
 * <TD> exponent </TD> 
 * <TD> float </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp; </TD>
 * </TR>
	


 * </TABLE>
 */
class SeeingTable : public Representable {
	friend class ASDM;

public:


	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 * @return a vector of string.
	 */	
	static vector<string> getKeyName();


	virtual ~SeeingTable();
	
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


	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row with default values.
	 * @return a pointer on a SeeingRow
	 */
	SeeingRow *newRow();
	
	
	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param timeInterval
	
 	 * @param numBaseLength
	
 	 * @param baseLength
	
 	 * @param phaseRms
	
 	 * @param seeing
	
 	 * @param exponent
	
     */
	SeeingRow *newRow(ArrayTimeInterval timeInterval, int numBaseLength, vector<Length > baseLength, vector<Angle > phaseRms, float seeing, float exponent);
	


	/**
	 * Create a new row using a copy constructor mechanism.
	 * 
	 * The method creates a new SeeingRow owned by this. Each attribute of the created row 
	 * is a (deep) copy of the corresponding attribute of row. The method does not add 
	 * the created row to this, its simply parents it to this, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a new SeeingRow with default values for its attributes. 
	 *
	 * @param row the row which is to be copied.
	 */
	 SeeingRow *newRow(SeeingRow *row); 

	//
	// ====> Append a row to its table.
	//
 
	
	/**
	 * Add a row.
	 * @param x a pointer to the SeeingRow to be added.
	 *
	 * @return a pointer to a SeeingRow. If the table contains a SeeingRow whose attributes (key and mandatory values) are equal to x ones
	 * then returns a pointer on that SeeingRow, otherwise returns x.
	 *
	 * @throw DuplicateKey { thrown when the table contains a SeeingRow with a key equal to the x one but having
	 * and a value section different from x one }
	 *
	
	 * @note The row is inserted in the table in such a way that all the rows having the same value of
	 * (  ) are stored by ascending time.
	 * @see method getByContext.
	
	 */
	SeeingRow* add(SeeingRow* x) ; 

 



	//
	// ====> Methods returning rows.
	//
		
	/**
	 * Get all rows.
	 * @return Alls rows as a vector of pointers of SeeingRow. The elements of this vector are stored in the order 
	 * in which they have been added to the SeeingTable.
	 */
	vector<SeeingRow *> get() ;
	

	/**
	 * Returns all the rows sorted by ascending startTime for a given context. 
	 * The context is defined by a value of (  ).
	 *
	 * @return a pointer on a vector<SeeingRow *>. A null returned value means that the table contains
	 * no SeeingRow for the given (  ).
	 */
	 vector <SeeingRow*> *getByContext();
	 


 
	
	/**
 	 * Returns a SeeingRow* given a key.
 	 * @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 	 * no row exists for that key.
	
	 * @param timeInterval
	
 	 *
	 */
 	SeeingRow* getRowByKey(ArrayTimeInterval timeInterval);

 	 	



	/**
 	 * Look up the table for a row whose all attributes 
 	 * are equal to the corresponding parameters of the method.
 	 * @return a pointer on this row if any, null otherwise.
 	 *
			
 	 * @param timeInterval
 	 		
 	 * @param numBaseLength
 	 		
 	 * @param baseLength
 	 		
 	 * @param phaseRms
 	 		
 	 * @param seeing
 	 		
 	 * @param exponent
 	 		 
 	 */
	SeeingRow* lookup(ArrayTimeInterval timeInterval, int numBaseLength, vector<Length > baseLength, vector<Angle > phaseRms, float seeing, float exponent); 


private:

	/**
	 * Create a SeeingTable.
	 * <p>
	 * This constructor is private because only the
	 * container can create tables.  All tables must know the container
	 * to which they belong.
	 * @param container The container to which this table belongs.
	 */ 
	SeeingTable (ASDM & container);

	ASDM & container;
	
	bool archiveAsBin; // If true archive binary else archive XML
	bool fileAsBin ; // If true file binary else file XML	
	
	Entity entity;
	


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
	 * @throws DuplicateKey
	 
	 */
	SeeingRow* checkAndAdd(SeeingRow* x) ;


	
	
	/**
	 * Insert a SeeingRow* in a vector of SeeingRow* so that it's ordered by ascending time.
	 *
	 * @param SeeingRow* x . The pointer to be inserted.
	 * @param vector <SeeingRow*>& row . A reference to the vector where to insert x.
	 *
	 */
	 SeeingRow * insertByStartTime(SeeingRow* x, vector<SeeingRow* >& row);
	  


// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private vector of SeeingRow s.
   vector<SeeingRow * > privateRows;
   

	

	
	
		
		
	vector <SeeingRow *> row;
		 
		
	
	


	/**
	 * Return this table's Entity.
	 */
	Entity getEntity() const;

	/**
	 * Set this table's Entity.
	 * @param e An entity. 
	 */
	void setEntity(Entity e);

#ifndef WITHOUT_ACS
	// Conversion Methods
	/**
	 * Convert this table into a SeeingTableIDL CORBA structure.
	 *
	 * @return a pointer to a SeeingTableIDL
	 */
	SeeingTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a SeeingTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(SeeingTableIDL x) ;
#endif
	
	
	void error() ; //throw(ConversionException);

	/**
	 * Translate this table to an XML representation conform
	 * to the schema defined for Seeing (SeeingTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 * @throws ConversionException
	 */
	string toXML()  ;
	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a Seeing (SeeingTable.xsd).
	 * @throws ConversionException
	 * 
	 */
	void fromXML(string xmlDoc) ;
		
	/**
	  * Private methods involved during the build of this table out of the content
	  * of file(s) containing an external representation of a Seeing table.
	  */
	void setFromMIMEFile(const string& directory);
	void setFromXMLFile(const string& directory);
	
		 /**
	 * Serialize this into a stream of bytes and encapsulates that stream into a MIME message.
	 * @returns a string containing the MIME message.
	 *
	 * @param byteOrder a const pointer to a static instance of the class ByteOrder.
	 * 
	 */
	string toMIME(const asdm::ByteOrder* byteOrder=asdm::ByteOrder::Machine_Endianity);
  
	
   /** 
     * Extracts the binary part of a MIME message and deserialize its content
	 * to fill this with the result of the deserialization. 
	 * @param mimeMsg the string containing the MIME message.
	 * @throws ConversionException
	 */
	 void setFromMIME(const string & mimeMsg);
	
	/**
	  * Private methods involved during the export of this table into disk file(s).
	  */
	string MIMEXMLPart(const asdm::ByteOrder* byteOrder=asdm::ByteOrder::Machine_Endianity);
	
	/**
	  * Stores a representation (binary or XML) of this table into a file.
	  *
	  * Depending on the boolean value of its private field fileAsBin a binary serialization  of this (fileAsBin==true)  
	  * will be saved in a file "Seeing.bin" or an XML representation (fileAsBin==false) will be saved in a file "Seeing.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(string directory);
	  
	/**
	 * Reads and parses a file containing a representation of a SeeingTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const string& directory);	
 
};

} // End namespace asdm

#endif /* SeeingTable_CLASS */
