//# VlaSelectInfo.cc:  this defines VlaSelectInfo, which ...
//# 
//# Copyright (C) 1997,2001
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
//# $Id$

//# Includes

#include <nrao/VLA/VlaSelectInfo.h>
#include <string.h>
#include <casa/sstream.h>
#include <casa/iostream.h>
 
Int VlaSelectInfo::timeRangeOK(Double theTime){
   Int rStat(1);
   if(theTime < startTime)
      rStat = 0;
   if(theTime > stopTime)
      rStat = -1;
   return(rStat);
}
 
Int VlaSelectInfo::subarrayOK(int subarrayID){
   Int r_status(1);
   if(subarrays){
      r_status = 0;
      for(Int i=0; i<subarrayCount; i++){
        if(subarrays[i] == subarrayID)
           r_status = 1;
           break;
      }
   }
   return r_status;
}
 
Int VlaSelectInfo::observingModeOK(const Char *observingMode){
   Int r_status(1);
   if(modes){
      r_status = 0;
      Char AMode[3]; AMode[2] ='\0';
      strncpy(AMode, observingMode, 2);
      if(!strcmp(AMode, "  "))
         strcpy(AMode, "C");
      if(!strstr(modes, AMode))
         r_status = 1;
   }
   return r_status;
}
 
Int VlaSelectInfo::sourceOK(const Char *sourceName){
   Int r_status(1);
   if(sources){
      r_status = 0;
      Char ASource[17];
      ASource[16] = '\0';
      strcpy(ASource, sourceName);
      if(!strstr(sources, ASource))
         r_status = 1;
   }
   return r_status;
}
 
Int VlaSelectInfo::programIDOK(const Char *programID){
   Int r_status (1) ;
   if(programs){
      r_status = 0;
      Char AProgram[7];
      AProgram[6] = '\0';
      strcpy(AProgram, programID);
      if(!strstr(programs, AProgram))
         r_status = 1;
   }
   return r_status;
}
 
Int *VlaSelectInfo::antennasOK(Int numAntennas, Int *antennaIDs,
                                Int *the_flags){
   if(antennas){
      for(Int j=0; j<numAntennas;j++){
         the_flags[j] = 0;
      }
      for(Int i=0;i<numAntennas;i++){
         for(Int j=0; j<numAntennas;j++){
            if(antennaIDs[i] == antennas[j]){
               the_flags[i] = 1;
               break;
            }
         }
      }
   }
   return the_flags;}
 
 
void VlaSelectInfo::setValues(const Char *buffer){
   Char keyword[80];
   istringstream iss(buffer);
   iss >> keyword;
   if(!strcasecmp(keyword, "ALL.ANTENNAS")){
     antennas = new int[24];
     iss >> antennas[0];
     antennaCount = 0;
     while(iss.gcount() > 0){
        iss >> antennas[++antennaCount];
     }
   } else if(!strcasecmp(keyword, "ALL.TIMERANGE")){
     iss >> startTime >> stopTime;
   } else if(!strcasecmp(keyword, "ALL.SUBARRAY")){
     subarrays = new int[4];
     iss >> subarrays[0];
     subarrayCount = 0;
     while(1){
        iss >> subarrays[++subarrayCount];
        if(iss.gcount() < 1)
           break;
        cerr << iss.gcount() << endl;
     }
   } else if(!strcasecmp(keyword, "ALL.MODE")){
     modes = new Char[81];
     iss.getline(modes, 80);
   } else if(!strcasecmp(keyword, "ALL.SOURCES")){
     sources = new Char[81];
     iss.getline(sources, 80);
   } else if(!strcasecmp(keyword, "ALL.PROGRAMID")){
     programs = new Char[81];
     iss.getline(programs, 80);
   }
   return; }
 
 
