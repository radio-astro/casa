
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
 * File FocusModelTable.h
 */
 
#ifndef FocusModelTable_CLASS
#define FocusModelTable_CLASS

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;



#include <Tag.h>
using  asdm::Tag;




	

	
#include "CPolarizationType.h"
using namespace PolarizationTypeMod;
	

	
#include "CReceiverBand.h"
using namespace ReceiverBandMod;
	

	

	

	

	

	



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
using asdmIDL::FocusModelTableIDL;
#endif

#include <Representable.h>

namespace asdm {

//class asdm::ASDM;
//class asdm::FocusModelRow;

class ASDM;
class FocusModelRow;
/**
 * The FocusModelTable class is an Alma table.
 * <BR>
 * 
 * \par Role
 * Contains the focus model data (function of elevation and temperature).
 * <BR>
 
 * Generated from model's revision "1.57", branch "HEAD"
 *
 * <TABLE BORDER="1">
 * <CAPTION> Attributes of FocusModel </CAPTION>
 * <TR BGCOLOR="#AAAAAA"> <TH> Name </TH> <TH> Type </TH> <TH> Expected shape  </TH> <TH> Comment </TH></TR>
 
 * <TR> <TH BGCOLOR="#CCCCCC" colspan="4" align="center"> Key </TD></TR>
	
 * <TR>
 		
 * <TD> antennaId </TD>
 		 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;refers to a unique row in AntennaTable. </TD>
 * </TR>
	
 * <TR>
 		
 * <TD><I> focusModelId </I></TD>
 		 
 * <TD> int</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;refers to a collection of rows in the table. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Mandatory) </TH></TR>
	
 * <TR>
 * <TD> polarizationType </TD> 
 * <TD> PolarizationTypeMod::PolarizationType </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;identifies the polarization type. </TD>
 * </TR>
	
 * <TR>
 * <TD> receiverBand </TD> 
 * <TD> ReceiverBandMod::ReceiverBand </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;identifies the receiver band. </TD>
 * </TR>
	
 * <TR>
 * <TD> numCoeff </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the number of coefficients. </TD>
 * </TR>
	
 * <TR>
 * <TD> coeffName </TD> 
 * <TD> vector<string > </TD>
 * <TD>  numCoeff </TD> 
 * <TD> &nbsp;the names of the coefficients (one string per coefficient). </TD>
 * </TR>
	
 * <TR>
 * <TD> coeffFormula </TD> 
 * <TD> vector<string > </TD>
 * <TD>  numCoeff </TD> 
 * <TD> &nbsp;textual representations of the fitted functions (one string per coefficient). </TD>
 * </TR>
	
 * <TR>
 * <TD> coeffVal </TD> 
 * <TD> vector<float > </TD>
 * <TD>  numCoeff </TD> 
 * <TD> &nbsp;the values of the coefficients used (one value per coefficient). </TD>
 * </TR>
	
 * <TR>
 * <TD> assocNature </TD> 
 * <TD> string </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;nature of the association with the row refered to by associatedFocusModelId. </TD>
 * </TR>
	
 * <TR>
 * <TD> assocFocusModelId </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;refers to a collection of rows in the table. </TD>
 * </TR>
	


 * </TABLE>
 */
class FocusModelTable : public Representable {
	friend class ASDM;

public:


	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 * @return a vector of string.
	 */	
	static vector<string> getKeyName();


