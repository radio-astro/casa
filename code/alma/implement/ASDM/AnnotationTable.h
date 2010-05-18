
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
 * File AnnotationTable.h
 */
 
#ifndef AnnotationTable_CLASS
#define AnnotationTable_CLASS

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;



#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Interval.h>
using  asdm::Interval;

#include <Tag.h>
using  asdm::Tag;




	

	

	

	

	

	
#include "CBasebandName.h"
using namespace BasebandNameMod;
	

	

	

	

	

	

	

	

	



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
using asdmIDL::AnnotationTableIDL;
#endif

#include <Representable.h>

namespace asdm {

//class asdm::ASDM;
//class asdm::AnnotationRow;

class ASDM;
class AnnotationRow;
/**
 * The AnnotationTable class is an Alma table.
 * <BR>
 * 
 * \par Role
 * The Annotation Table is intended to offer space for unexpected data to be  added in the software development process at short notice, without  redefining the data model.
 * <BR>
 
 * Generated from model's revision "1.54", branch "HEAD"
 *
 * <TABLE BORDER="1">
 * <CAPTION> Attributes of Annotation </CAPTION>
 * <TR BGCOLOR="#AAAAAA"> <TH> Name </TH> <TH> Type </TH> <TH> Expected shape  </TH> <TH> Comment </TH></TR>
 
 * <TR> <TH BGCOLOR="#CCCCCC" colspan="4" align="center"> Key </TD></TR>
	
 * <TR>
 		
 * <TD><I> annotationId </I></TD>
 		 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;identifies a unique row in the table. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Mandarory) </TH></TR>
	
 * <TR>
 * <TD> time </TD> 
 * <TD> ArrayTime </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;mid point of the interval of time on which the recorded information is pertinent. </TD>
 * </TR>
	
 * <TR>
 * <TD> issue </TD> 
 * <TD> string </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;name of this annotation. </TD>
 * </TR>
	
 * <TR>
 * <TD> details </TD> 
 * <TD> string </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;details of this annotation. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Optional) </TH></TR>
	
 * <TR>
 * <TD> numAntenna </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; number of antennas. </TD>
 * </TR>
	
 * <TR>
 * <TD> basebandName </TD> 
 * <TD> vector<BasebandNameMod::BasebandName > </TD>
 * <TD>  numBaseband  </TD>
 * <TD>&nbsp; an array of numBaseband baseband names. </TD>
 * </TR>
	
 * <TR>
 * <TD> numBaseband </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; number of basebands. </TD>
 * </TR>
	
 * <TR>
 * <TD> interval </TD> 
 * <TD> Interval </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; time interval </TD>
 * </TR>
	
 * <TR>
 * <TD> dValue </TD> 
 * <TD> double </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; scalar data. </TD>
 * </TR>
	
 * <TR>
 * <TD> vdValue </TD> 
 * <TD> vector<double > </TD>
 * <TD>    </TD>
 * <TD>&nbsp; useful to store an array of double values. </TD>
 * </TR>
	
 * <TR>
 * <TD> vvdValues </TD> 
 * <TD> vector<vector<double > > </TD>
 * <TD>  ,   </TD>
 * <TD>&nbsp; useful to store an array of array(s) of double values. </TD>
 * </TR>
	
 * <TR>
 * <TD> llValue </TD> 
 * <TD> long long </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; useful to record a long long data. </TD>
 * </TR>
	
 * <TR>
 * <TD> vllValue </TD> 
 * <TD> vector<long long > </TD>
 * <TD>    </TD>
 * <TD>&nbsp; useful to store an array of array(s) of long long values. </TD>
 * </TR>
	
 * <TR>
 * <TD> vvllValue </TD> 
 * <TD> vector<vector<long long > > </TD>
 * <TD>  ,   </TD>
 * <TD>&nbsp; useful to store an array of array(s) long long values. </TD>
 * </TR>
	
 * <TR>
 * <TD> antennaId </TD> 
 * <TD> vector<Tag>  </TD>
 * <TD>  numAntenna  </TD>
 * <TD>&nbsp; refers to a collection of rows in the AntennaTable. </TD>
 * </TR>
	

 * </TABLE>
 */
class AnnotationTable : public Representable {
	friend class ASDM;

public:


	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 * @return a vector of string.
	 */	
	static vector<string> getKeyName();


