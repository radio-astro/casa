//# TBXMLDriver.h: Driver for converting table data into an XML String.
//# Copyright (C) 2005
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: $
#ifndef TBXMLDRIVER_H_
#define TBXMLDRIVER_H_

/*
#include <casa/BasicSL/String.h>
#include <tables/Tables/Table.h>

#include <casa/namespace.h>

// <summary>
// Driver for converting table data into an XML String.
// </summary>
//
// <synopsis>
// TBXMLDriver, initially used in the CasaJNI code for the Java table browser,
// is an interface to read from a table on disk and return XML which
// holds the requested data.  There is one public method which takes a
// command in string form and returns the XML result.  There is infrastructure
// to access remote tables, but it is not implemented.
// </synopsis>

/* Examples of commands:
 *     
 * To load rows from the table:
 *      send.table.query # SELECT FROM /casa/table/ <START = 0 number = 1000>
 * where # is the length of the string starting from the next character.
 * 
 * To load rows from given fields in the table:
 *      send.table.query # SELECT FIELD1,FIELD2 FROM /casa/table/
 *      <START = 0 number = 1000>
 * where # is the length of the string starting from the next character.
 * 
 * For the previous two commands, for array data only the shape is returned
 * for arrays with dimension > 2.  To return the full array data, use
 * send.table.full instead of send.table.query.
 * 
 * To view a data array:
 *      send.table.array #<ARRAYINFO>
 *          <QUERY> SELECT FROM /casa/table/ </QUERY>
 *          <ROW> 5 </ROW> <COLUMN> 5 </COLUMN> <TYPE> TpArrayInt </TYPE> 
 *      </ARRAYINFO>
 * where # is the length of the string starting from the next character.
 * 
 * To update data in the table:
 *      send.table.updat #<QUERY> SELECT FROM /casa/table/ </QUERY>
 *      <COMMAND>
 *          <UPDATE row = 5 col = 5 val = "newValue" >
 *      </COMMAND>
 * where # is the length of the string starting from the next character.
 * 
 * To update data in an array cell:
 *      send.table.updat #<QUERY> SELECT FROM /casa/table/ </QUERY>
 *      <COMMAND>
 *          <ARRAYUPDATE row = 5 col = 5 >
 *              <ARRAYCELLUPDATE coordinates = [ 4 2 ] val = "newValue" >
 *          </ARRAYUPDATE>
 *      </COMMAND>
 * where # is the length of the string starting from the next character.
 * 
 * To insert rows:
 *      send.table.updat #<QUERY> SELECT FROM /casa/table/ </QUERY>
 *      <COMMAND>
 *          <ADDROW>
 *          <ADDROW>
 *      </COMMAND>
 * where # is the length of the string starting from the next character.
 * 
 * To delete rows:
 *      send.table.updat #<QUERY> SELECT FROM /casa/table/ </QUERY>
 *      <COMMAND>
 *          <DELROW 10 >
 *          <DELROW 5 >
 *      </COMMAND
 *      
 */

/*
class TBXMLDriver {
public:
    // Takes a command in pseudo-TaQL format and returns XML in pseduo-VOTable
    // format.  For command examples, see TBXMLDriver.h code comments.
    static String dowork(const char* buff);

private:
    // Indicates whether debug information should be printed or not.
    static const bool showdebug;
    
    // Buffer size.
    static const int BUF_SIZE = 2048;
    
    // Packet size.
    static const int PacketSize = 4096;
    
    // Creates the XML representation of a VOTable from the given parameters.
    static String createVOTab(String tablename, int totalrows,
                             Vector<String> colnames, Vector<String> datatype,
                             String records, String keyword, Bool insRowOk,
                             Bool delRowOk, String columnkeywords);
    
    // Creates the XML representation of a keyword from the given parameters.
    static String createKeyword(TableRecord &trec, int a);
    
    // Sends the given String.
    static int SendData(int fd, const String &hits);
    
    // Makes the return result from the given String.
    static char* mkReturnResult(const String &hits);
    
    // Sets up communication.
    static Bool setupComm(Int &fd);
    
    // Reads input into the given char*.
    static int readn(int fd, char *ptr, int nbytes);
    
    // Writes output to the given char*.
    static int writen(int fd, char *ptr, int nbytes);
};
*/

#endif /* TBXMLDRIVER_H_*/
