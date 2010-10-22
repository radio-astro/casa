
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
 * File HistoryRow.h
 */
 
#ifndef HistoryRow_CLASS
#define HistoryRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::HistoryRowIDL;
#endif






#include <ArrayTime.h>
using  asdm::ArrayTime;

#include <Tag.h>
using  asdm::Tag;




	

	

	

	

	

	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file History.h
    \brief Generated from model's revision "1.55", branch "HEAD"
*/

namespace asdm {

//class asdm::HistoryTable;


// class asdm::ExecBlockRow;
class ExecBlockRow;
	

class HistoryRow;
typedef void (HistoryRow::*HistoryAttributeFromBin) (EndianISStream& eiss);

/**
 * The HistoryRow class is a row of a HistoryTable.
 * 
 * Generated from model's revision "1.55", branch "HEAD"
 *
 */
class HistoryRow {
friend class asdm::HistoryTable;

public:

	virtual ~HistoryRow();

	/**
	 * Return the table to which this row belongs.
	 */
	HistoryTable &getTable() const;
	
	/**
	 * Has this row been added to its table ?
	 * @return true if and only if it has been added.
	 */
	bool isAdded() const;
		
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute time
	
	
	

	
 	/**
 	 * Get time.
 	 * @return time as ArrayTime
 	 */
 	ArrayTime getTime() const;
	
 
 	
 	
 	/**
 	 * Set time with the specified ArrayTime.
 	 * @param time The ArrayTime value to which time is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setTime (ArrayTime time);
  		
	
	
	


	
	// ===> Attribute message
	
	
	

	
 	/**
 	 * Get message.
 	 * @return message as string
 	 */
 	string getMessage() const;
	
 
 	
 	
 	/**
 	 * Set message with the specified string.
 	 * @param message The string value to which message is to be set.
 	 
 		
 			
 	 */
 	void setMessage (string message);
  		
	
	
	


	
	// ===> Attribute priority
	
	
	

	
 	/**
 	 * Get priority.
 	 * @return priority as string
 	 */
 	string getPriority() const;
	
 
 	
 	
 	/**
 	 * Set priority with the specified string.
 	 * @param priority The string value to which priority is to be set.
 	 
 		
 			
 	 */
 	void setPriority (string priority);
  		
	
	
	


	
	// ===> Attribute origin
	
	
	

	
 	/**
 	 * Get origin.
 	 * @return origin as string
 	 */
 	string getOrigin() const;
	
 
 	
 	
 	/**
 	 * Set origin with the specified string.
 	 * @param origin The string value to which origin is to be set.
 	 
 		
 			
 	 */
 	void setOrigin (string origin);
  		
	
	
	


	
	// ===> Attribute objectId
	
	
	

	
 	/**
 	 * Get objectId.
 	 * @return objectId as string
 	 */
 	string getObjectId() const;
	
 
 	
 	
 	/**
 	 * Set objectId with the specified string.
 	 * @param objectId The string value to which objectId is to be set.
 	 
 		
 			
 	 */
 	void setObjectId (string objectId);
  		
	
	
	


	
	// ===> Attribute application
	
	
	

	
 	/**
 	 * Get application.
 	 * @return application as string
 	 */
 	string getApplication() const;
	
 
 	
 	
 	/**
 	 * Set application with the specified string.
 	 * @param application The string value to which application is to be set.
 	 
 		
 			
 	 */
 	void setApplication (string application);
  		
	
	
	


	
	// ===> Attribute cliCommand
	
	
	

	
 	/**
 	 * Get cliCommand.
 	 * @return cliCommand as string
 	 */
 	string getCliCommand() const;
	
 
 	
 	
 	/**
 	 * Set cliCommand with the specified string.
 	 * @param cliCommand The string value to which cliCommand is to be set.
 	 
 		
 			
 	 */
 	void setCliCommand (string cliCommand);
  		
	
	
	


	
	// ===> Attribute appParms
	
	
	

	
 	/**
 	 * Get appParms.
 	 * @return appParms as string
 	 */
 	string getAppParms() const;
	
 
 	
 	
 	/**
 	 * Set appParms with the specified string.
 	 * @param appParms The string value to which appParms is to be set.
 	 
 		
 			
 	 */
 	void setAppParms (string appParms);
  		
	
	
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute execBlockId
	
	
	

	
 	/**
 	 * Get execBlockId.
 	 * @return execBlockId as Tag
 	 */
 	Tag getExecBlockId() const;
	
 
 	
 	
 	/**
 	 * Set execBlockId with the specified Tag.
 	 * @param execBlockId The Tag value to which execBlockId is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setExecBlockId (Tag execBlockId);
  		
	
	
	


	///////////
	// Links //
	///////////
	
	

	
		
	/**
	 * execBlockId pointer to the row in the ExecBlock table having ExecBlock.execBlockId == execBlockId
	 * @return a ExecBlockRow*
	 * 
	 
	 */
	 ExecBlockRow* getExecBlockUsingExecBlockId();
	 

	

	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this HistoryRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param execBlockId
	    
