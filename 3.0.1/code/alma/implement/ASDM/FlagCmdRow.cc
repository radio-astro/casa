
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
 * File FlagCmdRow.cpp
 */
 
#include <vector>
using std::vector;

#include <set>
using std::set;

#include <ASDM.h>
#include <FlagCmdRow.h>
#include <FlagCmdTable.h>
	

using asdm::ASDM;
using asdm::FlagCmdRow;
using asdm::FlagCmdTable;


#include <Parser.h>
using asdm::Parser;

#include <EnumerationParser.h>
 
#include <InvalidArgumentException.h>
using asdm::InvalidArgumentException;

namespace asdm {
	FlagCmdRow::~FlagCmdRow() {
	}

	/**
	 * Return the table to which this row belongs.
	 */
	FlagCmdTable &FlagCmdRow::getTable() const {
		return table;
	}
	
	void FlagCmdRow::isAdded() {
		hasBeenAdded = true;
	}
	
	
#ifndef WITHOUT_ACS
	/**
	 * Return this row in the form of an IDL struct.
	 * @return The values of this row as a FlagCmdRowIDL struct.
	 */
	FlagCmdRowIDL *FlagCmdRow::toIDL() const {
		FlagCmdRowIDL *x = new FlagCmdRowIDL ();
		
		// Fill the IDL structure.
	
		
	
  		
		
		
			
		x->timeInterval = timeInterval.toIDLArrayTimeInterval();
			
		
	

	
  		
		
		
			
				
		x->type = CORBA::string_dup(type.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->reason = CORBA::string_dup(reason.c_str());
				
 			
		
	

	
  		
		
		
			
				
		x->level = level;
 				
 			
		
	

	
  		
		
		
			
				
		x->severity = severity;
 				
 			
		
	

	
  		
		
		
			
				
		x->applied = applied;
 				
 			
		
	

	
  		
		
		
			
				
		x->command = CORBA::string_dup(command.c_str());
				
 			
		
	

	
	
		
		
		return x;
	
	}
#endif
	

#ifndef WITHOUT_ACS
	/**
	 * Fill the values of this row from the IDL struct FlagCmdRowIDL.
	 * @param x The IDL struct containing the values used to fill this row.
	 */
	void FlagCmdRow::setFromIDL (FlagCmdRowIDL x){
		try {
		// Fill the values from x.
	
		
	
		
		
			
		setTimeInterval(ArrayTimeInterval (x.timeInterval));
			
 		
		
	

	
		
		
			
		setType(string (x.type));
			
 		
		
	

	
		
		
			
		setReason(string (x.reason));
			
 		
		
	

	
		
		
			
		setLevel(x.level);
  			
 		
		
	

	
		
		
			
		setSeverity(x.severity);
  			
 		
		
	

	
		
		
			
		setApplied(x.applied);
  			
 		
		
	

	
		
		
			
		setCommand(string (x.command));
			
 		
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"FlagCmd");
		}
	}
#endif
	
	/**
	 * Return this row in the form of an XML string.
	 * @return The values of this row as an XML string.
	 */
	string FlagCmdRow::toXML() const {
		string buf;
		buf.append("<row> \n");
		
	
		
  	
 		
		
		Parser::toXML(timeInterval, "timeInterval", buf);
		
		
	

  	
 		
		
		Parser::toXML(type, "type", buf);
		
		
	

  	
 		
		
		Parser::toXML(reason, "reason", buf);
		
		
	

  	
 		
		
		Parser::toXML(level, "level", buf);
		
		
	

  	
 		
		
		Parser::toXML(severity, "severity", buf);
		
		
	

  	
 		
		
		Parser::toXML(applied, "applied", buf);
		
		
	

  	
 		
		
		Parser::toXML(command, "command", buf);
		
		
	

	
	
		
		
		buf.append("</row>\n");
		return buf;
	}

