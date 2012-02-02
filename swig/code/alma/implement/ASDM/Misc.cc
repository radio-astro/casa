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
 * File Misc.cpp
 */

#include <Misc.h>
 
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <algorithm> //required for std::swap
#include <iostream>
namespace asdm {
  bool directoryExists(const char* dir) {
    DIR* dhandle = opendir(dir);

    if (dhandle != NULL) {
      closedir(dhandle);
      return true;
    }
    else {
      return false;
    }
  }

  bool createDirectory(const char* dir) { 
    return mkdir(dir, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0;
  }
	
  bool createPath(const char* path) {
    char localpath[256];
    strcpy(localpath, path);

    char directory[256];
    if (path[0] == '/') {
      strcpy(directory, "/");
    }
    else {
      strcpy(directory, "");
    }
    	
    char* pch = strtok(localpath, "/");
    while (pch != NULL) {
      strcat(directory, pch);
      strcat(directory, "/");
      if (!directoryExists(directory) && !createDirectory(directory)) {
	return false;
      }
      pch = strtok(NULL, "/");
    }
    return true;
  }
    
  void ByteSwap(unsigned char * b, int n) {
    register int i = 0;
    register int j = n-1;
    while (i<j) {
      std::swap(b[i], b[j]);
      i++, j--;
    }
  }

#if defined(__APPLE__)
  const ByteOrder* ByteOrder::Little_Endian = new ByteOrder("Little_Endian", __DARWIN_LITTLE_ENDIAN);
  const ByteOrder* ByteOrder::Big_Endian = new ByteOrder("Big_Endian", __DARWIN_BIG_ENDIAN);
#else 
  const ByteOrder* ByteOrder::Little_Endian = new ByteOrder("Little_Endian", __LITTLE_ENDIAN);
  const ByteOrder* ByteOrder::Big_Endian = new ByteOrder("Big_Endian", __BIG_ENDIAN);
#endif
  const ByteOrder* ByteOrder::Machine_Endianity = ByteOrder::machineEndianity();

  ByteOrder::ByteOrder(const string& name, int endianity):
    name_(name), endianity_(endianity){;}

  ByteOrder::~ByteOrder() {;}

  const ByteOrder* ByteOrder::machineEndianity() {
#if defined(__APPLE__)
    if (__DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN)
#else 
      if (__BYTE_ORDER == __LITTLE_ENDIAN)
#endif
	return Little_Endian;
      else
	return Big_Endian;
  }
  
  string ByteOrder::toString() const {
    return name_;
  }

  const ByteOrder* ByteOrder::fromString(const string &s) {
    if (s == "Little_Endian") return Little_Endian;
    if (s == "Big_Endian") return Big_Endian;
    return 0;
  }

  string uniqSlashes(const string & s) {
	  string result;
	  char c;
	  bool inslash = false;
	  size_t indexi=0;

	  while (indexi < s.size()) {
		  if ((c = s.at(indexi)) != '/') {
			  inslash = false;
			  result.push_back(c);
		  }
		  else
			if (inslash == false) {
				result.push_back(c);
				inslash = true;
			}
		  indexi++;
	  }
	  return result;
  }
} // end namespace asdm
 
 
