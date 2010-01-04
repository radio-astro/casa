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
 } // end namespace asdm