	/**
	 * Fill the values of this row from an XML string 
	 * that was produced by the toXML() method.
	 * @param x The XML string being used to set the values of this row.
	 */
	void FlagCmdRow::setFromXML (string rowDoc) {
		Parser row(rowDoc);
		string s = "";
		try {
	
		
	
  		
			
	  	setTimeInterval(Parser::getArrayTimeInterval("timeInterval","FlagCmd",rowDoc));
			
		
	

	
  		
			
	  	setType(Parser::getString("type","FlagCmd",rowDoc));
			
		
	

	
  		
			
	  	setReason(Parser::getString("reason","FlagCmd",rowDoc));
			
		
	

	
  		
			
	  	setLevel(Parser::getInteger("level","FlagCmd",rowDoc));
			
		
	

	
  		
			
	  	setSeverity(Parser::getInteger("severity","FlagCmd",rowDoc));
			
		
	

	
  		
			
	  	setApplied(Parser::getBoolean("applied","FlagCmd",rowDoc));
			
		
	

	
  		
			
	  	setCommand(Parser::getString("command","FlagCmd",rowDoc));
			
		
	

	
	
		
		} catch (IllegalAccessException err) {
			throw ConversionException (err.getMessage(),"FlagCmd");
		}
	}
	
	void FlagCmdRow::toBin(EndianOSStream& eoss) {
	
	
	
	
		
	timeInterval.toBin(eoss);
		
	

	
	
		
						
			eoss.writeString(type);
				
		
	

	
	
		
						
			eoss.writeString(reason);
				
		
	

	
	
		
						
			eoss.writeInt(level);
				
		
	

	
	
		
						
			eoss.writeInt(severity);
				
		
	

	
	
		
						
			eoss.writeBoolean(applied);
				
		
	

	
	
		
						
			eoss.writeString(command);
				
		
	


	
	
	}
	
void FlagCmdRow::timeIntervalFromBin(EndianISStream& eiss) {
		
	
		
		
		timeInterval =  ArrayTimeInterval::fromBin(eiss);
		
	
	
}
void FlagCmdRow::typeFromBin(EndianISStream& eiss) {
		
	
	
		
			
		type =  eiss.readString();
			
		
	
	
}
void FlagCmdRow::reasonFromBin(EndianISStream& eiss) {
		
	
	
		
			
		reason =  eiss.readString();
			
		
	
	
}
void FlagCmdRow::levelFromBin(EndianISStream& eiss) {
		
	
	
		
			
		level =  eiss.readInt();
			
		
	
	
}
void FlagCmdRow::severityFromBin(EndianISStream& eiss) {
		
	
	
		
			
		severity =  eiss.readInt();
			
		
	
	
}
void FlagCmdRow::appliedFromBin(EndianISStream& eiss) {
		
	
	
		
			
		applied =  eiss.readBoolean();
			
		
	
	
}
void FlagCmdRow::commandFromBin(EndianISStream& eiss) {
		
	
	
		
			
		command =  eiss.readString();
			
		
	
	
}

		
	
	FlagCmdRow* FlagCmdRow::fromBin(EndianISStream& eiss, FlagCmdTable& table, const vector<string>& attributesSeq) {
		FlagCmdRow* row = new  FlagCmdRow(table);
		
		map<string, FlagCmdAttributeFromBin>::iterator iter ;
		for (unsigned int i = 0; i < attributesSeq.size(); i++) {
			iter = row->fromBinMethods.find(attributesSeq.at(i));
			if (iter == row->fromBinMethods.end()) {
				throw ConversionException("There is not method to read an attribute '"+attributesSeq.at(i)+"'.", "FlagCmdTable");
			}
			(row->*(row->fromBinMethods[ attributesSeq.at(i) ] ))(eiss);
		}				
		return row;
	}
	
	////////////////////////////////
	// Intrinsic Table Attributes //
	////////////////////////////////
	
	

	
 	/**
 	 * Get timeInterval.
 	 * @return timeInterval as ArrayTimeInterval
 	 */
 	ArrayTimeInterval FlagCmdRow::getTimeInterval() const {
	
  		return timeInterval;
 	}

