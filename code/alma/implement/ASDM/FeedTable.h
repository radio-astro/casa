
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
 * File FeedTable.h
 */
 
#ifndef FeedTable_CLASS
#define FeedTable_CLASS

#include <string>
#include <vector>
#include <map>
#include <set>
using std::string;
using std::vector;
using std::map;



#include <Angle.h>
using  asdm::Angle;

#include <Tag.h>
using  asdm::Tag;

#include <Length.h>
using  asdm::Length;

#include <ArrayTimeInterval.h>
using  asdm::ArrayTimeInterval;

#include <Complex.h>
using  asdm::Complex;




	

	

	

	

	

	
#include "CPolarizationType.h"
using namespace PolarizationTypeMod;
	

	

	

	

	

	



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
using asdmIDL::FeedTableIDL;
#endif

#include <Representable.h>

namespace asdm {

//class asdm::ASDM;
//class asdm::FeedRow;

class ASDM;
class FeedRow;
/**
 * The FeedTable class is an Alma table.
 * <BR>
 * 
 * \par Role
 * Contains characteristics of the feeds.
 * <BR>
 
 * Generated from model's revision "1.53", branch "HEAD"
 *
 * <TABLE BORDER="1">
 * <CAPTION> Attributes of Feed </CAPTION>
 * <TR BGCOLOR="#AAAAAA"> <TH> Name </TH> <TH> Type </TH> <TH> Expected shape  </TH> <TH> Comment </TH></TR>
 
 * <TR> <TH BGCOLOR="#CCCCCC" colspan="4" align="center"> Key </TD></TR>
	
 * <TR>
 		
 * <TD> antennaId </TD>
 		 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;refers to a unique row in AntennaTable. </TD>
 * </TR>
	
 * <TR>
 		
 * <TD> spectralWindowId </TD>
 		 
 * <TD> Tag</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;refers to a unique row in SpectralWindowTable. </TD>
 * </TR>
	
 * <TR>
 		
 * <TD> timeInterval </TD>
 		 
 * <TD> ArrayTimeInterval</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;the time interval of validity of the content of the row. </TD>
 * </TR>
	
 * <TR>
 		
 * <TD><I> feedId </I></TD>
 		 
 * <TD> int</TD>
 * <TD> &nbsp; </TD>
 * <TD> &nbsp;identifies a collection of rows in the table. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Mandarory) </TH></TR>
	
 * <TR>
 * <TD> numReceptor </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp;  </TD> 
 * <TD> &nbsp;the number of receptors. </TD>
 * </TR>
	
 * <TR>
 * <TD> beamOffset </TD> 
 * <TD> vector<vector<double > > </TD>
 * <TD>  numReceptor, 2 </TD> 
 * <TD> &nbsp;the offsets of the beam (one pair per receptor). </TD>
 * </TR>
	
 * <TR>
 * <TD> focusReference </TD> 
 * <TD> vector<vector<Length > > </TD>
 * <TD>  numReceptor, 3 </TD> 
 * <TD> &nbsp;the references for the focus position (one triple per receptor). </TD>
 * </TR>
	
 * <TR>
 * <TD> polarizationTypes </TD> 
 * <TD> vector<PolarizationTypeMod::PolarizationType > </TD>
 * <TD>  numReceptor </TD> 
 * <TD> &nbsp;identifies the polarization types (one value per receptor). </TD>
 * </TR>
	
 * <TR>
 * <TD> polResponse </TD> 
 * <TD> vector<vector<Complex > > </TD>
 * <TD>  numReceptor, numReceptor </TD> 
 * <TD> &nbsp;the polarization response (one value per pair of receptors). </TD>
 * </TR>
	
 * <TR>
 * <TD> receptorAngle </TD> 
 * <TD> vector<Angle > </TD>
 * <TD>  numReceptor </TD> 
 * <TD> &nbsp;the receptors angles (one value per receptor). </TD>
 * </TR>
	
 * <TR>
 * <TD> receiverId </TD> 
 * <TD> vector<int>  </TD>
 * <TD>  numReceptor </TD> 
 * <TD> &nbsp;refers to one or more collections of rows in ReceiverTable. </TD>
 * </TR>
	


 * <TR> <TH BGCOLOR="#CCCCCC"  colspan="4" valign="center"> Value <br> (Optional) </TH></TR>
	
 * <TR>
 * <TD> feedNum </TD> 
 * <TD> int </TD>
 * <TD>  &nbsp; </TD>
 * <TD>&nbsp; the feed number to be used for multi-feed receivers. </TD>
 * </TR>
	
 * <TR>
 * <TD> illumOffset </TD> 
 * <TD> vector<Length > </TD>
 * <TD>  2  </TD>
 * <TD>&nbsp; the illumination offset. </TD>
 * </TR>
	
 * <TR>
 * <TD> position </TD> 
 * <TD> vector<Length > </TD>
 * <TD>  3  </TD>
 * <TD>&nbsp; the position of the feed. </TD>
 * </TR>
	
 * <TR>
 * <TD> beamId </TD> 
 * <TD> vector<Tag>  </TD>
 * <TD>  numReceptor  </TD>
 * <TD>&nbsp; refers for each receptor to a unique row in BeamTable. </TD>
 * </TR>
	

 * </TABLE>
 */
class FeedTable : public Representable {
	friend class ASDM;

public:


