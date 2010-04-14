
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
 * File FlagCmdRow.h
 */
 
#ifndef FlagCmdRow_CLASS
#define FlagCmdRow_CLASS

#include <vector>
#include <string>
#include <set>
using std::vector;
using std::string;
using std::set;

#ifndef WITHOUT_ACS
#include <asdmIDLC.h>
using asdmIDL::FlagCmdRowIDL;
#endif



#include <ArrayTimeInterval.h>
using  asdm::ArrayTimeInterval;




	

	

	

	

	

	

	



#include <ConversionException.h>
#include <NoSuchRow.h>
#include <IllegalAccessException.h>


/*\file FlagCmd.h
    \brief Generated from model's revision "1.53", branch "HEAD"
*/

namespace asdm {

//class asdm::FlagCmdTable;

	

class FlagCmdRow;
typedef void (FlagCmdRow::*FlagCmdAttributeFromBin) (EndianISStream& eiss);

/**
 * The FlagCmdRow class is a row of a FlagCmdTable.
 * 
 * Generated from model's revision "1.53", branch "HEAD"
 *
 */
class FlagCmdRow {
friend class asdm::FlagCmdTable;

public:

	virtual ~FlagCmdRow();

	/**
	 * Return the table to which this row belongs.
	 */
	FlagCmdTable &getTable() const;
	
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute timeInterval
	
	
	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval getTimeInterval() const;
	
 
 	
 	
 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 		
 			
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 		
 	 */
 	void setTimeInterval (ArrayTimeInterval timeInterval);
  		
	
	
	


	
	// ===> Attribute type
	
	
	

	
 	/**
 	 * Get type.
 	 * @return type as string
 	 */
 	string getType() const;
	
 
 	
 	
 	/**
 	 * Set type with the specified string.
 	 * @param type The string value to which type is to be set.
 	 
 		
 			
 	 */
 	void setType (string type);
  		
	
	
	


	
	// ===> Attribute reason
	
	
	

	
 	/**
 	 * Get reason.
 	 * @return reason as string
 	 */
 	string getReason() const;
	
 
 	
 	
 	/**
 	 * Set reason with the specified string.
 	 * @param reason The string value to which reason is to be set.
 	 
 		
 			
 	 */
 	void setReason (string reason);
  		
	
	
	


	
	// ===> Attribute level
	
	
	

	
 	/**
 	 * Get level.
 	 * @return level as int
 	 */
 	int getLevel() const;
	
 
 	
 	
 	/**
 	 * Set level with the specified int.
 	 * @param level The int value to which level is to be set.
 	 
 		
 			
 	 */
 	void setLevel (int level);
  		
	
	
	


	
	// ===> Attribute severity
	
	
	

	
 	/**
 	 * Get severity.
 	 * @return severity as int
 	 */
 	int getSeverity() const;
	
 
 	
 	
 	/**
 	 * Set severity with the specified int.
 	 * @param severity The int value to which severity is to be set.
 	 
 		
 			
 	 */
 	void setSeverity (int severity);
  		
	
	
	


	
	// ===> Attribute applied
	
	
	

	
 	/**
 	 * Get applied.
 	 * @return applied as bool
 	 */
 	bool getApplied() const;
	
 
 	
 	
 	/**
 	 * Set applied with the specified bool.
 	 * @param applied The bool value to which applied is to be set.
 	 
 		
 			
 	 */
 	void setApplied (bool applied);
  		
	
	
	


	
	// ===> Attribute command
	
	
	

	
 	/**
 	 * Get command.
 	 * @return command as string
 	 */
 	string getCommand() const;
	
 
 	
 	
 	/**
 	 * Set command with the specified string.
 	 * @param command The string value to which command is to be set.
 	 
 		
 			
 	 */
 	void setCommand (string command);
  		
	
	
	


	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	
	
	/**
	 * Compare each mandatory attribute except the autoincrementable one of this FlagCmdRow with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param timeInterval
	    
	 * @param type
	    
	 * @param reason
	    
	 * @param level
	    
	 * @param severity
	    
	 * @param applied
	    
	 * @param command
	    
	 */ 
	bool compareNoAutoInc(ArrayTimeInterval timeInterval, string type, string reason, int level, int severity, bool applied, string command);
	
	

	
	/**
	 * Compare each mandatory value (i.e. not in the key) attribute  with 
	 * the corresponding parameters and return true if there is a match and false otherwise.
	 	
	 * @param type
	    
	 * @param reason
	    
	 * @param level
	    
	 * @param severity
	    
	 * @param applied
	    
	 * @param command
	    
	 */ 
	bool compareRequiredValue(string type, string reason, int level, int severity, bool applied, string command); 
		 
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the FlagCmdRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool equalByRequiredValue(FlagCmdRow* x) ;
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a FlagCmdRowIDL struct.
	 */
	FlagCmdRowIDL *toIDL() const;
#endif
	
#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct FlagCmdRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 * @throws ConversionException
	 */
	void setFromIDL (FlagCmdRowIDL x) ;
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
	FlagCmdTable &table;
	/**
	 * Whether this row has been added to the table or not.
	 */
	bool hasBeenAdded;

	// This method is used by the Table class when this row is added to the table.
	void isAdded();


	/**
	 * Create a FlagCmdRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	FlagCmdRow (FlagCmdTable &table);

	/**
	 * Create a FlagCmdRow using a copy constructor mechanism.
	 * <p>
	 * Given a FlagCmdRow row and a FlagCmdTable table, the method creates a new
	 * FlagCmdRow owned by table. Each attribute of the created row is a copy (deep)
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
	 FlagCmdRow (FlagCmdTable &table, FlagCmdRow &row);
	 	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	
	// ===> Attribute timeInterval
	
	

	ArrayTimeInterval timeInterval;

	
	
 	

	
	// ===> Attribute type
	
	

	string type;

	
	
 	

	
	// ===> Attribute reason
	
	

	string reason;

	
	
 	

	
	// ===> Attribute level
	
	

	int level;

	
	
 	

	
	// ===> Attribute severity
	
	

	int severity;

	
	
 	

	
	// ===> Attribute applied
	
	

	bool applied;

	
	
 	

	
	// ===> Attribute command
	
	

	string command;

	
	
 	

	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	///////////////////////////////
	// binary-deserialization material//
	///////////////////////////////
	map<string, FlagCmdAttributeFromBin> fromBinMethods;
void timeIntervalFromBin( EndianISStream& eiss);
void typeFromBin( EndianISStream& eiss);
void reasonFromBin( EndianISStream& eiss);
void levelFromBin( EndianISStream& eiss);
void severityFromBin( EndianISStream& eiss);
void appliedFromBin( EndianISStream& eiss);
void commandFromBin( EndianISStream& eiss);

		
	
	/**
	 * Serialize this into a stream of bytes written to an EndianOSStream.
	 * @param eoss the EndianOSStream to be written to
	 */
	 void toBin(EndianOSStream& eoss);
	 	 
	 /**
	  * Deserialize a stream of bytes read from an EndianISStream to build a PointingRow.
	  * @param eiss the EndianISStream to be read.
	  * @param table the FlagCmdTable to which the row built by deserialization will be parented.
	  * @param attributesSeq a vector containing the names of the attributes . The elements order defines the order 
	  * in which the attributes are written in the binary serialization.
	  */
	 static FlagCmdRow* fromBin(EndianISStream& eiss, FlagCmdTable& table, const vector<string>& attributesSeq);	 

};

} // End namespace asdm

#endif /* FlagCmd_CLASS */