	 * @param time
	    
	 * @param message
	    
	 * @param priority
	    
	 * @param origin
	    
	 * @param objectId
	    
	 * @param application
	    
	 * @param cliCommand
	    
	 * @param appParms
	    
	 */ 
	bool compareNoAutoInc(Tag execBlockId, ArrayTime time, string message, string priority, string origin, string objectId, string application, string cliCommand, string appParms);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param message
	    
	 * @param priority
	    
	 * @param origin
	    
	 * @param objectId
	    
	 * @param application
	    
	 * @param cliCommand
	    
	 * @param appParms
	    
	 */ 
	bool compareRequiredValue(string message, string priority, string origin, string objectId, string application, string cliCommand, string appParms); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the HistoryRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(HistoryRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a HistoryRowIDL struct.
	 */
	HistoryRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct HistoryRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (HistoryRowIDL x) ;
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string toXML() const;

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param rowDoc the XML string being used to set the values of this row.
	 * @throws ConversionException
	 */
	void setFromXML (string rowDoc) ;	

private:
	/**
	 * The table to which this row belongs.
	 */
	HistoryTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded(bool added);


	/**
	 * Create a HistoryRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	HistoryRow (HistoryTable &table);

	/**
	 * Create a HistoryRow using a copy constructor mechanism.
	 * <p>
	 * Given a HistoryRow row and a HistoryTable table, the method creates a new
	 * HistoryRow owned by table. Each attribute of the created row is a copy (deep)
	 * of the corresponding attribute of row. The method does not add the created
	 * row to its table, its simply parents it to table, a call to the add method
	 * has to be done in order to get the row added (very likely after having modified
	 * some of its attributes).
	 * If row is null then the method returns a row with default values for its attributes. 
	 *
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 * @param row  The row which is to be copied.
	 */
	 HistoryRow (HistoryTable &table, HistoryRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute time
	
	

	ArrayTime time;

	
	
 	

	
	// ===> Attribute message
	
	

	string message;

	
	
 	

	
	// ===> Attribute priority
	
	

	string priority;

	
	
 	

	
	// ===> Attribute origin
	
	

	string origin;

	
	
 	

	
	// ===> Attribute objectId
	
	

	string objectId;

	
	
 	

	
	// ===> Attribute application
	
	

	string application;

	
	
 	

	
	// ===> Attribute cliCommand
	
	

	string cliCommand;

	
	
 	

	
	// ===> Attribute appParms
	
	

	string appParms;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute execBlockId
	
	

	Tag execBlockId;

	
	
 	

	///////////
	// Links //
	///////////
	
	
		

	 

	

	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, HistoryAttributeFromBin> fromBinMethods;
void execBlockIdFromBin( EndianISStream& eiss);
void timeFromBin( EndianISStream& eiss);
void messageFromBin( EndianISStream& eiss);
void priorityFromBin( EndianISStream& eiss);
void originFromBin( EndianISStream& eiss);
void objectIdFromBin( EndianISStream& eiss);
void applicationFromBin( EndianISStream& eiss);
void cliCommandFromBin( EndianISStream& eiss);
void appParmsFromBin( EndianISStream& eiss);

		
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the HistoryTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static HistoryRow* fromBin(EndianISStream& eiss, HistoryTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* History_CLASS */
