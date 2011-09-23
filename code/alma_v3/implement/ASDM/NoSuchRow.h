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
 * File NoSuchRow.h
 */
 
#ifndef NoSuchRow_CLASS
#define NoSuchRow_CLASS

#include <string>
using std::string;

#include <IntegerWrapper.h>
using asdm::Integer;

namespace asdm {

  /**
   * Generate an exception when an expected row cannot be found. 
   */
  class NoSuchRow {

  public:
    /**
     * An empty CTOR.
     */
    NoSuchRow();

    /**
     * Create an exception when an expected row cannot be found.
     * @param rowNumber The row number that cannot be found.
     * @param tableName The table being searched.
     */
    NoSuchRow(int rowNumber, string tableName);

    /**
     * Create an exception when an expected row cannot be found.
     * @param key The key of the row that cannot be found.
     * @param tableName The table being searched.
     */
    NoSuchRow(string key, string tableName);

    /**
     * Create an exception when an expected link cannot be found.
     * @param N The link number that cannot be found.
     * @param toTableName The table to which the link is directed.
     * @param fromTableName The table from which the link is directed.
     */
    NoSuchRow(int N, string toTableName, string fromTableName);

    /**
     * Create an exception when an expected link cannot be found.
     * @param key The key of the link that cannot be found.
     * @param toTableName The table to which the link is directed.
     * @param fromTableName The table from which the link is directed.
     */
    NoSuchRow(string key, string toTableName, string fromTableName);

    /**
     * Create an exception when an optional link does not exist.
     * @param toTableName The table to which the link is directed.
     * @param fromTableName The table from which the link is directed.
     * @param option Is not really used.
     */
    NoSuchRow(string toTableName, string fromTableName, bool option);
	

    /**
     * The DTOR.
     */
    virtual ~NoSuchRow();

    /**
     * @return a text describing the exception.
     */
    string getMessage() const;
	
  protected:

    string message;

  private:
    static const string ITSNAME;

  };
} // End namespace asdm

#endif /* NoSuchRow_CLASS */