	virtual ~AnnotationTable();
	
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
	 * to the schema defined for Annotation (AnnotationTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 * @throws ConversionException
	 */
	string toXML()  ;

#ifndef WITHOUT_ACS
	// Conversion Methods
	/**
	 * Convert this table into a AnnotationTableIDL CORBA structure.
	 *
	 * @return a pointer to a AnnotationTableIDL
	 */
	AnnotationTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a AnnotationTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(AnnotationTableIDL x) ;
#endif
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row with default values.
	 * @return a pointer on a AnnotationRow
	 */
	AnnotationRow *newRow();
	
	
	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param time
	
 	 * @param issue
	
 	 * @param details
	
     */
	AnnotationRow *newRow(ArrayTime time, string issue, string details);
	


	/**
	 * Create a new row using a copy constructor mechanism.
	 * 
	 * The method creates a new AnnotationRow owned by this. Each attribute of the created row 
	 * is a (deep) copy of the corresponding attribute of row. The method does not add 
	 * the created row to this, its simply parents it to this, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a new AnnotationRow with default values for its attributes. 
	 *
	 * @param row the row which is to be copied.
	 */
	 AnnotationRow *newRow(AnnotationRow *row); 

	//
	// ====> Append a row to its table.
	//

	
	
	
	/** 
	 * Add a row.
	 * If there table contains a row whose key's fields are equal
	 * to x's ones then return a pointer on this row (i.e. no actual insertion is performed) 
	 * otherwise add x to the table and return x.
	 * @param x . A pointer on the row to be added.
 	 * @returns a pointer to a AnnotationRow.	 
	 */	 
	 
 	 AnnotationRow* add(AnnotationRow* x) ;



	//
	// ====> Methods returning rows.
	//
		
	/**
	 * Get all rows.
	 * @return Alls rows as a vector of pointers of AnnotationRow. The elements of this vector are stored in the order 
	 * in which they have been added to the AnnotationTable.
	 */
	vector<AnnotationRow *> get() ;
	


 
	
	/**
 	 * Returns a AnnotationRow* given a key.
 	 * @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 	 * no row exists for that key.
	
	 * @param annotationId
	
 	 *
	 */
 	AnnotationRow* getRowByKey(Tag annotationId);

 	 	



	/**
 	 * Look up the table for a row whose all attributes  except the autoincrementable one 
 	 * are equal to the corresponding parameters of the method.
 	 * @return a pointer on this row if any, null otherwise.
 	 *
			
 	 * @param time
 	 		
 	 * @param issue
 	 		
 	 * @param details
 	 		 
 	 */
	AnnotationRow* lookup(ArrayTime time, string issue, string details); 


private:

	/**
	 * Create a AnnotationTable.
	 * <p>
	 * This constructor is private because only the
	 * container can create tables.  All tables must know the container
	 * to which they belong.
	 * @param container The container to which this table belongs.
	 */ 
	AnnotationTable (ASDM & container);

	ASDM & container;
	
	bool archiveAsBin; // If true archive binary else archive XML
	bool fileAsBin ; // If true file binary else file XML	
	
	Entity entity;
	

	// A map for the autoincrementation algorithm
	map<string,int>  noAutoIncIds;
	void autoIncrement(string key, AnnotationRow* x);


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
	AnnotationRow* checkAndAdd(AnnotationRow* x) ;



// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private vector of AnnotationRow s.
   vector<AnnotationRow * > privateRows;
   

			
	vector<AnnotationRow *> row;

	
	void error() ; //throw(ConversionException);

	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a Annotation (AnnotationTable.xsd).
	 * @throws ConversionException
	 * 
	 */
	void fromXML(string xmlDoc) ;
		
	/**
	  * Private methods involved during the build of this table out of the content
	  * of file(s) containing an external representation of a Annotation table.
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
	  * will be saved in a file "Annotation.bin" or an XML representation (fileAsBin==false) will be saved in a file "Annotation.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(string directory);
	  
	/**
	 * Reads and parses a file containing a representation of a AnnotationTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const string& directory);	
 
};

} // End namespace asdm

#endif /* AnnotationTable_CLASS */
