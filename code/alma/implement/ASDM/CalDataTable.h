
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
 * File CalDataTable.h
 */
 
#ifndef CalDataTable_CLASS
#define CalDataTable_CLASS

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;



#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Tag.h>
using  asdm::Tag;

#include <EntityRef.h>
using  asdm::EntityRef;




	

	

	

	

	
#include "CCalDataOrigin.h"
using namespace CalDataOriginMod;
	

	
#include "CCalType.h"
using namespace CalTypeMod;
	

	

	

	

	
#include "CAssociatedCalNature.h"
using namespace AssociatedCalNatureMod;
	

	

	

	

	
#include "CScanIntent.h"
using namespace ScanIntentMod;
	



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
using asdmIDL::CalDataTableIDL;
#endif

#include <Representable.h>

namespace asdm {

//class asdm::ASDM;
//class asdm::CalDataRow;

class ASDM;
class CalDataRow;
/**
 * The CalDataTable class is an Alma table.
 * <BR>
 * 
 * \par Role
 * This table describes the data used to derive the calibration results.
 * <BR>
 
 * Generated from model's revision "1.53", branch "HEAD"
 *
 * <TABLE BORDER="1">
 * <CAPTION> Attributes of CalData </CAPTION>
 * <TR BGCOLOR="#AAAAAA"> <TH> Name </TH> <TH> Type </TH> <TH> Expected shape  </TH> <TH> Comment </TH></TR>
 
 * <TR> <TH BGCOLOR="#CCCCCC" colspan="4" align="center"> Key </TD></TR>
	
 * <TR>
 		
 * <TD><I> calDataId </I></TD>
 		 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;identifies a unique row in the table. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Mandarory) </TH></TR>
	
 * <TR>
 * <TD> startTimeObserved </TD> 
 * <TD> ArrayTime </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the start time of observation. </TD>
 * </TR>
	
 * <TR>
 * <TD> endTimeObserved </TD> 
 * <TD> ArrayTime </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the end time of observation. </TD>
 * </TR>
	
 * <TR>
 * <TD> execBlockUID </TD> 
 * <TD> EntityRef </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the UID of the Execution Block. </TD>
 * </TR>
	
 * <TR>
 * <TD> calDataType </TD> 
 * <TD> CalDataOriginMod::CalDataOrigin </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;identifies the origin of the data used for the calibration. </TD>
 * </TR>
	
 * <TR>
 * <TD> calType </TD> 
 * <TD> CalTypeMod::CalType </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;identifies the type of performed calibration. </TD>
 * </TR>
	
 * <TR>
 * <TD> numScan </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the number of scans (in this Execution Block). </TD>
 * </TR>
	
 * <TR>
 * <TD> scanSet </TD> 
 * <TD> vector<int > </TD>
 * <TD>  numScan </TD> 
 * <TD> &nbsp;the set of scan numbers. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Optional) </TH></TR>
	
 * <TR>
 * <TD> assocCalDataId </TD> 
 * <TD> Tag </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; refers to an associate row in CalDataTable. </TD>
 * </TR>
	
 * <TR>
 * <TD> assocCalNature </TD> 
 * <TD> AssociatedCalNatureMod::AssociatedCalNature </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; identifies the nature of the relation with the associate row in CalDataTable. </TD>
 * </TR>
	
 * <TR>
 * <TD> fieldName </TD> 
 * <TD> vector<string > </TD>
 * <TD>  numScan  </TD>
 * <TD>&nbsp; the names of the fields (one name per scan). </TD>
 * </TR>
	
 * <TR>
 * <TD> sourceName </TD> 
 * <TD> vector<string > </TD>
 * <TD>  numScan  </TD>
 * <TD>&nbsp; the names of the sources as given during observations (one source name per scan). </TD>
 * </TR>
	
 * <TR>
 * <TD> sourceCode </TD> 
 * <TD> vector<string > </TD>
 * <TD>  numScan  </TD>
 * <TD>&nbsp; the special characteristics of sources expressed in a textual form (one string per scan). </TD>
 * </TR>
	
 * <TR>
 * <TD> scanIntent </TD> 
 * <TD> vector<ScanIntentMod::ScanIntent > </TD>
 * <TD>  numScan  </TD>
 * <TD>&nbsp; identifies the intents of  the scans (one value per scan). </TD>
 * </TR>
	

 * </TABLE>
 */
class CalDataTable : public Representable {
	friend class ASDM;

public:


	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 * @return a vector of string.
	 */	
	static vector<string> getKeyName();


	virtual ~CalDataTable();
	
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
	 * @return a pointer on a CalDataRow
	 */
	CalDataRow *newRow();
	
	
	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param startTimeObserved
	