	/**
	 * Return the list of field names that make up key key
	 * as an array of strings.
	 * @return a vector of string.
	 */	
	static vector<string> getKeyName();


	virtual ~FeedTable();
	
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
		
	/**
	 * Produces an XML representation conform
	 * to the schema defined for Feed (FeedTable.xsd).
	 *
	 * @returns a string containing the XML representation.
	 * @throws ConversionException
	 */
	string toXML()  ;

#ifndef WITHOUT_ACS
	// Conversion Methods
	/**
	 * Convert this table into a FeedTableIDL CORBA structure.
	 *
	 * @return a pointer to a FeedTableIDL
	 */
	FeedTableIDL *toIDL() ;
#endif

#ifndef WITHOUT_ACS
	/**
	 * Populate this table from the content of a FeedTableIDL Corba structure.
	 *
	 * @throws DuplicateKey Thrown if the method tries to add a row having a key that is already in the table.
	 * @throws ConversionException
	 */	
	void fromIDL(FeedTableIDL x) ;
#endif
	
	//
	// ====> Row creation.
	//
	
	/**
	 * Create a new row with default values.
	 * @return a pointer on a FeedRow
	 */
	FeedRow *newRow();
	
	
	/**
	 * Create a new row initialized to the specified values.
	 * @return a pointer on the created and initialized row.
	
 	 * @param antennaId
	
 	 * @param spectralWindowId
	
 	 * @param timeInterval
	
 	 * @param numReceptor
	
 	 * @param beamOffset
	
 	 * @param focusReference
	
 	 * @param polarizationTypes
	
 	 * @param polResponse
	
 	 * @param receptorAngle
	
 	 * @param receiverId
	
     */
	FeedRow *newRow(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int numReceptor, vector<vector<double > > beamOffset, vector<vector<Length > > focusReference, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Complex > > polResponse, vector<Angle > receptorAngle, vector<int>  receiverId);
	


	/**
	 * Create a new row using a copy constructor mechanism.
	 * 
	 * The method creates a new FeedRow owned by this. Each attribute of the created row 
	 * is a (deep) copy of the corresponding attribute of row. The method does not add 
	 * the created row to this, its simply parents it to this, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a new FeedRow with default values for its attributes. 
	 *
	 * @param row the row which is to be copied.
	 */
	 FeedRow *newRow(FeedRow *row); 

	//
	// ====> Append a row to its table.
	//

	
	 
	
	
	/** 
	 * Add a row.
	 * If there table contains a row whose key's fields except² feedId are equal
	 * to x's ones then return a pointer on this row (i.e. no actual insertion is performed) 
	 * otherwise add x to the table and return x.
	 * @param x . A pointer on the row to be added.
 	 * @returns a pointer to a FeedRow.
	 */
	 
 	 FeedRow* add(FeedRow* x) ;



	//
	// ====> Methods returning rows.
	//
		
	/**
	 * Get all rows.
	 * @return Alls rows as a vector of pointers of FeedRow. The elements of this vector are stored in the order 
	 * in which they have been added to the FeedTable.
	 */
	vector<FeedRow *> get() ;
	


 
	
	/**
 	 * Returns a FeedRow* given a key.
 	 * @return a pointer to the row having the key whose values are passed as parameters, or 0 if
 	 * no row exists for that key.
	
	 * @param antennaId
	
	 * @param spectralWindowId
	
	 * @param timeInterval
	
	 * @param feedId
	
 	 *
	 */
 	FeedRow* getRowByKey(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int feedId);

 	 	
 	
