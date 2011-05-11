#ifndef MISC_H
#define MISC_H
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
 *
 * File Misc.h
 */
 
 namespace asdm {
 	/**
	 * Some utility methods to manipulate directories.
	 */
	 
	/**
	 * Returns whether the specified directory exists.
	 *
	 * Example:
	 * --------
	 *          if (!directoryExists("output"))
	 *          {
	 *              createDirectory("output");
	 *          }
	 */ 
	bool directoryExists(const char* dir);
	
	/**
	 * Creates the specified directory. Fails if the path leading to
	 * this directory does not exist.
	 *
	 * Example:
	 * --------
	 *          createDirectory("output");	
	 */
	bool createDirectory(const char* dir);
	
	/**
	 * Creates a complete path.
	 *
	 * Example:
	 * --------
	 *           ("output/sample1/temperature0/");
	 */
	bool createPath(const char* path);
	
	
	/**
	 * Swap bytes 
	 */
#define ByteSwap5(x) ByteSwap((unsigned char *) &x,sizeof(x))	 
	void ByteSwap(unsigned char * b, int n);
 	
 } // end namespace asdm
 #endif  // MISC_H