	virtual ~FocusModelTable();
	
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
	 * Return the names of the attributes of this table.
	 *
	 * @return a vector of string
	 */
	 static const vector<string>& getAttributesNames();

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
	 * to the schema defined for FocusModel (FocusModelTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 * @throws ConversionException
	 */
	string toXML()  ;

#ifndef WITHOUT_ACS
	// Conversion Methods
	/**
	 * Convert this table into a FocusModelTableIDL CORBA structure.
	 *
	 * @return a pointer to a FocusModelTableIDL
	 */
	FocusModelTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a FocusModelTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(FocusModelTableIDL x) ;
#endif
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row with default values.
	 * @return a pointer on a FocusModelRow
	 */
	FocusModelRow *newRow();
	
	
	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param antennaId
	
 	 * @param polarizationType
	
 	 * @param receiverBand
	
 	 * @param numCoeff
	
 	 * @param coeffName
	
 	 * @param coeffFormula
	
 	 * @param coeffVal
	
 	 * @param assocNature
	
 	 * @param assocFocusModelId
	
     */
	FocusModelRow *newRow(Tag antennaId, PolarizationTypeMod::PolarizationType polarizationType, ReceiverBandMod::ReceiverBand receiverBand, int numCoeff, vector<string > coeffName, vector<string > coeffFormula, vector<float > coeffVal, string assocNature, int assocFocusModelId);
	


	/**
	 * Create a new row using a copy constructor mechanism.
	 * 
	 * The method creates a new FocusModelRow owned by this. Each attribute of the created row 
	 * is a (deep) copy of the corresponding attribute of row. The method does not add 
	 * the created row to this, its simply parents it to this, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a new FocusModelRow with default values for its attributes. 
	 *
	 * @param row the row which is to be copied.
	 */
	 FocusModelRow *newRow(FocusModelRow *row); 

	//
	// ====> Append a row to its table.
	//

	
	
	
	/** 
	 * Add a row.
	 * If there table contains a row whose key's fields are equal
	 * to x's ones then return a pointer on this row (i.e. no actual insertion is performed) 
	 * otherwise add x to the table and return x.
	 * @param x . A pointer on the row to be added.
 	 * @returns a pointer to a FocusModelRow.	 
	 */	 
	 
 	 FocusModelRow* add(FocusModelRow* x) ;



	//
	// ====> Methods returning rows.
	//
		
	/**
	 * Get all rows.
	 * @return Alls rows as a vector of pointers of FocusModelRow. The elements of this vector are stored in the order 
	 * in which they have been added to the FocusModelTable.
	 */
	vector<FocusModelRow *> get() ;
	


 
	
	/**
 	 * Returns a FocusModelRow* given a key.
 	 * @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 	 * no row exists for that key.
	
	 * @param antennaId
	
	 * @param focusModelId
	
 	 *
	 */
 	FocusModelRow* getRowByKey(Tag antennaId, int focusModelId);

 	 	
 	
	/**
 	 * Returns a vector of pointers on rows whose key element focusModelId 
	 * is equal to the parameter focusModelId.
	 * @return a vector of vector <FocusModelRow *>. A returned vector of size 0 means that no row has been found.
	 * @param focusModelId int contains the value of
	 * the autoincrementable attribute that is looked up in the table.
	 */
 	vector <FocusModelRow *>  getRowByFocusModelId(int);



	/**
 	 * Look up the table for a row whose all attributes  except the autoincrementable one 
 	 * are equal to the corresponding parameters of the method.
 	 * @return a pointer on this row if any, null otherwise.
 	 *
			
 	 * @param antennaId
 	 		
 	 * @param polarizationType
 	 		
 	 * @param receiverBand
 	 		
 	 * @param numCoeff
 	 		
 	 * @param coeffName
 	 		
 	 * @param coeffFormula
 	 		
 	 * @param coeffVal
 	 		
 	 * @param assocNature
 	 		
 	 * @param assocFocusModelId
 	 		 
 	 */
	FocusModelRow* lookup(Tag antennaId, PolarizationTypeMod::PolarizationType polarizationType, ReceiverBandMod::ReceiverBand receiverBand, int numCoeff, vector<string > coeffName, vector<string > coeffFormula, vector<float > coeffVal, string assocNature, int assocFocusModelId); 


private:

	/**
	 * Create a FocusModelTable.
	 * <p>
	 * This constructor is private because only the
	 * container can create tables.  All tables must know the container
	 * to which they belong.
	 * @param container The container to which this table belongs.
	 */ 
	FocusModelTable (ASDM & container);

	ASDM & container;
	
	bool archiveAsBin; // If true archive binary else archive XML
	bool fileAsBin ; // If true file binary else file XML	
	
	Entity entity;
	

	// A map for the autoincrementation algorithm
	map<string,int>  noAutoIncIds;
	void autoIncrement(string key, FocusModelRow* x);


	/**
	 * The name of this table.
	 */
	static string tableName;
	
	/**
	 * The attributes names.
	 */
	static const vector<string> attributesNames;
	
	/**
	 * A method to fill attributesNames;
	 */
	static vector<string> initAttributesNames();


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
	FocusModelRow* checkAndAdd(FocusModelRow* x) ;



// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private vector of FocusModelRow s.
   vector<FocusModelRow * > privateRows;
   

			
	vector<FocusModelRow *> row;

	
	void error() ; //throw(ConversionException);

	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a FocusModel (FocusModelTable.xsd).
	 * @throws ConversionException
	 * 
	 */
	void fromXML(string xmlDoc) ;
		
	/**
	  * Private methods involved during the build of this table out of the content
	  * of file(s) containing an external representation of a FocusModel table.
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
	  * will be saved in a file "FocusModel.bin" or an XML representation (fileAsBin==false) will be saved in a file "FocusModel.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(string directory);
	  
	/**
	 * Reads and parses a file containing a representation of a FocusModelTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const string& directory);	
 
};

} // End namespace asdm

#endif /* FocusModelTable_CLASS */