 	 * @param endTimeObserved
	
 	 * @param execBlockUID
	
 	 * @param calDataType
	
 	 * @param calType
	
 	 * @param numScan
	
 	 * @param scanSet
	
     */
	CalDataRow *newRow(ArrayTime startTimeObserved, ArrayTime endTimeObserved, EntityRef execBlockUID, CalDataOriginMod::CalDataOrigin calDataType, CalTypeMod::CalType calType, int numScan, vector<int > scanSet);
	


	/**
	 * Create a new row using a copy constructor mechanism.
	 * 
	 * The method creates a new CalDataRow owned by this. Each attribute of the created row 
	 * is a (deep) copy of the corresponding attribute of row. The method does not add 
	 * the created row to this, its simply parents it to this, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a new CalDataRow with default values for its attributes. 
	 *
	 * @param row the row which is to be copied.
	 */
	 CalDataRow *newRow(CalDataRow *row); 

	//
	// ====> Append a row to its table.
	//

	
	
	
	/** 
	 * Add a row.
	 * If there table contains a row whose key's fields are equal
	 * to x's ones then return a pointer on this row (i.e. no actual insertion is performed) 
	 * otherwise add x to the table and return x.
	 * @param x . A pointer on the row to be added.
 	 * @returns a pointer to a CalDataRow.	 
	 */	 
	 
 	 CalDataRow* add(CalDataRow* x) ;



	//
	// ====> Methods returning rows.
	//
		
	/**
	 * Get all rows.
	 * @return Alls rows as a vector of pointers of CalDataRow. The elements of this vector are stored in the order 
	 * in which they have been added to the CalDataTable.
	 */
	vector<CalDataRow *> get() ;
	


 
	
	/**
 	 * Returns a CalDataRow* given a key.
 	 * @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 	 * no row exists for that key.
	
	 * @param calDataId
	
 	 *
	 */
 	CalDataRow* getRowByKey(Tag calDataId);

 	 	



	/**
 	 * Look up the table for a row whose all attributes  except the autoincrementable one 
 	 * are equal to the corresponding parameters of the method.
 	 * @return a pointer on this row if any, null otherwise.
 	 *
			
 	 * @param startTimeObserved
 	 		
 	 * @param endTimeObserved
 	 		
 	 * @param execBlockUID
 	 		
 	 * @param calDataType
 	 		
 	 * @param calType
 	 		
 	 * @param numScan
 	 		
 	 * @param scanSet
 	 		 
 	 */
	CalDataRow* lookup(ArrayTime startTimeObserved, ArrayTime endTimeObserved, EntityRef execBlockUID, CalDataOriginMod::CalDataOrigin calDataType, CalTypeMod::CalType calType, int numScan, vector<int > scanSet); 


private:

	/**
	 * Create a CalDataTable.
	 * <p>
	 * This constructor is private because only the
	 * container can create tables.  All tables must know the container
	 * to which they belong.
	 * @param container The container to which this table belongs.
	 */ 
	CalDataTable (ASDM & container);

	ASDM & container;
	
	bool archiveAsBin; // If true archive binary else archive XML
	bool fileAsBin ; // If true file binary else file XML	
	
	Entity entity;
	

	// A map for the autoincrementation algorithm
	map<string,int>  noAutoIncIds;
	void autoIncrement(string key, CalDataRow* x);


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
	 
	 * @throws UniquenessViolationException
	 
	 */
	CalDataRow* checkAndAdd(CalDataRow* x) ;



// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private vector of CalDataRow s.
   vector<CalDataRow * > privateRows;
   

			
	vector<CalDataRow *> row;


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
	 * Convert this table into a CalDataTableIDL CORBA structure.
	 *
	 * @return a pointer to a CalDataTableIDL
	 */
	CalDataTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a CalDataTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(CalDataTableIDL x) ;
#endif
	
	
	void error() ; //throw(ConversionException);

	/**
	 * Translate this table to an XML representation conform
	 * to the schema defined for CalData (CalDataTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 * @throws ConversionException
	 */
	string toXML()  ;
	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a CalData (CalDataTable.xsd).
	 * @throws ConversionException
	 * 
	 */
	void fromXML(string xmlDoc) ;
		
	/**
	  * Private methods involved during the build of this table out of the content
	  * of file(s) containing an external representation of a CalData table.
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
	  * will be saved in a file "CalData.bin" or an XML representation (fileAsBin==false) will be saved in a file "CalData.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(string directory);
	  
	/**
	 * Reads and parses a file containing a representation of a CalDataTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const string& directory);	
 
};

} // End namespace asdm

#endif /* CalDataTable_CLASS */
