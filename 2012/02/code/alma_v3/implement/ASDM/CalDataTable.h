
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



	
#include <ArrayTime.h>
	

	
#include <Tag.h>
	

	
#include <EntityRef.h>
	




	

	

	

	

	
#include "CCalDataOrigin.h"
	

	
#include "CCalType.h"
	

	

	

	

	
#include "CAssociatedCalNature.h"
	

	

	

	

	
#include "CScanIntent.h"
	



#include <ConversionException.h>
#include <DuplicateKey.h>
#include <UniquenessViolationException.h>
#include <NoSuchRow.h>
#include <DuplicateKey.h>


#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
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
 
 * Generated from model's revision "1.61", branch "HEAD"
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
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Mandatory) </TH></TR>
	
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
	static std::vector<std::string> getKeyName();


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
	unsigned int size() const;
	
	/**
	 * Return the name of this table.
	 *
	 * This is a instance method of the class.
	 *
	 * @return the name of this table in a string.
	 */
	std::string getName() const;
	
	/**
	 * Return the name of this table.
	 *
	 * This is a static method of the class.
	 *
	 * @return the name of this table in a string.
	 */
	static std::string name() ;	
	
	/**
	 * Return the version information about this table.
	 *
	 */
	 std::string getVersion() const ;
	
	/**
	 * Return the names of the attributes of this table.
	 *
	 * @return a vector of string
	 */
	 static const std::vector<std::string>& getAttributesNames();

	/**
	 * Return the default sorted list of attributes names in the binary representation of the table.
	 *
	 * @return a const reference to a vector of string
	 */
	 static const std::vector<std::string>& defaultAttributesNamesInBin();
	 
	/**
	 * Return this table's Entity.
	 */
	Entity getEntity() const;

	/**
	 * Set this table's Entity.
	 * @param e An entity. 
	 */
	void setEntity(Entity e);
		
	/**
	 * Produces an XML representation conform
	 * to the schema defined for CalData (CalDataTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 * @throws ConversionException
	 */
	std::string toXML()  ;

#ifndef WITHOUT_ACS
	// Conversion Methods
	/**
	 * Convert this table into a CalDataTableIDL CORBA structure.
	 *
	 * @return a pointer to a CalDataTableIDL
	 */
	asdmIDL::CalDataTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a CalDataTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(asdmIDL::CalDataTableIDL x) ;
#endif
	
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
	 * Get a collection of pointers on the rows of the table.
	 * @return Alls rows in a vector of pointers of CalDataRow. The elements of this vector are stored in the order 
	 * in which they have been added to the CalDataTable.
	 */
	std::vector<CalDataRow *> get() ;
	
	/**
	 * Get a const reference on the collection of rows pointers internally hold by the table.
	 * @return A const reference of a vector of pointers of CalDataRow. The elements of this vector are stored in the order 
	 * in which they have been added to the CalDataTable.
	 *
	 */
	 const std::vector<CalDataRow *>& get() const ;
	


 
	
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


	void setUnknownAttributeBinaryReader(const std::string& attributeName, BinaryAttributeReaderFunctor* barFctr);
	BinaryAttributeReaderFunctor* getUnknownAttributeBinaryReader(const std::string& attributeName) const;

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
	
	std::string version ; 
	
	Entity entity;
	

	// A map for the autoincrementation algorithm
	std::map<std::string,int>  noAutoIncIds;
	void autoIncrement(std::string key, CalDataRow* x);


	/**
	 * The name of this table.
	 */
	static std::string itsName;
	
	/**
	 * The attributes names.
	 */
	static std::vector<std::string> attributesNames;
	
	/**
	 * The attributes names in the order in which they appear in the binary representation of the table.
	 */
	static std::vector<std::string> attributesNamesInBin;
	

	/**
	 * A method to fill attributesNames and attributesNamesInBin;
	 */
	static bool initAttributesNames(), initAttributesNamesDone ;
	

	/**
	 * The list of field names that make up key key.
	 */
	static std::vector<std::string> key;


	/**
	 * If this table has an autoincrementable attribute then check if *x verifies the rule of uniqueness and throw exception if not.
	 * Check if *x verifies the key uniqueness rule and throw an exception if not.
	 * Append x to its table.
	 * @throws DuplicateKey
	 
	 * @throws UniquenessViolationException
	 
	 */
	CalDataRow* checkAndAdd(CalDataRow* x) ;
	
	/**
	 * Brutally append an CalDataRow x to the collection of rows already stored in this table. No uniqueness check is done !
	 *
	 * @param CalDataRow* x a pointer onto the CalDataRow to be appended.
	 */
	 void append(CalDataRow* x) ;
	 
	/**
	 * Brutally append an CalDataRow x to the collection of rows already stored in this table. No uniqueness check is done !
	 *
	 * @param CalDataRow* x a pointer onto the CalDataRow to be appended.
	 */
	 void addWithoutCheckingUnique(CalDataRow* x) ;
	 
	 



// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private vector of CalDataRow s.
   std::vector<CalDataRow * > privateRows;
   

			
	std::vector<CalDataRow *> row;

	
	void error() ; //throw(ConversionException);

	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a CalData (CalDataTable.xsd).
	 * @throws ConversionException
	 * 
	 */
	void fromXML(std::string& xmlDoc) ;
		
	std::map<std::string, BinaryAttributeReaderFunctor *> unknownAttributes2Functors;

	/**
	  * Private methods involved during the build of this table out of the content
	  * of file(s) containing an external representation of a CalData table.
	  */
	void setFromMIMEFile(const std::string& directory);
	/*
	void openMIMEFile(const std::string& directory);
	*/
	void setFromXMLFile(const std::string& directory);
	
		 /**
	 * Serialize this into a stream of bytes and encapsulates that stream into a MIME message.
	 * @returns a string containing the MIME message.
	 *
	 * @param byteOrder a const pointer to a static instance of the class ByteOrder.
	 * 
	 */
	std::string toMIME(const asdm::ByteOrder* byteOrder=asdm::ByteOrder::Machine_Endianity);
  
	
   /** 
     * Extracts the binary part of a MIME message and deserialize its content
	 * to fill this with the result of the deserialization. 
	 * @param mimeMsg the string containing the MIME message.
	 * @throws ConversionException
	 */
	 void setFromMIME(const std::string & mimeMsg);
	
	/**
	  * Private methods involved during the export of this table into disk file(s).
	  */
	std::string MIMEXMLPart(const asdm::ByteOrder* byteOrder=asdm::ByteOrder::Machine_Endianity);
	
	/**
	  * Stores a representation (binary or XML) of this table into a file.
	  *
	  * Depending on the boolean value of its private field fileAsBin a binary serialization  of this (fileAsBin==true)  
	  * will be saved in a file "CalData.bin" or an XML representation (fileAsBin==false) will be saved in a file "CalData.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(std::string directory);
	  
	  /**
	   * Load the table in memory if necessary.
	   */
	  bool loadInProgress;
	  void checkPresenceInMemory() {
		if (!presentInMemory && !loadInProgress) {
			loadInProgress = true;
			setFromFile(getContainer().getDirectory());
			presentInMemory = true;
			loadInProgress = false;
	  	}
	  }
	/**
	 * Reads and parses a file containing a representation of a CalDataTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const std::string& directory);	
 
};

} // End namespace asdm

#endif /* CalDataTable_CLASS */