 	/**
 	 * Set timeInterval with the specified ArrayTimeInterval.
 	 * @param timeInterval The ArrayTimeInterval value to which timeInterval is to be set.
 	 
 	
 		
 	 * @throw IllegalAccessException If an attempt is made to change this field after is has been added to the table.
 	 	
 	 */
 	void FlagCmdRow::setTimeInterval (ArrayTimeInterval timeInterval)  {
  	
  	
  		if (hasBeenAdded) {
 		
			throw IllegalAccessException("timeInterval", "FlagCmd");
		
  		}
  	
 		this->timeInterval = timeInterval;
	
 	}
	
	

	

	
 	/**
 	 * Get type.
 	 * @return type as string
 	 */
 	string FlagCmdRow::getType() const {
	
  		return type;
 	}

 	/**
 	 * Set type with the specified string.
 	 * @param type The string value to which type is to be set.
 	 
 	
 		
 	 */
 	void FlagCmdRow::setType (string type)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->type = type;
	
 	}
	
	

	

	
 	/**
 	 * Get reason.
 	 * @return reason as string
 	 */
 	string FlagCmdRow::getReason() const {
	
  		return reason;
 	}

 	/**
 	 * Set reason with the specified string.
 	 * @param reason The string value to which reason is to be set.
 	 
 	
 		
 	 */
 	void FlagCmdRow::setReason (string reason)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->reason = reason;
	
 	}
	
	

	

	
 	/**
 	 * Get level.
 	 * @return level as int
 	 */
 	int FlagCmdRow::getLevel() const {
	
  		return level;
 	}

 	/**
 	 * Set level with the specified int.
 	 * @param level The int value to which level is to be set.
 	 
 	
 		
 	 */
 	void FlagCmdRow::setLevel (int level)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->level = level;
	
 	}
	
	

	

	
 	/**
 	 * Get severity.
 	 * @return severity as int
 	 */
 	int FlagCmdRow::getSeverity() const {
	
  		return severity;
 	}

 	/**
 	 * Set severity with the specified int.
 	 * @param severity The int value to which severity is to be set.
 	 
 	
 		
 	 */
 	void FlagCmdRow::setSeverity (int severity)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->severity = severity;
	
 	}
	
	

	

	
 	/**
 	 * Get applied.
 	 * @return applied as bool
 	 */
 	bool FlagCmdRow::getApplied() const {
	
  		return applied;
 	}

 	/**
 	 * Set applied with the specified bool.
 	 * @param applied The bool value to which applied is to be set.
 	 
 	
 		
 	 */
 	void FlagCmdRow::setApplied (bool applied)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->applied = applied;
	
 	}
	
	

	

	
 	/**
 	 * Get command.
 	 * @return command as string
 	 */
 	string FlagCmdRow::getCommand() const {
	
  		return command;
 	}

 	/**
 	 * Set command with the specified string.
 	 * @param command The string value to which command is to be set.
 	 
 	
 		
 	 */
 	void FlagCmdRow::setCommand (string command)  {
  	
  	
  		if (hasBeenAdded) {
 		
  		}
  	
 		this->command = command;
	
 	}
	
	

	
	////////////////////////////////
	// Extrinsic Table Attributes //
	////////////////////////////////
	
	///////////
	// Links //
	///////////
	
	
	/**
	 * Create a FlagCmdRow.
	 * <p>
	 * This constructor is private because only the
	 * table can create rows.  All rows know the table
	 * to which they belong.
	 * @param table The table to which this row belongs.
	 */ 
	FlagCmdRow::FlagCmdRow (FlagCmdTable &t) : table(t) {
		hasBeenAdded = false;
		
	
	

	

	

	

	

	

	

	
	
	
	
	

	

	

	

	

	

	

	
	
	 fromBinMethods["timeInterval"] = &FlagCmdRow::timeIntervalFromBin; 
	 fromBinMethods["type"] = &FlagCmdRow::typeFromBin; 
	 fromBinMethods["reason"] = &FlagCmdRow::reasonFromBin; 
	 fromBinMethods["level"] = &FlagCmdRow::levelFromBin; 
	 fromBinMethods["severity"] = &FlagCmdRow::severityFromBin; 
	 fromBinMethods["applied"] = &FlagCmdRow::appliedFromBin; 
	 fromBinMethods["command"] = &FlagCmdRow::commandFromBin; 
		
	
	
	}
	
	FlagCmdRow::FlagCmdRow (FlagCmdTable &t, FlagCmdRow &row) : table(t) {
		hasBeenAdded = false;
		
		if (&row == 0) {
	
	
	

	

	

	

	

	

	

			
		}
		else {
	
		
			timeInterval = row.timeInterval;
		
		
		
		
			type = row.type;
		
			reason = row.reason;
		
			level = row.level;
		
			severity = row.severity;
		
			applied = row.applied;
		
			command = row.command;
		
		
		
		
		}
		
		 fromBinMethods["timeInterval"] = &FlagCmdRow::timeIntervalFromBin; 
		 fromBinMethods["type"] = &FlagCmdRow::typeFromBin; 
		 fromBinMethods["reason"] = &FlagCmdRow::reasonFromBin; 
		 fromBinMethods["level"] = &FlagCmdRow::levelFromBin; 
		 fromBinMethods["severity"] = &FlagCmdRow::severityFromBin; 
		 fromBinMethods["applied"] = &FlagCmdRow::appliedFromBin; 
		 fromBinMethods["command"] = &FlagCmdRow::commandFromBin; 
			
	
			
	}

	
	bool FlagCmdRow::compareNoAutoInc(ArrayTimeInterval timeInterval, string type, string reason, int level, int severity, bool applied, string command) {
		bool result;
		result = true;
		
	
		
		result = result && (this->timeInterval.overlaps(timeInterval));
		
		if (!result) return false;
	

	
		
		result = result && (this->type == type);
		
		if (!result) return false;
	

	
		
		result = result && (this->reason == reason);
		
		if (!result) return false;
	

	
		
		result = result && (this->level == level);
		
		if (!result) return false;
	

	
		
		result = result && (this->severity == severity);
		
		if (!result) return false;
	

	
		
		result = result && (this->applied == applied);
		
		if (!result) return false;
	

	
		
		result = result && (this->command == command);
		
		if (!result) return false;
	

		return result;
	}	
	
	
	
	bool FlagCmdRow::compareRequiredValue(string type, string reason, int level, int severity, bool applied, string command) {
		bool result;
		result = true;
		
	
		if (!(this->type == type)) return false;
	

	
		if (!(this->reason == reason)) return false;
	

	
		if (!(this->level == level)) return false;
	

	
		if (!(this->severity == severity)) return false;
	

	
		if (!(this->applied == applied)) return false;
	

	
		if (!(this->command == command)) return false;
	

		return result;
	}
	
	
	/**
	 * Return true if all required attributes of the value part are equal to their homologues
	 * in x and false otherwise.
	 *
	 * @param x a pointer on the FlagCmdRow whose required attributes of the value part 
	 * will be compared with those of this.
	 * @return a boolean.
	 */
	bool FlagCmdRow::equalByRequiredValue(FlagCmdRow* x) {
		
			
		if (this->type != x->type) return false;
			
		if (this->reason != x->reason) return false;
			
		if (this->level != x->level) return false;
			
		if (this->severity != x->severity) return false;
			
		if (this->applied != x->applied) return false;
			
		if (this->command != x->command) return false;
			
		
		return true;
	}	
	
/*
	 map<string, FlagCmdAttributeFromBin> FlagCmdRow::initFromBinMethods() {
		map<string, FlagCmdAttributeFromBin> result;
		
		result["timeInterval"] = &FlagCmdRow::timeIntervalFromBin;
		result["type"] = &FlagCmdRow::typeFromBin;
		result["reason"] = &FlagCmdRow::reasonFromBin;
		result["level"] = &FlagCmdRow::levelFromBin;
		result["severity"] = &FlagCmdRow::severityFromBin;
		result["applied"] = &FlagCmdRow::appliedFromBin;
		result["command"] = &FlagCmdRow::commandFromBin;
		
		
			
		
		return result;	
	}
*/	
} // End namespace asdm
 
