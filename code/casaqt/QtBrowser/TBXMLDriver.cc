//# TBXMLDriver.cc: Driver for converting table data into an XML String.
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
/*
#include <casaqt/QtBrowser/TBXMLDriver.h>
#include <casaqt/QtBrowser/TBConstants.h>

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>

#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableColumn.h>
#include <casa/Containers/RecordField.h>
#include <casa/Arrays/Vector.h>
#include <casa/iostream.h>
#include <casa/sstream.h>
#include <iostream>
#include <string>
#include <casa/Exceptions/Error.h>
#include <sstream>

#include <casa/namespace.h>

const bool TBXMLDriver::showdebug = false;

char* TBXMLDriver::mkReturnResult(const String &hits) {
   int length = htonl(hits.length());
   char *termString = "thats.all.folks\n";
   char *retBuf = new char[length+sizeof(length)+hits.length()+
                           strlen(termString)];
   memcpy(retBuf, &length, sizeof(length));
   memcpy(retBuf+sizeof(length), hits.chars(), hits.length());
   memcpy(retBuf+sizeof(length)+hits.length(), termString, strlen(termString));
   return retBuf;
}

Bool TBXMLDriver::setupComm(Int &fd) {
    Bool rstat = false;
    fd = 1;
    return rstat;
}

String TBXMLDriver::dowork(const char *buff) {
   int bytesToRead(0);
   String initQuery;
   String keywords;
   //int in_bytes;
   int fd;
   String rstat;

   if(!setupComm(fd)){
      //initQuery = buff;
          istringstream istrbuff(buff);
          istrbuff >> initQuery >> bytesToRead;
                  // If BUF_SIZE < bytesToRead we've got to do something else.
      // null terminate things
          //*(buff+bytesToRead) = '\0';
          String query(buff+20,bytesToRead);
      TBConstants::dprint(TBConstants::DEBUG_LOW, "querrying: " + query);
          try {
        if(initQuery== "send.table.array"){
          
          TBConstants::dprint(TBConstants::DEBUG_LOW, "arrayInfo\n" + query);
          string qu="";
          string tag;
        
          int row;
          String type;
          int col;
          stringstream stream;
          stream<<query;
          stream>>tag;
          TBConstants::dprint(TBConstants::DEBUG_LOW, "first tag:" + tag);
          stream>>tag;
         
          stream>>tag;
          while(tag!="</QUERY>"){
        qu.append(tag);
        qu.append(" ");
        stream>>tag;
          }
          
          
          stream>>tag;
          stream>>row;
          TBConstants::dprint(TBConstants::DEBUG_LOW, "row num: " +
                              TBConstants::itoa(row));
          stream>>tag;
          stream>> tag;
          stream >>col;
          TBConstants::dprint(TBConstants::DEBUG_LOW, "col num: " +
                              TBConstants::itoa(col));
          stream >>tag;
          stream>> tag;
          stream>>type;

          stringstream diff;
          diff<<qu;
          string word;
          string word2;
          string name;
          diff>>word;
          diff>>word2;
          diff>>name;
          Table result;
          //  TBConstants::dprint(TBConstants::DEBUG_LOW, "words: " + word +
          //                      " " + word2 + " " + name);
          
          String qur(qu);
          
          if(word=="SELECT"&&word2=="FROM"&&(!qur.contains("WHERE"))){
        result=Table(name);
          } else{
        result = tableCommand(qu);
          }
          
         
          TableDesc td = result.tableDesc();
          ColumnDesc cd = td.columnDesc(col);
          String columnName = cd.name();
         
          
          
          String arrayInfo;
          ostringstream hits;
          
          
          if(type==TBConstants::TYPE_ARRAY_BOOL){
    
        ROArrayColumn< Bool > column(result, columnName);
        Array<Bool > array = column(row);
        hits<<array;
        
          } else if(type==TBConstants::TYPE_ARRAY_FLOAT){
    
        ROArrayColumn< Float > column(result, columnName);
        Array<Float > array = column(row);
        hits<<array;
    
          } else if(type==TBConstants::TYPE_ARRAY_DOUBLE){
    
        ROArrayColumn< Double > column(result, columnName);
        Array<Double > array = column(row);
        hits<<array;
        
          } else if(type==TBConstants::TYPE_ARRAY_UCHAR){
    
        ROArrayColumn< uChar > column(result, columnName);
        Array<uChar > array = column(row);
        hits<<array;
          } else if(type==TBConstants::TYPE_ARRAY_SHORT){
    
        ROArrayColumn< Short > column(result, columnName);
        Array<Short > array = column(row);
        hits<<array;
          } else if(type==TBConstants::TYPE_ARRAY_INT){
    
        ROArrayColumn< Int > column(result, columnName);
        Array<Int > array = column(row);
        hits<<array;
        
          } else if(type==TBConstants::TYPE_ARRAY_UINT){
    
        ROArrayColumn< uInt > column(result, columnName);
        Array<uInt > array = column(row);
        hits<<array;
          } else if(type==TBConstants::TYPE_ARRAY_COMPLEX){
    
        ROArrayColumn< Complex > column(result, columnName);
        Array<Complex > array = column(row);
        hits<<array;
          } else if(type==TBConstants::TYPE_ARRAY_DCOMPLEX){
        ROArrayColumn< DComplex > column(result, columnName);
        Array<DComplex > array = column(row);
        hits<<array;
        
          } else if(type==TBConstants::TYPE_ARRAY_STRING){
    
        ROArrayColumn< String > column(result, columnName);
        Array<String > array = column(row);
        hits<<array;
          }
          rstat += hits.str();
          
          
          //if(SendData(fd,  hits) == -1){
          //TBConstants::dprint(TBConstants::DEBUG_LOW, "Error sending data0");
         //rstat = -1;
         //return(rstat);
          //}

        } else if(initQuery== "send.table.updat"){
          TBConstants::dprint(TBConstants::DEBUG_LOW,
                              "Attempting to update table:\n" + query);
          
          string tag;
          string content;
          stringstream stream;
          stream<<query;
          stream>>tag;
          TBConstants::dprint(TBConstants::DEBUG_LOW, "first tag: " + tag);
          stream>>tag;
          TBConstants::dprint(TBConstants::DEBUG_LOW,
                              "should be select: " + tag);
          
          while(tag!="</QUERY>"){
        content.append(tag);
        content.append(" ");
        stream>>tag;
        TBConstants::dprint(TBConstants::DEBUG_LOW, tag);
          }

          TBConstants::dprint(TBConstants::DEBUG_LOW, "query is: " + content);
          
          stringstream diff;
          diff<<content;
          string word;
          string word2;
          string name;
          diff>>word;
          diff>>word2;
          diff>>name;
          Table subtable;
          // TBConstants::dprint(TBConstants::DEBUG_LOW, "update words: " +
          //                     word + " " + word2 + " " + name);

          String qur(content);
          
          if(word=="SELECT"&&word2=="FROM"&&(!qur.contains("WHERE"))){
        subtable = Table(name);
          } else{
        subtable =tableCommand(content);
          }

          subtable.reopenRW();

          /////////////////

          ROTableRow row(subtable);
          Vector<String> colNames = row.columnNames();
          void **fieldPtrs = (void **)new uInt*[colNames.nelements()];

          Vector<String> dataTypes(colNames.nelements());
          {
        
        for(int i=0;i<Int(colNames.nelements());i++){
          // TBConstants::dprint(TBConstants::DEBUG_LOW, "entering for 1");
          switch(row.record().type(row.record().fieldNumber(colNames(i)))){
          case TpString :
            fieldPtrs[i] =
              new RORecordFieldPtr<String>(row.record(), colNames(i));
            
            dataTypes[i]=TBConstants::TYPE_STRING;
            break;
            
          case TpInt :
            fieldPtrs[i] =
              new RORecordFieldPtr<Int>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_INT;
            
            break;
           
          case TpFloat :
            fieldPtrs[i] =  (void *)
              new RORecordFieldPtr<Float>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_FLOAT;
            break;
           
          case TpDouble :
            fieldPtrs[i] =
              new RORecordFieldPtr<Double>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_DOUBLE;
            break;

          case TpBool :
            fieldPtrs[i] =  (void *)
              new RORecordFieldPtr<Bool>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_BOOL;
            
            break;
            
          case TpUChar :
            fieldPtrs[i] =
              new RORecordFieldPtr<uChar>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_UCHAR;
            break;
            
          case TpShort :
            fieldPtrs[i] =
              new RORecordFieldPtr<Short>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_SHORT;
            break;

          case TpUInt :
            fieldPtrs[i] =
              new RORecordFieldPtr<uInt>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_UINT;
            break;
            
                   case TpComplex :
             fieldPtrs[i] =
               new RORecordFieldPtr<Complex>(row.record(), colNames(i));
             dataTypes[i]=TBConstants::TYPE_COMPLEX;
                      break;
              
          case TpDComplex :
            fieldPtrs[i] =
              new RORecordFieldPtr<DComplex>(row.record(), colNames(i));
              dataTypes[i]=TBConstants::TYPE_DCOMPLEX;
              break;

          case TpArrayDouble :
          
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Double> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_DOUBLE;
            break;

          case TpArrayBool :
            
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Bool> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_BOOL;
            break;

          case TpArrayChar :
            
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Char> >(row.record(), colNames(i));
            dataTypes[i]="unsupported";
            break;
            
          case TpArrayUChar :
            
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<uChar> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_UCHAR;
            break;
            
          case TpArrayShort :
            
            fieldPtrs[i] =
            new RORecordFieldPtr<Array<Short> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_SHORT;
            break;
            
          case TpArrayInt :
            
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Int> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_INT;
            break;
            
          case TpArrayUInt :
            
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<uInt> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_UINT;
            break;
            
          case TpArrayFloat :
            
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Float> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_FLOAT;
            break;

          case TpArrayComplex :
            
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Complex> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_COMPLEX;
            break;
            
          case TpArrayDComplex :
            
            fieldPtrs[i] =
             new RORecordFieldPtr<Array<DComplex> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_DCOMPLEX;
            break;
            
          case TpArrayString :
            
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<String> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_STRING;
            break;
            
          default:
            TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "index i: " + TBConstants::itoa(i));
            TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "columnName: " + colNames(i));
            TableDesc td = subtable.tableDesc();
            ColumnDesc cd = td.columnDesc(colNames(i));
            TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "datatype is: " + cd.dataType());

            throw(AipsError("atabd: unexpected type, this should never happ"));
           
            break;
          }
        }
          }
        //////////////////////
         
          int ncol = subtable.tableDesc().ncolumn();
          TableColumn* colarr = new TableColumn[ncol];
          for(int a=0; a<ncol; a++){
        colarr[a].attach(subtable, a);
          }
          
          stream>>tag;
          TBConstants::dprint(TBConstants::DEBUG_LOW,
                              "should be command: " + tag);
          int rownum;
          int colnum;
          double doubleval;
          int intval;
          bool boolval;
          float floatval;
          String strval;
          while(stream>>tag){
        if(tag!="</COMMAND>"){
          if(tag=="<UPDATE"){
            TBConstants::dprint(TBConstants::DEBUG_LOW, "update cell");
            stream>>tag;
            TBConstants::dprint(TBConstants::DEBUG_LOW, "mark row?: " + tag);
            
            stream>>tag;
            TBConstants::dprint(TBConstants::DEBUG_LOW, tag);
            
            stream>>rownum;
            TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "row number: " + rownum);
            
            stream>>tag;
            TBConstants::dprint(TBConstants::DEBUG_LOW, tag);
            
            stream>>tag;
            TBConstants::dprint(TBConstants::DEBUG_LOW, tag);
            
            stream>>colnum;
            TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "col number: " + colnum);
            
            stream>>tag;
            TBConstants::dprint(TBConstants::DEBUG_LOW, tag);
            
            stream>>tag;
            TBConstants::dprint(TBConstants::DEBUG_LOW, tag);

            // val = "value"
            unsigned int i = stream.str().find('"', stream.tellg());
            unsigned int j = stream.str().find('>', stream.tellg());
            j = stream.str().find_last_of('"', j);
            String value = stream.str().substr(i + 1, j - i - 1);

            stringstream ss;
            ss << value;
            stream.seekg(j + 1);
            
            if(dataTypes[colnum]==TBConstants::TYPE_DOUBLE){
              
              ColumnDesc desc = colarr[colnum].columnDesc();
              
              if(desc.comment()==TBConstants::COMMENT_DATE){
            ss>>strval;
            TBConstants::dprint(TBConstants::DEBUG_LOW, "date is: " + strval);

            double dval = TBConstants::date(strval);

//          int index = strval.find("-",0);
//          String year = strval.at(0, index);
//          
//          int index2 = strval.find("-",index+1);
//          String month = strval.at(index+1, index2-index-1);
//          
//          index = strval.find("-",index2+1);
//          String day = strval.at(index2+1, index-index2-1);
//          
//          index2 = strval.find(":", index+1);
//          String hour = strval.at(index+1, index2-index-1);
//          
//          index = strval.find(":",index2+1);
//          String min = strval.at(index2+1, index-index2-1);
//          
//          String sec =strval.at(index+1, strval.length()-index-1);
//          
//          stringstream converter;
//          uInt iyear;
//          uInt imonth;
//          uInt iday;
//          uInt ihour;
//          uInt imin;
//          double isec;
//
//          converter<<year;
//          converter>>iyear;
//          converter.clear();
//          converter<<month;
//          converter>>imonth;
//          converter.clear();
//          converter<<day;
//          converter>>iday;
//          converter.clear();
//          converter<<hour;
//          converter>>ihour;
//          converter.clear();
//          converter<<min;
//          converter>>imin;
//          converter.clear();
//          converter<<sec;
//          converter>>isec;
//          
//          Time temptime(iyear, imonth, iday, ihour, imin, isec);
//            stringstream tempss;
//            tempss << temptime;
//            TBConstants::dprint(TBConstants::DEBUG_LOW,
//                              "the date is: " + tempss.str());
//          const double dval = (temptime.julianDay()-2400000.5)*86400;
            
            colarr[colnum].putScalar((uInt)rownum, dval);
              } else{
                 TBConstants::dprint(TBConstants::DEBUG_LOW,
                                     "col " + TBConstants::itoa(colnum) +
                                     " = double");
                 ss>>doubleval;
              
                 TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "newval is: " + TBConstants::dtoa(doubleval));
                 const double dval = doubleval;
                 colarr[colnum].putScalar((uInt)rownum, dval);
              }
            } else if(dataTypes[colnum]==TBConstants::TYPE_FLOAT){

              TBConstants::dprint(TBConstants::DEBUG_LOW,
                            "col " + TBConstants::itoa(colnum) + " = float");
              ss>>floatval;

              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                  "newval is: " + TBConstants::ftoa(floatval));
              const float fval = floatval;
              colarr[colnum].putScalar((uInt)rownum, fval);
              
            } else if(dataTypes[colnum]==TBConstants::TYPE_INT){
              
              TBConstants::dprint(TBConstants::DEBUG_LOW, "col " +
                                  TBConstants::itoa(colnum) + " = int");
              ss>>intval;

              TBConstants::dprint(TBConstants::DEBUG_LOW, "newval is: " +
                                  TBConstants::itoa(intval));
              const Int ival= intval;
              colarr[colnum].putScalar((uInt)rownum,ival);
              
            } else if(dataTypes[colnum]==TBConstants::TYPE_BOOL){
              
              TBConstants::dprint(TBConstants::DEBUG_LOW, "col " +
                                 TBConstants::itoa(colnum) + " = bool");
              ss>>boolval;

              TBConstants::dprint(TBConstants::DEBUG_LOW, "newval is: " +
                                  String::toString(boolval));
              const Bool bval=boolval;
              colarr[colnum].putScalar((uInt)rownum, bval);
              
            } else if(dataTypes[colnum]==TBConstants::TYPE_STRING){
              
              TBConstants::dprint(TBConstants::DEBUG_LOW, "col " +
                                  TBConstants::itoa(colnum) + " = string");
              strval = value;

              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                  "newval is: " + strval);
              const String strival=strval;
              colarr[colnum].putScalar((uInt)rownum, strival);
              
            } else if(dataTypes[colnum]==TBConstants::TYPE_COMPLEX) {
              
              ss>>strval;
              int comma = strval.find(",");
              String real = strval.at(1,comma-1);
              String imag = strval.at(comma+1, strval.length()-1-comma);
              stringstream converter;
              stringstream converter2;
              float dreal;
              float dimag;
              converter<<real;
              converter>>dreal;
              converter2<<imag;
              converter2>>dimag;
              Complex x(dreal,dimag);

              colarr[colnum].putScalar((uInt)rownum, x);
            } else if(dataTypes[colnum]==TBConstants::TYPE_DCOMPLEX) {
              ss>>strval;
              int comma = strval.find(",");
              String real = strval.at(1,comma-1);
              String imag = strval.at(comma+1, strval.length()-1-comma);
              stringstream converter;
              stringstream converter2;
              double dreal;
              double dimag;
              converter<<real;
              converter>>dreal;
              converter2<<imag;
              converter2>>dimag;
              DComplex x(dreal,dimag);
              colarr[colnum].putScalar((uInt)rownum, x);
            } else{

              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                  "unknown type: " + dataTypes[colnum]);
              break;
              
            }
            stream>>tag;
            TBConstants::dprint(TBConstants::DEBUG_LOW, "should be >");
          } else if(tag=="<ARRAYUPDATE"){

            TBConstants::dprint(TBConstants::DEBUG_LOW, "update array");
            stream>>tag;
            TBConstants::dprint(TBConstants::DEBUG_LOW, "marker row?: " + tag);
            
            stream>>tag;
            TBConstants::dprint(TBConstants::DEBUG_LOW, tag);
            
            stream>>rownum;
            TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "row number: " + TBConstants::itoa(rownum));
            
            stream>>tag;
            TBConstants::dprint(TBConstants::DEBUG_LOW, tag);
            
            stream>>tag;
            TBConstants::dprint(TBConstants::DEBUG_LOW, tag);
            
            stream>>colnum;
            TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "col number: " + TBConstants::itoa(colnum));
            TableDesc td =subtable.tableDesc();
            ColumnDesc cd = td.columnDesc(colnum);
            DataType type =cd.dataType();
            String name = cd.name();
            stream>>tag;
            int coord;
            
            if(type==TpBool){
              
              ArrayColumn<Bool> arraycol(subtable, name);
              Array<Bool> array =arraycol(rownum);
              int dim = array.ndim();
              
              Bool value;
              bool val;
              stream>>tag; //<ARRAYCELLUPDATE
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                  "should be <ARRAYCELLUPDATE: " + tag);
              while(tag!="</ARRAYUPDATE>"){
            if(tag=="<ARRAYCELLUPDATE"){
              stream>>tag; //coordinates
              stream>>tag; //=
              stream>>tag; //[
              IPosition coordinates((uInt)dim);
              for(int abc=0;abc<dim;abc++){
                stream>>coord;
                
                coordinates(abc)=coord;
              }
              stream>>tag; // ]
              stream>>tag; //val
              stream>>tag; //=
              stream>>val;
              value=val;
              stream>>tag; //>
              stream>>tag; // either <ARRACELLUPDATE or </ARRAYUPDATE
              ostringstream def;
              def<<coordinates<<" = "<<value;
              String tester =def.str();
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                  "array cell action setting: " + tester);
              Bool before = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "before assignment: "
                                  + String::toString(before));
              
              array(coordinates) =value;
              Bool after = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "after assingment: "
                                  + String::toString(after));
            }
            
              }
              arraycol.put(rownum, array);
            } else if(type==TpUChar){
              
              ArrayColumn<uChar> arraycol(subtable, name);
              Array<uChar> array =arraycol(rownum);
              int dim = array.ndim();
              
              uChar value;
              char val;
              stream>>tag; //<ARRAYCELLUPDATE
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                    "should be <ARRAYCELLUPDATE: " + tag);
              while(tag!="</ARRAYUPDATE>"){
            if(tag=="<ARRAYCELLUPDATE"){
              stream>>tag; //coordinates
              stream>>tag; //=
              stream>>tag; //[
              IPosition coordinates((uInt)dim);
              for(int abc=0;abc<dim;abc++){
                stream>>coord;
                
                coordinates(abc)=coord;
              }
              stream>>tag; // ]
              stream>>tag; //val
              stream>>tag; //=
              stream>>val;
              value=val;
              stream>>tag; //>
              stream>>tag; // either <ARRACELLUPDATE or </ARRAYUPDATE
              ostringstream def;
              def<<coordinates<<" = "<<value;
              String tester =def.str();
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                    "array cell action setting: " + tester);
              uChar before = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "before assignment: "
                                + String::toString(before));
              
              array(coordinates) =value;
              uChar after = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "after assignment: "
                                    + String::toString(after));
            }
            
              }
              arraycol.put(rownum, array);
         
            } else if(type==TpShort){
              
              ArrayColumn<Short> arraycol(subtable, name);
              Array<Short> array =arraycol(rownum);
              int dim = array.ndim();
              
              Short value;
              short val;
              stream>>tag; //<ARRAYCELLUPDATE
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                    "should be <ARRAYCELLUPDATE: " + tag);
              while(tag!="</ARRAYUPDATE>") {
            if(tag=="<ARRAYCELLUPDATE"){
              stream>>tag; //coordinates
              stream>>tag; //=
              stream>>tag; //[
              IPosition coordinates((uInt)dim);
              for(int abc=0;abc<dim;abc++){
                stream>>coord;
                
                coordinates(abc)=coord;
              }
              stream>>tag; // ]
              stream>>tag; //val
              stream>>tag; //=
              stream>>val;
              value=val;
              stream>>tag; //>
              stream>>tag; // either <ARRACELLUPDATE or </ARRAYUPDATE
              ostringstream def;
              def<<coordinates<<" = "<<value;
              String tester =def.str();
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                    "array cell action setting: " + tester);
              Short before = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "before assignment: "
                                    + String::toString(before));
              
              array(coordinates) =value;
              Short after = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "after assignment: "
                                    + String::toString(after));
            }
            
              }
              arraycol.put(rownum, array);
            } else if(type==TpInt){
              
              ArrayColumn<Int> arraycol(subtable, name);
              Array<Int> array =arraycol(rownum);
              int dim = array.ndim();
              
              Int value;
              int val;
              stream>>tag; //<ARRAYCELLUPDATE
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                            "should be <ARRAYCELLUPDATE: " + tag);
              while(tag!="</ARRAYUPDATE>"){
            if(tag=="<ARRAYCELLUPDATE"){
              stream>>tag; //coordinates
              stream>>tag; //=
              stream>>tag; //[
              IPosition coordinates((uInt)dim);
              for(int abc=0;abc<dim;abc++){
                stream>>coord;
                
                coordinates(abc)=coord;
              }
              stream>>tag; // ]
              stream>>tag; //val
              stream>>tag; //=
              stream>>val;
              value=val;
              stream>>tag; //>
              stream>>tag; // either <ARRACELLUPDATE or </ARRAYUPDATE
              ostringstream def;
              def<<coordinates<<" = "<<value;
              String tester =def.str();
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                    "array cell action setting: " + tester);
              Int before = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "before assignment: "
                                    + String::toString(before));
              
              array(coordinates) =value;
              Int after = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "after assignment: "
                                    + String::toString(after));
            }
            
              }
              arraycol.put(rownum, array);
            } else if(type==TpUInt){
              
              ArrayColumn<uInt> arraycol(subtable, name);
              Array<uInt> array =arraycol(rownum);
              int dim = array.ndim();
              
              uInt value;
              int val;
              stream>>tag; //<ARRAYCELLUPDATE
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                    "should be <ARRAYCELLUPDATE: " + tag);
              while(tag!="</ARRAYUPDATE>"){
            if(tag=="<ARRAYCELLUPDATE"){
              stream>>tag; //coordinates
              stream>>tag; //=
              stream>>tag; //[
              IPosition coordinates((uInt)dim);
              for(int abc=0;abc<dim;abc++){
                stream>>coord;
                
                coordinates(abc)=coord;
              }
              stream>>tag; // ]
              stream>>tag; //val
              stream>>tag; //=
              stream>>val;
              value=val;
              stream>>tag; //>
              stream>>tag; // either <ARRACELLUPDATE or </ARRAYUPDATE
              ostringstream def;
              def<<coordinates<<" = "<<value;
              String tester =def.str();
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                    "array cell action setting: " + tester);
              uInt before = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "before assignment: "
                                    + String::toString(before));
              
              array(coordinates) =value;
              uInt after = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "after assignment: "
                                  + String::toString(after));
            }
            
              }
              arraycol.put(rownum, array);
              
            } else if(type==TpFloat){
              ArrayColumn<Float> arraycol(subtable, name);
              Array<Float> array =arraycol(rownum);
              int dim = array.ndim();
              
              Float value;
              float val;
              stream>>tag; //<ARRAYCELLUPDATE
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "should be <ARRAYCELLUPDATE: " + tag);
              while(tag!="</ARRAYUPDATE>"){
            if(tag=="<ARRAYCELLUPDATE"){
              stream>>tag; //coordinates
              stream>>tag; //=
              stream>>tag; //[
              IPosition coordinates((uInt)dim);
              for(int abc=0;abc<dim;abc++){
                stream>>coord;
                
                coordinates(abc)=coord;
              }
              stream>>tag; // ]
              stream>>tag; //val
              stream>>tag; //=
              stream>>val;
              value=val;
              stream>>tag; //>
              stream>>tag; // either <ARRACELLUPDATE or </ARRAYUPDATE
              ostringstream def;
              def<<coordinates<<" = "<<value;
              String tester =def.str();
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "array cell action setting: " + tester);
              Float before = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW,"before assignment: "
                                    + String::toString(before));
              
              array(coordinates) =value;
              Float after = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "after assignment: "
                                    + String::toString(after));
            }
              }
              arraycol.put(rownum, array);
              
            } else if(type==TpDouble){
              ArrayColumn<Double> arraycol(subtable, name);
              Array<Double> array =arraycol(rownum);
              int dim = array.ndim();
              
              Double value;
              double val;
              stream>>tag; //<ARRAYCELLUPDATE
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                    "should be <ARRAYCELLUPDATE: " + tag);
              while(tag!="</ARRAYUPDATE>"){
            if(tag=="<ARRAYCELLUPDATE"){
              stream>>tag; //coordinates
              stream>>tag; //=
              stream>>tag; //[
              IPosition coordinates((uInt)dim);
              for(int abc=0;abc<dim;abc++){
                stream>>coord;
                
                coordinates(abc)=coord;
              }
              stream>>tag; // ]
              stream>>tag; //val
              stream>>tag; //=
              stream>>val;
              value=val;
              stream>>tag; //>
              stream>>tag; // either <ARRACELLUPDATE or </ARRAYUPDATE
              ostringstream def;
              def<<coordinates<<" = "<<value;
              String tester =def.str();
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                    "array cell action setting: " + tester);
              Double before = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW,"before assignment: "
                                    + String::toString(before));
              
              array(coordinates) =value;
              Double after = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "after assignment: "
                                    + String::toString(after));
            }
            
              }
              arraycol.put(rownum, array);
              
            }  else if(type==TpComplex){
              ArrayColumn<Complex> arraycol(subtable, name);
              Array<Complex> array =arraycol(rownum);
              int dim = array.ndim();

              String strval;

              stream>>tag; //<ARRAYCELLUPDATE
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                    "should be <ARRAYCELLUPDATE: " + tag);
              while(tag!="</ARRAYUPDATE>"){
            if(tag=="<ARRAYCELLUPDATE"){
              stream>>tag; //coordinates
              stream>>tag; //=
              stream>>tag; //[
              IPosition coordinates((uInt)dim);
              for(int abc=0;abc<dim;abc++){
                stream>>coord;
                
                coordinates(abc)=coord;
              }
              stream>>tag; // ]
              stream>>tag; //val
              stream>>tag; //=
              stream>>strval;
              
              int comma = strval.find(",");
              String real = strval.at(1,comma-1);
              String imag = strval.at(comma+1, strval.length()-1-comma);
              stringstream converter;
              float dreal;
              stringstream converter2;
              float dimag;
              converter<<real;
              converter>>dreal;
              converter2<<imag;
              converter2>>dimag;
              Complex value(dreal,dimag);
              //TBConstants::dprint(TBConstants::DEBUG_LOW,
              // "the complex value being stored is: " + value.toString());
              TBConstants::dprint(TBConstants::DEBUG_LOW, "complex real: " +
                                  String::toString(dreal) + ", complex imag: "
                                  + String::toString(dimag));
              
              stream>>tag; //>
              stream>>tag; // either <ARRACELLUPDATE or </ARRAYUPDATE
              array(coordinates) =value;
            }
              }
              arraycol.put(rownum, array);
            }  else if(type==TpDComplex){

              ArrayColumn<DComplex> arraycol(subtable, name);
              Array<DComplex> array =arraycol(rownum);
              int dim = array.ndim();
              
                  String strval;

              stream>>tag; //<ARRAYCELLUPDATE
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "should be <ARRAYCELLUPDATE: " + tag);
              while(tag!="</ARRAYUPDATE>"){
        
            if(tag=="<ARRAYCELLUPDATE"){
              stream>>tag; //coordinates
              stream>>tag; //=
              stream>>tag; //[
              IPosition coordinates((uInt)dim);
              for(int abc=0;abc<dim;abc++){
                stream>>coord;
                
                coordinates(abc)=coord;
              }
              stream>>tag; // ]
              stream>>tag; //val
              stream>>tag; //=
              stream>>strval;
              
              int comma = strval.find(",");
              String real = strval.at(1,comma-1);
              String imag = strval.at(comma+1, strval.length()-1-comma);
              stringstream converter;
              stringstream converter2;
              double dreal;
              double dimag;
              converter<<real;
              converter>>dreal;
             
              converter2<<imag;
              converter2>>dimag;
              DComplex value(dreal,dimag);
              
              stream>>tag; //>
              stream>>tag; // either <ARRACELLUPDATE or </ARRAYUPDATE
              array(coordinates) =value;
            }
            
              }
              arraycol.put(rownum, array);
            }
          //   else if(type==TpComplex){
//            ArrayColumn<Complex> arraycol(subtable, name);
//                Array<Complex> array =arraycol(rownum);
//                int dim = array.ndim();
//            String strval;
//            stream>>tag; //<ARRAYCELLUPDATE
//            TBConstants::dprint(TBConstants::DEBUG_LOW,
//                      "should be <ARRAYCELLUPDATE: " + tag);
//            while(tag!="</ARRAYUPDATE>"){
//              if(tag=="<ARRAYCELLUPDATE"){
//                stream>>tag; //coordinates
//                stream>>tag; //=
//                stream>>tag; //[
//                IPosition coordinates((uInt)dim);
//                for(int abc=0;abc<dim;abc++){
//                  stream>>coord;
//                  coordinates(abc)=coord;
//                }
//                stream>>tag; // ]
//                stream>>tag; //val
//                stream>>tag; //=
//                stream>>strval;
//            int comma = strval.find(",");
//            String real = strval.at(1,comma-1);
//            String imag = strval.at(comma+1, strval.length()-1-comma);
//            strstream converter;
//            float dreal;
//            float dimag;
//            converter<<real;
//            converter>>dreal;
//            converter<<imag;
//            converter>>dimag;
//            Complex value(dreal,dimag);
//                stream>>tag; //>
//            stream>>tag; // either <ARRACELLUPDATE or </ARRAYUPDATE
//                array(coordinates) =value;
//          }
//            }
//                arraycol.put(rownum, array);
//          }
            else if(type==TpString){
              ArrayColumn<String> arraycol(subtable, name);
              Array<String> array =arraycol(rownum);
              int dim = array.ndim();
              
              String value;
              string val;
              stream>>tag; //<ARRAYCELLUPDATE
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "should be <ARRAYCELLUPDATE: " + tag);
              while(tag!="</ARRAYUPDATE>"){
        
            if(tag=="<ARRAYCELLUPDATE"){
              stream>>tag; //coordinates
              stream>>tag; //=
              stream>>tag; //[
              IPosition coordinates((uInt)dim);
              for(int abc=0;abc<dim;abc++){
                stream>>coord;
                
                coordinates(abc)=coord;
              }
              stream>>tag; // ]
              stream>>tag; //val
              stream>>tag; //=
              stream>>val;
              value=val;
              stream>>tag; //>
              stream>>tag; // either <ARRACELLUPDATE or </ARRAYUPDATE
              ostringstream def;
              def<<coordinates<<" = "<<value;
              String tester =def.str();
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                    "array cell action setting: " + tester);
              String before = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "before assignment"
                                    + before);
              
              array(coordinates) =value;
              String after = array(coordinates);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "after assignment: "
                                    + after);
            }
            
              }
              arraycol.put(rownum, array);
              
            } else{
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                  "unidentified array type: " + type);
            }
            
          } else if(tag=="<DELROW"){
            TBConstants::dprint(TBConstants::DEBUG_LOW, "can remove row: " +
                                String::toString(subtable.canRemoveRow()));
            int number;
            stream >>number;
            stream>>tag;
           
            TBConstants::dprint(TBConstants::DEBUG_LOW, "deleting row: " +
                                TBConstants::itoa(number));
            subtable.removeRow((uInt)number);
            
          } else if(tag=="<ADDROW>"){
            TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "attempting to add row");
            
            subtable.addRow(1,true);

          } else{
            TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "unidentified command: " + tag);
          }
        }
        
          }
          
          delete [] colarr;

          String hits="Done";
          
          int length = htonl(hits.length());
         //length = length/65000+1;
         
         // TBConstants::dprint(TBConstants::DEBUG_LOW, "numtimes = " +
          //                    TBConstants::itoa(length));
         // TBConstants::dprint(TBConstants::DEBUG_LOW, "char at 154048: " +
          //                    hits[154048]);
         stringstream converter;
         converter<<length;
         string strlength;
         converter>>strlength;

         String slen = strlength+"\n";
         
         // TBConstants::dprint(TBConstants::DEBUG_LOW, "slen of string = " +
         //                             slen);
         
         rstat += hits;
//       if(SendData(fd,  hits) == -1){
//         TBConstants::dprint(TBConstants::DEBUG_LOW, "Error sending data1");
//       rstat = -1;
//       return rstat;
//       }

        } else if(initQuery == "send.table.query"){
          // TBConstants::dprint(TBConstants::DEBUG_LOW, "got into if");
          TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "query string: " + query);
          stringstream diff;
          diff<<query;
          string word;
          string word2;
          string name;
          diff>>word;
          diff>>word2;
          diff>>name;
          int totalNumRows;
          int start;
          int numRows;
          string tag;
          
          diff>>tag;
          while(tag!="<START")
        diff>>tag;
          
          if(tag=="<START"){
        TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "should be <START = " + tag);
        diff>>tag;
        diff>>start;
        diff>>tag;
        diff>>tag;
        diff>>numRows;
          } else{
        diff>>tag;
          }
          
          int hjk  = query.index("<START");
          TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "index of hjk: " + TBConstants::itoa(hjk));
          query=query.at(0, hjk);
         
          TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "the actual query: " + query);

          TBConstants::dprint(TBConstants::DEBUG_LOW, "start pos: " +
                                TBConstants::itoa(start) + ", rows per page: "
                                + TBConstants::itoa(numRows));
         
          Table result;

          String qur(query);
          if(word=="SELECT"&&word2=="FROM"&&(!qur.contains("WHERE"))){
        result=Table(name);
          } else{
        result = tableCommand(query);
          }
          
          totalNumRows=result.nrow();
          TBConstants::dprint(TBConstants::DEBUG_LOW, "Total rows are: " + 
                                TBConstants::itoa(totalNumRows));
          int startIndex;
          if (start<0){
        startIndex=0;
          }else{
        startIndex=start;
          }
          int endIndex;
          
          if(start+numRows<(int)result.nrow()){
        endIndex=start+numRows;
          }else if(numRows==0){
        endIndex = result.nrow();
          }
          else{
        endIndex = result.nrow();
          }
          TBConstants::dprint(TBConstants::DEBUG_LOW, "startIndex: " +
                                TBConstants::itoa(startIndex) + ", endIndex: "
                                + TBConstants::itoa(endIndex));
          String hits;
          if(endIndex<=startIndex){
        throw(AipsError("EMPTY"));
          }
          
          Bool cinsRowOk= result.canAddRow();
          Bool cdelRowOk = result.canRemoveRow();
          
          TableRecord trec = result.keywordSet();
          
          keywords= createKeyword(trec, -1);
          
          ROTableRow row(result);
          Vector<String> colNames = row.columnNames();
          void **fieldPtrs = (void **)new uInt*[colNames.nelements()];
          
          ostringstream oss;
          Vector<String> dataTypes(colNames.nelements());
          
          //TBConstants::dprint(TBConstants::DEBUG_LOW,
//                              String::toString(colNames));
          {
        // String columnkw = "<COLUMNKEYWORDS>\n";
        String columnkw="none";
        TableDesc tdesc= result.tableDesc();
        if(colNames.nelements()>0){
          columnkw="";
        }
        for(uInt i=0;i<colNames.nelements();i++){
          // TBConstants::dprint(TBConstants::DEBUG_LOW, "entering for 1");
          
              TBConstants::dprint(TBConstants::DEBUG_LOW, "a");
          ColumnDesc cdesc = tdesc.columnDesc(i);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "a");
          //columnkw+="<COLUMN num = "+ String::toString(i)+" >\n";
          TableRecord ctrec = cdesc.keywordSet();
              TBConstants::dprint(TBConstants::DEBUG_LOW, "a");
          String tempkw="";
              TBConstants::dprint(TBConstants::DEBUG_LOW, "a");
          
          tempkw = createKeyword(ctrec,i);
              TBConstants::dprint(TBConstants::DEBUG_LOW, "a");
         
          if(tempkw!="none"){
            columnkw+=tempkw;
          } else{
            TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "column keyword is null");
            columnkw+=" ";
          }
          
          TBConstants::dprint(TBConstants::DEBUG_LOW, "I am here");
          TBConstants::dprint(TBConstants::DEBUG_LOW,
                              TBConstants::itoa(colNames.nelements()) + " " +
                              TBConstants::itoa(i) + " " + colNames(i) + " " +
                              row.record().type(row.record().fieldNumber(
                                                colNames(i))));
          try{
          switch(row.record().type(row.record().fieldNumber(colNames(i)))) {
          case TpString :
            fieldPtrs[i] =
              new RORecordFieldPtr<String>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_STRING;
            break;
            
          case TpInt :
            TBConstants::dprint(TBConstants::DEBUG_LOW, " Here ");
            fieldPtrs[i] =
              new RORecordFieldPtr<Int>(row.record(), colNames(i));
            TBConstants::dprint(TBConstants::DEBUG_LOW, " Then here ");
            dataTypes[i]=TBConstants::TYPE_INT;
            break;
            
          case TpFloat :
            fieldPtrs[i] =  (void *)
              new RORecordFieldPtr<Float>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_FLOAT;
            break;
            
          case TpDouble :
            {
                      fieldPtrs[i] =
            new RORecordFieldPtr<Double>(row.record(), colNames(i));
              TableDesc td = result.tableDesc();
              ColumnDesc cd = td[i];
              String com = cd.comment();
              TBConstants::dprint(TBConstants::DEBUG_LOW,
                                    "double comment: " + com);
              if(com==TBConstants::COMMENT_DATE){
            TBConstants::dprint(TBConstants::DEBUG_LOW, "it's a date");
            dataTypes[i]=TBConstants::TYPE_DATE;
              }
              else{
            dataTypes[i]=TBConstants::TYPE_DOUBLE;
              }
                      break;
            }
          case TpBool :
            fieldPtrs[i] =  (void *)
              new RORecordFieldPtr<Bool>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_BOOL;
            break;
            
          case TpUChar :
            fieldPtrs[i] =
              new RORecordFieldPtr<uChar>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_UCHAR;
            break;
            
          case TpShort :
            fieldPtrs[i] =
              new RORecordFieldPtr<Short>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_SHORT;
            break;
            
          case TpUInt :
            fieldPtrs[i] =
              new RORecordFieldPtr<uInt>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_UINT;
            break;
            
          case TpComplex :
            fieldPtrs[i] =
              new RORecordFieldPtr<Complex>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_COMPLEX;
            break;
            
          case TpDComplex :
            fieldPtrs[i] =
              new RORecordFieldPtr<DComplex>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_DCOMPLEX;
                    break;
            
          case TpArrayDouble :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Double> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_DOUBLE;
            break;
            
          case TpArrayBool :
            fieldPtrs[i] =
            new RORecordFieldPtr<Array<Bool> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_BOOL;
            break;

          case TpArrayChar :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Char> >(row.record(), colNames(i));
            dataTypes[i]="unsupported";
            break;
            
          case TpArrayUChar :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<uChar> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_UCHAR;
            break;
            
          case TpArrayShort :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Short> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_SHORT;
            break;
          
          case TpArrayInt :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Int> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_INT;
            break;
            
          case TpArrayUInt :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<uInt> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_UINT;
            break;
          
          case TpArrayFloat :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Float> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_FLOAT;
            break;
            
          case TpArrayComplex :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Complex> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_COMPLEX;
            break;
            
          case TpArrayDComplex :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<DComplex> >(row.record(),
                                                     colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_DCOMPLEX;
            break;
            
          case TpArrayString :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<String> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_STRING;
            break;
            
          default:
            TBConstants::dprint(TBConstants::DEBUG_LOW, "unknown type: " +
                                row.record().type(row.record().fieldNumber(
                                        colNames(i))));
    // throw(AipsError("atabd: unexpected type, this should never happen"));
            break;
          }
          
          } catch(AipsError x) {
              TBConstants::dprint(TBConstants::DEBUG_LOW, "AipsError: ta" +
                                    x.getMesg());
          }
       
           TBConstants::dprint(TBConstants::DEBUG_LOW, "I am now here");
          }
        
        TBConstants::dprint(TBConstants::DEBUG_LOW, "getting entries from " +
                            TBConstants::itoa(start) + " to " +
                            TBConstants::itoa(start + numRows));
          
        for(int i=startIndex;i<endIndex;i++){
          row.get(i);
          oss<<"<"<<TBConstants::XML_TR<<">"<<endl;
          for(int j=0;j<Int(colNames.nelements());j++){
            
            switch(row.record().type(j)){
            case TpString :
              oss << "<" << TBConstants::XML_TD << "> ";
              oss <<**((RORecordFieldPtr<String> *)fieldPtrs[j])<<" </";
              oss << TBConstants::XML_TD <<">";
              break;
            case TpFloat :
              oss << "<" << TBConstants::XML_TD << "> ";
              oss << **((RORecordFieldPtr<Float> *)fieldPtrs[j])<<" </";
              oss << TBConstants::XML_TD << ">";
              break;
            case TpInt :
              oss << "<"<<TBConstants::XML_TD<<"> ";
              oss << **((RORecordFieldPtr<Int> *)fieldPtrs[j])<<" </";
              oss <<TBConstants::XML_TD<<">";
              break;
            case TpDouble :
              {
            TableDesc td = result.tableDesc();
            ColumnDesc cd = td[j];
            String com = cd.comment();
            
            if(com==TBConstants::COMMENT_DATE){
              
              double days = (**((RORecordFieldPtr<Double> *)fieldPtrs[j]));
              String tstring = TBConstants::date(days);
              oss <<  "<"<<TBConstants::XML_TD<<"> "<<tstring<<" </";
              oss <<TBConstants::XML_TD<<">";
                
            } else{
              oss << "<"<<TBConstants::XML_TD<<"> ";
              oss <<**((RORecordFieldPtr<Double> *)fieldPtrs[j])<<" </";
              oss <<TBConstants::XML_TD<<">";
            }
              }
                         break;
                      case TpBool :
                         oss << "<"<<TBConstants::XML_TD<<"> ";
                         oss << **((RORecordFieldPtr<Bool> *)fieldPtrs[j]);
                         oss <<" </"<<TBConstants::XML_TD<<">";
                         break;
                      case TpUChar :
                         oss << "<"<<TBConstants::XML_TD<<"> ";
                         oss << **((RORecordFieldPtr<uChar> *)fieldPtrs[j]);
                         oss <<" </"<<TBConstants::XML_TD<<">";
                         break;
                      case TpShort :
                         oss << "<"<<TBConstants::XML_TD<<"> ";
                         oss << **((RORecordFieldPtr<Short> *)fieldPtrs[j]);
                         oss <<" </"<<TBConstants::XML_TD<<">";
                         break;
                      case TpUInt :
                         oss << "<"<<TBConstants::XML_TD<<"> ";
                         oss << **((RORecordFieldPtr<uInt> *)fieldPtrs[j]);
                         oss <<" </"<<TBConstants::XML_TD<<">";
                         break;
                      case TpComplex :
                         oss << "<"<<TBConstants::XML_TD<<"> ";
                         oss << **((RORecordFieldPtr<Complex> *)fieldPtrs[j]);
                         oss <<" </"<<TBConstants::XML_TD<<">";
                         break;
              case TpDComplex :
                         oss << "<"<<TBConstants::XML_TD<<"> ";
                         oss << **((RORecordFieldPtr<DComplex> *)fieldPtrs[j]);
                         oss <<" </"<<TBConstants::XML_TD<<">";
                         break;
              case TpArrayDouble :
                  {
                    IPosition pos = (**((RORecordFieldPtr<Array<Double> > *)
                                    fieldPtrs[j])).shape();
                    if(pos.nelements()==1){
                  oss << "<"<<TBConstants::XML_TD<<"> ";
                  oss << **((RORecordFieldPtr<Array<Double> > *)fieldPtrs[j]);
                  oss <<" </"<<TBConstants::XML_TD<<">";
                    } else{
                  oss <<"<"<<TBConstants::XML_TD<<"> "<<pos<<"Double"<<" </";
                  oss <<TBConstants::XML_TD<<">";
                    }
                  }
                  break;
              case TpArrayBool :
            {
              IPosition pos = (**((RORecordFieldPtr<Array<Bool> > *)
                                fieldPtrs[j])).shape();
              if(pos.nelements()==1){
            
                oss << "<"<<TBConstants::XML_TD<<"> ";
                oss << **((RORecordFieldPtr<Array<Bool> > *)fieldPtrs[j]);
                oss <<" </"<<TBConstants::XML_TD<<">";
              } else{
                oss<<"<"<<TBConstants::XML_TD<<"> "<<pos<<"Boolean"<<" </";
                oss<<TBConstants::XML_TD<<">";
              }
              break;
            }
              case TpArrayChar :
                {
              IPosition pos = (**((RORecordFieldPtr<Array<Char> > *)
                                fieldPtrs[j])).shape();
              if(pos.nelements()==1){
            
                oss << "<"<<TBConstants::XML_TD<<"> ";
                oss << **((RORecordFieldPtr<Array<Char> > *)fieldPtrs[j]);
                oss <<" </"<<TBConstants::XML_TD<<">";
              } else{
                oss<<"<"<<TBConstants::XML_TD<<"> "<<pos<<"Char"<<" </";
                oss<<TBConstants::XML_TD<<">";
              }
            }
              break;

              case TpArrayUChar :
            {
              IPosition pos = (**((RORecordFieldPtr<Array<uChar> > *)
                                fieldPtrs[j])).shape();
              if(pos.nelements()==1){
            
                oss << "<"<<TBConstants::XML_TD<<"> ";
                oss << **((RORecordFieldPtr<Array<uChar> > *)fieldPtrs[j]);
                oss <<" </"<<TBConstants::XML_TD<<">";
              } else {
                oss<<"<"<<TBConstants::XML_TD<<"> "<<pos<<"UChar"<<" </";
                oss <<TBConstants::XML_TD<<">";
              }
            }
              break;

             case TpArrayShort :
               {
             IPosition pos = (**((RORecordFieldPtr<Array<Short> > *)
                                fieldPtrs[j])).shape();
             if(pos.nelements()==1){
            
               oss << "<"<<TBConstants::XML_TD<<"> ";
               oss << **((RORecordFieldPtr<Array<Short> > *)fieldPtrs[j]);
               oss <<" </"<<TBConstants::XML_TD<<">";
             } else {
               oss<<"<"<<TBConstants::XML_TD<<"> "<<pos<<"Short"<<" </";
               oss <<TBConstants::XML_TD<<">";
             }
               }
             break;
           
           case TpArrayInt :
             {
               IPosition pos = (**((RORecordFieldPtr<Array<Int> > *)
                                    fieldPtrs[j])).shape();
               if(pos.nelements()==1){
             oss << "<"<<TBConstants::XML_TD<<"> ";
             oss << **((RORecordFieldPtr<Array<Int> > *)fieldPtrs[j]);
             oss <<" </"<<TBConstants::XML_TD<<">";
               } else {
             oss<<"<"<<TBConstants::XML_TD<<"> "<<pos<<"Integer"<<" </";
             oss<<TBConstants::XML_TD<<">";
               }
             }
               break;
             
           case TpArrayUInt :
             {
               IPosition pos = (**((RORecordFieldPtr<Array<uInt> > *)
                                fieldPtrs[j])).shape();
               if(pos.nelements()==1){
             oss << "<"<<TBConstants::XML_TD<<"> ";
             oss << **((RORecordFieldPtr<Array<uInt> > *)fieldPtrs[j]);
             oss <<" </"<<TBConstants::XML_TD<<">";
               } else {
             oss<<"<"<<TBConstants::XML_TD<<"> "<<pos<<"UInteger"<<" </";
             oss<<TBConstants::XML_TD<<">";
               }
             }
               break;

           case TpArrayFloat :
             {
               IPosition pos = (**((RORecordFieldPtr<Array<Float> > *)
                                fieldPtrs[j])).shape();
               if(pos.nelements()==1){
             oss << "<"<<TBConstants::XML_TD<<"> ";
             oss<< **((RORecordFieldPtr<Array<Float> > *)fieldPtrs[j]);
             oss<<" </"<<TBConstants::XML_TD<<">";
               } else{
             oss<<"<"<<TBConstants::XML_TD<<"> "<<pos<<"Float"<<" </";
             oss<<TBConstants::XML_TD<<">";
               }
             }
               break;

           case TpArrayComplex :
             {
               IPosition pos = (**((RORecordFieldPtr<Array<Complex> > *)
                                fieldPtrs[j])).shape();
               if(pos.nelements()==1){
             oss << "<"<<TBConstants::XML_TD<<"> ";
             oss<< **((RORecordFieldPtr<Array<Complex> > *)fieldPtrs[j]);
             oss<<" </"<<TBConstants::XML_TD<<">";
               } else{
             oss<<"<"<<TBConstants::XML_TD<<"> "<<pos<<"Complex"<<" </";
             oss<< TBConstants::XML_TD <<">";
               }
             }
               break;
            
           case TpArrayDComplex :
             {
               IPosition pos = (**((RORecordFieldPtr<Array<DComplex> > *)
                                fieldPtrs[j])).shape();
               if(pos.nelements()==1){
             oss << "<"<<TBConstants::XML_TD<<"> ";
             oss<< **((RORecordFieldPtr<Array<DComplex> > *)fieldPtrs[j]);
             oss<<" </"<<TBConstants::XML_TD<<">";
               } else{
             oss<<"<"<<TBConstants::XML_TD<<"> "<<pos<<"DComplex"<<" </";
             oss<<TBConstants::XML_TD<<">";
               }
             }
               break;
            
           case TpArrayString :
             {
               IPosition pos = (**((RORecordFieldPtr<Array<String> > *)
                                    fieldPtrs[j])).shape();
               if(pos.nelements()==1){
             oss << "<"<<TBConstants::XML_TD<<"> ";
             oss<< **((RORecordFieldPtr<Array<String> > *)fieldPtrs[j]);
             oss<<" </"<<TBConstants::XML_TD<<">";
               } else{
             oss<<"<"<<TBConstants::XML_TD<<"> "<<pos<<"String"<<" </";
             oss<<TBConstants::XML_TD<<">";
               }
             }
               break;

           default:
         throw(AipsError("atabd: unexpected type, this should never happen:"));
             
                 break;
                   }
                   oss << " ";
          }
                oss <<endl<< "</"<<TBConstants::XML_TR<<">"<<endl;
        }
        
             hits = oss.str();
             
         hits=createVOTab(result.tableName(),totalNumRows ,colNames, dataTypes,
                          hits, keywords, cinsRowOk, cdelRowOk, columnkw);
         
         int length = htonl(hits.length());
         stringstream converter;
         converter<<length;
         string strlength;
         converter>>strlength;

         String slen = strlength+"\n";

         rstat += hits;
//       if(SendData(fd,  hits) == -1){
//         TBConstants::dprint(TBConstants::DEBUG_LOW, "Error sending data2");
//       rstat = -1;
//       return rstat;
//       }
             
             for(int j=0;j<Int(colNames.nelements());j++){
                   switch(row.record().type(j)){
                      case TpString :
                         delete (RORecordFieldPtr<String> *)fieldPtrs[j];
                         break;
                      case TpFloat :
                         delete (RORecordFieldPtr<Float> *)fieldPtrs[j];
                         break;
                      case TpInt :
                         delete (RORecordFieldPtr<Int> *)fieldPtrs[j];
                         break;
                      case TpDouble :
                         delete (RORecordFieldPtr<Double> *)fieldPtrs[j];
                         break;
                      case TpBool :
                         delete (RORecordFieldPtr<Bool> *)fieldPtrs[j];
                         break;
                      case TpUChar :
                         delete (RORecordFieldPtr<uChar> *)fieldPtrs[j];
                         break;
                      case TpShort :
                         delete (RORecordFieldPtr<Short> *)fieldPtrs[j];
                         break;
                      case TpUInt :
                         delete (RORecordFieldPtr<uInt> *)fieldPtrs[j];
                         break;
                      case TpComplex :
                         delete (RORecordFieldPtr<Complex> *)fieldPtrs[j];
                         break;
                      case TpDComplex :
                         delete (RORecordFieldPtr<DComplex> *)fieldPtrs[j];
                         break;
              case TpArrayDouble :
                 delete (RORecordFieldPtr<Array<Double> > *)fieldPtrs[j];
                 break;
              case TpArrayBool   :
                 delete (RORecordFieldPtr<Array<Bool> > *)fieldPtrs[j];
                 break;
              case TpArrayChar :
                delete (RORecordFieldPtr<Array<Char> > *)fieldPtrs[j];
                break;
              case TpArrayUChar :
                delete (RORecordFieldPtr<Array<uChar> > *)fieldPtrs[j];
                break;
              case TpArrayShort :
                delete (RORecordFieldPtr<Array<Short> > *)fieldPtrs[j];
                break;
              case TpArrayInt :
                delete (RORecordFieldPtr<Array<Int> > *)fieldPtrs[j];
                break;
              case TpArrayUInt :
                delete (RORecordFieldPtr<Array<uInt> > *)fieldPtrs[j];
                break;
              case TpArrayFloat :
                delete (RORecordFieldPtr<Array<Float> > *)fieldPtrs[j];
                break;
              case TpArrayComplex :
                delete (RORecordFieldPtr<Array<Complex> > *)fieldPtrs[j];
                break;
              case TpArrayDComplex :
                delete (RORecordFieldPtr<Array<DComplex> > *)fieldPtrs[j];
                break;
              case TpArrayString :
                delete (RORecordFieldPtr<Array<String> > *)fieldPtrs[j];
                break;
             
              default:
          throw(AipsError("atabd: unexpected type, this should never happen"));
            break;
                   }
             }

             //result.~Table();
             delete [] fieldPtrs;
          }



        } else if(initQuery == "send.table.qfull"){
          //  TBConstants::dprint(TBConstants::DEBUG_LOW, "got into if");
         TBConstants::dprint(TBConstants::DEBUG_LOW, "query string: " + query);
          stringstream diff;
          diff<<query;
          string word;
          string word2;
          string name;
          diff>>word;
          diff>>word2;
          diff>>name;
          int totalNumRows;
          int start;
          int numRows;
          string tag;
          
          diff>>tag;
          while(tag!="<START")
        diff>>tag;
          
          if(tag=="<START"){
        TBConstants::dprint(TBConstants::DEBUG_LOW,
                            "should be <START = " + tag);
        diff>>tag;
        diff>>start;
        diff>>tag;
        diff>>tag;
        diff>>numRows;
          } else{
        diff>>tag;
          }
          
          int hjk  = query.index("<START");
          TBConstants::dprint(TBConstants::DEBUG_LOW,
                              "index of hjk: " + TBConstants::itoa(hjk));
          query=query.at(0, hjk);
         
          TBConstants::dprint(TBConstants::DEBUG_LOW,
                                "the actual query: " + query);
          
          TBConstants::dprint(TBConstants::DEBUG_LOW, "start pos: " +
                              TBConstants::itoa(start) + ", rows per page: " +
                              TBConstants::itoa(numRows));
         
          Table result;

          String qur(query);
          
          if(word=="SELECT"&&word2=="FROM"&&(!qur.contains("WHERE"))){
        result=Table(name);
          } else{
        result = tableCommand(query);
          }
          
          totalNumRows=result.nrow();
          TBConstants::dprint(TBConstants::DEBUG_LOW, "got some rows: " +
                                TBConstants::itoa(totalNumRows));
          int startIndex;
          if (start<0){
        startIndex=0;
          }else{
        startIndex=start;
          }
          int endIndex;
          
          if(start+numRows<(int)result.nrow()){
        endIndex=start+numRows;
          
          }else if(numRows==0){
        endIndex = result.nrow();
          } else{
        endIndex = result.nrow();
          }
          TBConstants::dprint(TBConstants::DEBUG_LOW, "startIndex: " +
                              TBConstants::itoa(startIndex) + ", endIndex: " +
                              TBConstants::itoa(endIndex));
          String hits;
          if(endIndex<=startIndex){
        throw(AipsError("EMPTY"));
          }
          
          Bool cinsRowOk= result.canAddRow();
          Bool cdelRowOk = result.canRemoveRow();
          
          TableRecord trec = result.keywordSet();
          // TBConstants::dprint(TBConstants::DEBUG_LOW,
          //                     "# of fields in keywordset: " +
          //                     TBConstants::itoa(trec.nfields()));
          
          keywords= createKeyword(trec, -1);
          
          // TBConstants::dprint(TBConstants::DEBUG_LOW, "\n\nKeywords\n" +
          //                                 keywords.toString());
          
          ROTableRow row(result);
          Vector<String> colNames = row.columnNames();
          void **fieldPtrs = (void **)new uInt*[colNames.nelements()];
          
          ostringstream oss;
          Vector<String> dataTypes(colNames.nelements());
          
          {
        // String columnkw = "<COLUMNKEYWORDS>\n";
        String columnkw="none";
        TableDesc tdesc= result.tableDesc();
        if(colNames.nelements()>0){
          columnkw="";
        }
        for(int i=0;i<Int(colNames.nelements());i++){
          // TBConstants::dprint(TBConstants::DEBUG_LOW, "entering for 1");
          
          ColumnDesc cdesc = tdesc.columnDesc((uInt)i);
          //columnkw+="<COLUMN num = "+ String::toString(i)+" >\n";
          TableRecord ctrec = cdesc.keywordSet();
          String tempkw="";
          
          tempkw = createKeyword(ctrec,i);
         
          if(tempkw!="none"){
            columnkw+=tempkw;
          }
          else{
            TBConstants::dprint(TBConstants::DEBUG_LOW,
                                    "column keyword is null");
            columnkw+=" ";
          }
          
          switch(row.record().type(row.record().fieldNumber(colNames(i)))){
          case TpString :
            fieldPtrs[i] =
              new RORecordFieldPtr<String>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_STRING;
            break;
            
          case TpInt :
            fieldPtrs[i] =
              new RORecordFieldPtr<Int>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_INT;
            
            break;
            
          case TpFloat :
            fieldPtrs[i] =  (void *)
              new RORecordFieldPtr<Float>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_FLOAT;
            break;
            
          case TpDouble :
            {
                      fieldPtrs[i] =
            new RORecordFieldPtr<Double>(row.record(), colNames(i));
              TableDesc td = result.tableDesc();
              ColumnDesc cd = td[i];
              String com = cd.comment();
              TBConstants::dprint(TBConstants::DEBUG_LOW, "double comment: " +
                                    com);
              if(com==TBConstants::COMMENT_DATE){
            TBConstants::dprint(TBConstants::DEBUG_LOW, "it's a date");
            dataTypes[i]=TBConstants::TYPE_DATE;
              }
              else{
            dataTypes[i]=TBConstants::TYPE_DOUBLE;
              }
            }
                      break;
          case TpBool :
            fieldPtrs[i] =  (void *)
              new RORecordFieldPtr<Bool>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_BOOL;
            
            break;
            
          case TpUChar :
            fieldPtrs[i] =
              new RORecordFieldPtr<uChar>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_UCHAR;
            break;
            
          case TpShort :
            fieldPtrs[i] =
              new RORecordFieldPtr<Short>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_SHORT;
            break;
            
          case TpUInt :
            fieldPtrs[i] =
              new RORecordFieldPtr<uInt>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_UINT;
            break;
            
          case TpComplex :
            fieldPtrs[i] =
              new RORecordFieldPtr<Complex>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_COMPLEX;
            break;
            
          case TpDComplex :
            fieldPtrs[i] =
              new RORecordFieldPtr<DComplex>(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_DCOMPLEX;
                    break;
            
          case TpArrayDouble :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Double> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_DOUBLE;
            break;
            
          case TpArrayBool :
            fieldPtrs[i] =
            new RORecordFieldPtr<Array<Bool> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_BOOL;
            break;

          case TpArrayChar :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Char> >(row.record(), colNames(i));
            dataTypes[i]="unsupported";
            break;
            
          case TpArrayUChar :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<uChar> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_UCHAR;
            break;
            
          case TpArrayShort :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Short> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_SHORT;
            break;
          
          case TpArrayInt :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Int> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_INT;
            break;
            
          case TpArrayUInt :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<uInt> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_UINT;
            break;
          
          case TpArrayFloat :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Float> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_FLOAT;
            break;
            
          case TpArrayComplex :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<Complex> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_COMPLEX;
            break;
            
          case TpArrayDComplex :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<DComplex> >(row.record(),colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_DCOMPLEX;
            break;
            
          case TpArrayString :
            fieldPtrs[i] =
              new RORecordFieldPtr<Array<String> >(row.record(), colNames(i));
            dataTypes[i]=TBConstants::TYPE_ARRAY_STRING;
            break;
            
          default:
            TBConstants::dprint(TBConstants::DEBUG_LOW, "unknown type: " +
                               row.record().type(row.record().fieldNumber(
                                       colNames(i))));
          throw(AipsError("atabd: unexpected type, this should never happen"));
            break;
          }
          
       
        }

        TBConstants::dprint(TBConstants::DEBUG_LOW, "getting entries from " +
                            TBConstants::itoa(start) + " to " +
                            TBConstants::itoa(start + numRows));
          
        for(int i=startIndex;i<endIndex;i++){
          row.get(i);
          oss<<"<"<<TBConstants::XML_TR<<">"<<endl;
          for(int j=0;j<Int(colNames.nelements());j++){
            
            switch(row.record().type(j)){
            case TpString :
              oss << "<"<<TBConstants::XML_TD<<"> ";
              oss<<**((RORecordFieldPtr<String> *)fieldPtrs[j])<<" </";
              oss<<TBConstants::XML_TD<<">";
              break;
            case TpFloat :
              oss << "<"<<TBConstants::XML_TD<<"> ";
              oss<< **((RORecordFieldPtr<Float> *)fieldPtrs[j])<<" </";
              oss<<TBConstants::XML_TD<<">";
              break;
            case TpInt :
              oss << "<"<<TBConstants::XML_TD<<"> ";
              oss<< **((RORecordFieldPtr<Int> *)fieldPtrs[j])<<" </";
              oss<<TBConstants::XML_TD<<">";
              break;
            case TpDouble :
              {
            TableDesc td = result.tableDesc();
            ColumnDesc cd = td[j];
            String com = cd.comment();
            
            if(com==TBConstants::COMMENT_DATE){
              double d = (**((RORecordFieldPtr<Double> *)fieldPtrs[j]));
              String tstring = TBConstants::date(d);
                oss <<  "<"<<TBConstants::XML_TD<<"> "<<tstring<<" </";
                oss<<TBConstants::XML_TD<<">";
                
              } else{
                oss <<  "<"<<TBConstants::XML_TD<<"> ";
                oss<<**((RORecordFieldPtr<Double> *)fieldPtrs[j])<<" </";
                oss<<TBConstants::XML_TD<<">";
              }
            }
                         break;
                      case TpBool :
                         oss << "<"<<TBConstants::XML_TD<<"> ";
                         oss<< **((RORecordFieldPtr<Bool> *)fieldPtrs[j]);
                         oss<<" </"<<TBConstants::XML_TD<<">";
                         break;
                      case TpUChar :
                         oss << "<"<<TBConstants::XML_TD<<"> ";
                         oss<< **((RORecordFieldPtr<uChar> *)fieldPtrs[j]);
                         oss<<" </"<<TBConstants::XML_TD<<">";
                         break;
                      case TpShort :
                         oss << "<"<<TBConstants::XML_TD<<"> ";
                         oss<< **((RORecordFieldPtr<Short> *)fieldPtrs[j]);
                         oss<<" </"<<TBConstants::XML_TD<<">";
                         break;
                      case TpUInt :
                         oss << "<"<<TBConstants::XML_TD<<"> ";
                         oss<< **((RORecordFieldPtr<uInt> *)fieldPtrs[j]);
                         oss<<" </"<<TBConstants::XML_TD<<">";
                         break;
                      case TpComplex :
                         oss << "<"<<TBConstants::XML_TD<<"> ";
                         oss<< **((RORecordFieldPtr<Complex> *)fieldPtrs[j]);
                         oss<<" </"<<TBConstants::XML_TD<<">";
                         break;
              case TpDComplex :
                 oss << "<"<<TBConstants::XML_TD<<"> ";
                 oss<< **((RORecordFieldPtr<DComplex> *)fieldPtrs[j]);
                 oss<<" </"<<TBConstants::XML_TD<<">";
                         break;
              case TpArrayDouble :
                   {
                   oss << "<"<<TBConstants::XML_TD<<"> ";
                   oss<< **((RORecordFieldPtr<Array<Double> > *)fieldPtrs[j]);
                   oss<<" </"<<TBConstants::XML_TD<<">";
                   }
                   break;
              case TpArrayBool :
            {
              oss << "<"<<TBConstants::XML_TD<<"> ";
              oss<< **((RORecordFieldPtr<Array<Bool> > *)fieldPtrs[j]);
              oss<<" </"<<TBConstants::XML_TD<<">";
            }
              break;
              case TpArrayChar :
                {
              oss << "<"<<TBConstants::XML_TD<<"> ";
              oss<< **((RORecordFieldPtr<Array<Char> > *)fieldPtrs[j]);
              oss<<" </"<<TBConstants::XML_TD<<">";
            }
              break;
              case TpArrayUChar :
            {
              oss << "<"<<TBConstants::XML_TD<<"> ";
              oss<< **((RORecordFieldPtr<Array<uChar> > *)fieldPtrs[j]);
              oss<<" </"<<TBConstants::XML_TD<<">";
            }
              break;
             case TpArrayShort :
               {
             oss << "<"<<TBConstants::XML_TD<<"> ";
             oss<< **((RORecordFieldPtr<Array<Short> > *)fieldPtrs[j]);
             oss<<" </"<<TBConstants::XML_TD<<">";
               }
             break;
             case TpArrayInt :
               {
                  oss << "<"<<TBConstants::XML_TD<<"> ";
                  oss<< **((RORecordFieldPtr<Array<Int> > *)fieldPtrs[j]);
                  oss<<" </"<<TBConstants::XML_TD<<">";
               }
               break;
             case TpArrayUInt :
               {
                  oss << "<"<<TBConstants::XML_TD<<"> ";
                  oss<< **((RORecordFieldPtr<Array<uInt> > *)fieldPtrs[j]);
                  oss<<" </"<<TBConstants::XML_TD<<">";
               }
               break;
             case TpArrayFloat :
               {
                  oss << "<"<<TBConstants::XML_TD<<"> ";
                  oss<< **((RORecordFieldPtr<Array<Float> > *)fieldPtrs[j]);
                  oss<<" </"<<TBConstants::XML_TD<<">";
               }
               break;
             case TpArrayComplex :
               {
                 oss << "<"<<TBConstants::XML_TD<<"> ";
                 oss<< **((RORecordFieldPtr<Array<Complex> > *)fieldPtrs[j]);
                 oss<<" </"<<TBConstants::XML_TD<<">";
               }
                 break;
             case TpArrayDComplex :
               {
                 oss << "<"<<TBConstants::XML_TD<<"> ";
                 oss<< **((RORecordFieldPtr<Array<DComplex> > *)fieldPtrs[j]);
                 oss<<" </"<<TBConstants::XML_TD<<">";
               }
                 break;
             case TpArrayString :
               {
                 oss << "<"<<TBConstants::XML_TD<<"> ";
                 oss<< **((RORecordFieldPtr<Array<DComplex> > *)fieldPtrs[j]);
                 oss<<" </"<<TBConstants::XML_TD<<">";
               }
                 break;
             default:
         throw(AipsError("atabd: unexpected type, this should never happen:"));
                 break;
                   }
                   oss << " ";
          }
                oss <<endl<< "</"<<TBConstants::XML_TR<<">"<<endl;
        }


             hits = oss.str();
         hits=createVOTab(result.tableName(),totalNumRows ,colNames, dataTypes,
                          hits, keywords, cinsRowOk, cdelRowOk, columnkw);

         TBConstants::dprint(TBConstants::DEBUG_LOW, "Hits: " + hits);

         int length = htonl(hits.length());
         stringstream converter;
         converter<<length;
         string strlength;
         converter>>strlength;

         String slen = strlength+"\n";
         rstat += hits;
//       if(SendData(fd,  hits) == -1){
//         TBConstants::dprint(TBConstants::DEBUG_LOW, "Error sending data3");
//       rstat = -1;
//       return rstat;
//       }
             {
             for(int j=0;j<Int(colNames.nelements());j++){
                   switch(row.record().type(j)){
                      case TpString :
                         delete (RORecordFieldPtr<String> *)fieldPtrs[j];
                         break;
                      case TpFloat :
                         delete (RORecordFieldPtr<Float> *)fieldPtrs[j];
                         break;
                      case TpInt :
                         delete (RORecordFieldPtr<Int> *)fieldPtrs[j];
                         break;
                      case TpDouble :
                         delete (RORecordFieldPtr<Double> *)fieldPtrs[j];
                         break;
                      case TpBool :
                         delete (RORecordFieldPtr<Bool> *)fieldPtrs[j];
                         break;
                      case TpUChar :
                         delete (RORecordFieldPtr<uChar> *)fieldPtrs[j];
                         break;
                      case TpShort :
                         delete (RORecordFieldPtr<Short> *)fieldPtrs[j];
                         break;
                      case TpUInt :
                         delete (RORecordFieldPtr<uInt> *)fieldPtrs[j];
                         break;
                      case TpComplex :
                         delete (RORecordFieldPtr<Complex> *)fieldPtrs[j];
                         break;
                      case TpDComplex :
                         delete (RORecordFieldPtr<DComplex> *)fieldPtrs[j];
                         break;
           case TpArrayDouble :
             delete (RORecordFieldPtr<Array<Double> > *)fieldPtrs[j];
             break;
          

           case TpArrayBool   :
             delete (RORecordFieldPtr<Array<Bool> > *)fieldPtrs[j];
             break;
             
           case TpArrayChar :
             delete (RORecordFieldPtr<Array<Char> > *)fieldPtrs[j];
             break;

           case TpArrayUChar :
             delete (RORecordFieldPtr<Array<uChar> > *)fieldPtrs[j];
             break;
             
           case TpArrayShort :
             delete (RORecordFieldPtr<Array<Short> > *)fieldPtrs[j];
             break;
             
           case TpArrayInt :
             delete (RORecordFieldPtr<Array<Int> > *)fieldPtrs[j];
             break;
             
           case TpArrayUInt :
             delete (RORecordFieldPtr<Array<uInt> > *)fieldPtrs[j];
             break;
             
           case TpArrayFloat :
             delete (RORecordFieldPtr<Array<Float> > *)fieldPtrs[j];
             break;
             
           case TpArrayComplex :
             delete (RORecordFieldPtr<Array<Complex> > *)fieldPtrs[j];
             break;
             
           case TpArrayDComplex :
             delete (RORecordFieldPtr<Array<DComplex> > *)fieldPtrs[j];
             break;

           case TpArrayString :
             delete (RORecordFieldPtr<Array<String> > *)fieldPtrs[j];
             break;
             
           default:
          throw(AipsError("atabd: unexpected type, this should never happen"));
                 break;
                   }
             }
             }
             //result.~Table();
             delete [] fieldPtrs;
          }
        }
      } catch (AipsError x) {
             ostringstream oss;
             oss << "AipsError: ta" << x.getMesg() << endl;
             String hits = oss.str();
         
         rstat += hits;
//       if(SendData(fd,  hits) == -1){
//         TBConstants::dprint(TBConstants::DEBUG_LOW, "Error sending data4");
//       rstat = -1;
//       return rstat;
//       }
             TBConstants::dprint(TBConstants::DEBUG_LOW,
                     "AipsError thrown: ta" + x.getMesg());
          }
   }

   return rstat;
}

String TBXMLDriver::createVOTab(String tablename, int totalrows,
        Vector<String> colnames, Vector<String> datatype, String records,
      String keyword, Bool insRowOk, Bool delRowOk, String columnkeywords) {
  String insRow;
  String delRow;
  if(insRowOk)
    insRow=TBConstants::XML_TRUE;
  else
    insRow=TBConstants::XML_FALSE;
  
  if(delRowOk)
    delRow=TBConstants::XML_TRUE;
  else
    delRow=TBConstants::XML_FALSE;
  
  String head= "<?xml version=\"1.0\"?>\n";
  head+="<!DOCTYPE VOTABLE SYSTEM \"http://us-vo.org/xml/VOTable.dtd\">\n";
  head+="<" + TBConstants::XML_VOTABLE + " version=\"1.0\">\n<";
  head+=TBConstants::XML_RESOURCE + ">\n<" + TBConstants::XML_TABLE + " ";
  head+=TBConstants::XML_NAME + "=\"" + tablename +"\">\n";
  head+="<" + TBConstants::XML_TOTAL + " " + TBConstants::XML_ROW + " = \"";
  head+=String::toString(totalrows)+"\" />\n";

  for (int i=0; i<(int)colnames.nelements();i++){
    head+="<" + TBConstants::XML_FIELD + " " + TBConstants::XML_NAME + "=\"";
    head+=colnames[i] + "\" " + TBConstants::XML_FIELD_UCD + "=\"\" ";
    head+=TBConstants::XML_FIELD_REF + "=\"\" " + TBConstants::XML_FIELD_UNIT;
    head+="=\"\" " + TBConstants::XML_FIELD_TYPE + "=\"" + datatype[i] + "\" ";
    head+=TBConstants::XML_FIELD_PRECISION + "=\"\" ";
    head+=TBConstants::XML_FIELD_WIDTH + "=\"\"/>\n";
  }
  if(keyword!="none"){
    head+=keyword;
  } else {
    TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword is null");
  }
  
  if(columnkeywords!="none"){
    head+=columnkeywords;
  } else{
    TBConstants::dprint(TBConstants::DEBUG_LOW, "no column keywords");
  }
  head+="<" + TBConstants::XML_RWINFO + " " + TBConstants::XML_INSERTROW;
  head+=" = \""+ insRow+ "\" " + TBConstants::XML_REMOVEROW + " = \""+delRow;
  head+="\" />\n<" + TBConstants::XML_DATA + ">\n<";
  head+= TBConstants::XML_TABLEDATA + ">\n" + records + "</";
  head+=TBConstants::XML_TABLEDATA + ">\n</";
  head+= TBConstants::XML_DATA + ">\n</" + TBConstants::XML_TABLE + ">\n</";
  head+=TBConstants::XML_RESOURCE + ">\n</";
  head+= TBConstants::XML_VOTABLE + ">";
  
  return head;
}

String TBXMLDriver::createKeyword(TableRecord &trec, int a) {
  TBConstants::dprint(TBConstants::DEBUG_LOW, "method create keyword called");
  stringstream kwordstream;
  String keywords="none";
  bool ent=false;

  for(Int g=0;g<(Int)trec.nfields();g++){
    if(a<0){
      kwordstream<<"<"<<TBConstants::XML_KEYWORD;
    } else{
      kwordstream<<"<"<<TBConstants::XML_COLUMNKW<<" ";
      kwordstream<< TBConstants::XML_COLKW_COL<<" = \"";
      kwordstream<<String::toString(a)<<"\" ";
    }
    RecordFieldId rfid(g);
    if(trec.type(g)==TpFloat){
      float c =trec.asFloat(rfid);
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: float");
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_FLOAT<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<< trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<<c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpTable){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: table");
      Table t = trec.asTable(rfid);
      String c = t.tableName();
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_TABLE<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpInt){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: int");
      int c = trec.asInt(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_INT<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpBool){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: bool");
      bool c = trec.asBool(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_BOOL<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpChar){
      TBConstants::dprint(TBConstants::DEBUG_LOW,
                          "keyword type: char not supported");
      unsigned char c = trec.asuChar(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_UCHAR<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpUChar){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: uchar");
      unsigned char c = trec.asuChar(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_UCHAR<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpShort){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: short");
      short c = trec.asShort(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_SHORT<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpUInt){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: uint");
      uInt c = trec.asuInt(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_UINT<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpDouble){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: double");
      double c = trec.asDouble(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_DOUBLE<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpComplex){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: complex");
      Complex c = trec.asComplex(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_COMPLEX<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpDComplex){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: dcomplex");
      DComplex c = trec.asDComplex(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_DCOMPLEX<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<<c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpString){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: string");
      String c = trec.asString(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_STRING<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpArrayBool){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: ArrayBool");
      Array<Bool> c = trec.asArrayBool(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_ARRAY_BOOL<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpArrayUChar){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: ArrayUChar");
      Array<uChar> c = trec.asArrayuChar(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_ARRAY_UCHAR<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpArrayShort){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: ArrayShort");
      Array<Short> c = trec.asArrayShort(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_ARRAY_SHORT<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpArrayInt){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: ArrayInt");
      Array<Int> c = trec.asArrayInt(rfid);
      // TBConstants::dprint(TBConstants::DEBUG_LOW, c.toString());
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_ARRAY_INT<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpArrayUInt){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: ArrayUInt");
      Array<uInt> c = trec.asArrayuInt(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_ARRAY_UINT<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpArrayFloat){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: ArrayFloat");
      Array<Float> c = trec.asArrayFloat(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_ARRAY_FLOAT<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpArrayDouble){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: ArrayDouble");
      Array<Double> c = trec.asArrayDouble(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_ARRAY_DOUBLE<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<< c<<"\"";
      ent=true;
    }  else if(trec.type(g)==TpArrayComplex){
      TBConstants::dprint(TBConstants::DEBUG_LOW,"keyword type: ArrayComplex");
      Array<Complex> c = trec.asArrayComplex(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_ARRAY_COMPLEX<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<<c<<"\"";
      ent=true;
    }  else if(trec.type(g)==TpArrayDComplex){
     TBConstants::dprint(TBConstants::DEBUG_LOW,"keyword type: ArrayDComplex");
      Array<DComplex> c = trec.asArrayDComplex(rfid);
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_ARRAY_DCOMPLEX<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<<c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpArrayString){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: ArrayString");
      Array<String> c = trec.asArrayString(rfid);
      //TBConstants::dprint(TBConstants::DEBUG_LOW, c.toString());
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_ARRAY_STRING<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \""<<c<<"\"";
      ent=true;
    } else if(trec.type(g)==TpRecord){
      TBConstants::dprint(TBConstants::DEBUG_LOW, "keyword type: record");
      kwordstream<<" "<<TBConstants::XML_KEYWORD_TYPE<<" = \"";
      kwordstream<<TBConstants::TYPE_RECORD<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_NAME<<" = \"";
      kwordstream<<trec.name(rfid)<<"\"";
      kwordstream<<" "<<TBConstants::XML_KEYWORD_VAL<<" = \" { ";
      for(Int ijk = 0 ; ijk< (Int)trec.nfields();ijk++){
    RecordFieldId fid(ijk);
    DataType tp = trec.dataType(fid);
    TBConstants::dprint(TBConstants::DEBUG_LOW, "the record's type is: " + tp);
    if(tp==TpBool){
      kwordstream<<"boolean ";
    } else if(tp==TpArrayString){
      kwordstream<<"stringArray ";
    } else if(tp==TpRecord){
      kwordstream<<"record ";
    }
      }
      kwordstream<<"}\"";
      ent=true;
    } else{
      TBConstants::dprint(TBConstants::DEBUG_LOW,
    "===============WARNING=====================\nKeyword type not supported: "
              + trec.type(g));
    }
    kwordstream<<" />"<<endl;
  }
  TBConstants::dprint(TBConstants::DEBUG_LOW, "kwordstream: " +
                        kwordstream.str());
  if(ent){
    keywords= kwordstream.str();
  }
  TBConstants::dprint(TBConstants::DEBUG_LOW, "returning keywords: " +
                        keywords);
  
  return keywords;
}

int TBXMLDriver::SendData(int fd, const String &hits) {
   int DataSize = hits.length();
   int r_status(0);
   int NumRecords(DataSize/PacketSize);
   if(DataSize%PacketSize)
      NumRecords++;
   char *theData = mkReturnResult(hits);
   for(int i=0;i<NumRecords;i++){
      int SendBytes = (i == NumRecords-1) ? DataSize%PacketSize : PacketSize;
      r_status = writen(fd, theData+i*PacketSize, SendBytes);
      TBConstants::dprint(TBConstants::DEBUG_LOW, "sent: " +
                            TBConstants::itoa(SendBytes));
   }
   delete [] theData;
   return(r_status);
}

int TBXMLDriver::readn(int fd, char *ptr, int nbytes) {
   int nleft(nbytes);
   while(nleft > 0){
   int nread = read(fd, ptr, nleft);
      if(nread < 0)
         return(nread);
      else if(nread == 0)
         break;
      nleft -= nread;
      ptr += nread;
   }
   return(nbytes - nleft);
}

int TBXMLDriver::writen(int fd, char *ptr, int nbytes) {
   int nleft(nbytes);
   while(nleft > 0){
      int nwrite = write(fd, ptr, nleft);
      if(nwrite < 0)
         return(nwrite);
      nleft -= nwrite;
      ptr += nwrite;
   }
   return(nbytes - nleft);
}
*/