	/**
 	 * Returns a vector of pointers on rows whose key element feedId 
	 * is equal to the parameter feedId.
	 * @return a vector of vector <FeedRow *>. A returned vector of size 0 means that no row has been found.
	 * @param feedId int contains the value of
	 * the autoincrementable attribute that is looked up in the table.
	 */
 	vector <FeedRow *>  getRowByFeedId(int);



	/**
 	 * Look up the table for a row whose all attributes  except the autoincrementable one 
 	 * are equal to the corresponding parameters of the method.
 	 * @return a pointer on this row if any, null otherwise.
 	 *
			
 	 * @param antennaId
 	 		
 	 * @param spectralWindowId
 	 		
 	 * @param timeInterval
 	 		
 	 * @param numReceptor
 	 		
 	 * @param beamOffset
 	 		
 	 * @param focusReference
 	 		
 	 * @param polarizationTypes
 	 		
 	 * @param polResponse
 	 		
 	 * @param receptorAngle
 	 		
 	 * @param receiverId
 	 		 
 	 */
	FeedRow* lookup(Tag antennaId, Tag spectralWindowId, ArrayTimeInterval timeInterval, int numReceptor, vector<vector<double > > beamOffset, vector<vector<Length > > focusReference, vector<PolarizationTypeMod::PolarizationType > polarizationTypes, vector<vector<Complex > > polResponse, vector<Angle > receptorAngle, vector<int>  receiverId); 


private:

	/**
	 * Create a FeedTable.
	 * <p>
	 * This constructor is private because only the
	 * container can create tables.  All tables must know the container
	 * to which they belong.
	 * @param container The container to which this table belongs.
	 */ 
	FeedTable (ASDM & container);

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
	 
	 * @throws UniquenessViolationException
	 
	 */
	FeedRow* checkAndAdd(FeedRow* x) ;


	
	
	/**
	 * Insert a FeedRow* in a vector of FeedRow* so that it's ordered by ascending time.
	 *
	 * @param FeedRow* x . The pointer to be inserted.
	 * @param vector <FeedRow*>& row . A reference to the vector where to insert x.
	 *
	 */
	 FeedRow * insertByStartTime(FeedRow* x, vector<FeedRow* >& row);
	  


// A data structure to store the pointers on the table's rows.

// In all cases we maintain a private vector of FeedRow s.
   vector<FeedRow * > privateRows;
   

	

	
	
		
		
	typedef vector <vector <FeedRow* > > ID_TIME_ROWS;
	map<string, ID_TIME_ROWS > context;
	
	/** 
	 * Returns a string built by concatenating the ascii representation of the
	 * parameters values suffixed with a "_" character.
	 */
	 string Key(Tag antennaId, Tag spectralWindowId) ;
	 	
		
	
	
	/**
	 * Fills the vector vout (passed by reference) with pointers on elements of vin 
	 * whose attributes are equal to the corresponding parameters of the method.
	 *
	 */
	void getByKeyNoAutoIncNoTime(vector <FeedRow*>& vin, vector <FeedRow*>& vout,  Tag antennaId, Tag spectralWindowId);
	

	
	void error() ; //throw(ConversionException);

	
	/**
	 * Populate this table from the content of a XML document that is required to
	 * be conform to the XML schema defined for a Feed (FeedTable.xsd).
	 * @throws ConversionException
	 * 
	 */
	void fromXML(string xmlDoc) ;
		
	/**
	  * Private methods involved during the build of this table out of the content
	  * of file(s) containing an external representation of a Feed table.
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
	  * will be saved in a file "Feed.bin" or an XML representation (fileAsBin==false) will be saved in a file "Feed.xml".
	  * The file is always written in a directory whose name is passed as a parameter.
	 * @param directory The name of directory  where the file containing the table's representation will be saved.
	  * 
	  */
	  void toFile(string directory);
	  
	/**
	 * Reads and parses a file containing a representation of a FeedTable as those produced  by the toFile method.
	 * This table is populated with the result of the parsing.
	 * @param directory The name of the directory containing the file te be read and parsed.
	 * @throws ConversionException If any error occurs while reading the 
	 * files in the directory or parsing them.
	 *
	 */
	 void setFromFile(const string& directory);	
 
};

} // End namespace asdm

#endif /* FeedTable_CLASS */
