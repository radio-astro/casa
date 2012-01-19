// -*- C++ -*-
//# AppRC.cc: Implementation of the AppRC class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
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
#include <synthesis/Utilities/AppRC.h>
#include <casa/OS/RegularFile.h>
namespace casa{
  //
  //--------------------------------------------------------------
  //
  AppRC::~AppRC() 
  {
    // if (deleteFile_p)
    //   {
    // 	RegularFile ff(fileName_p);
    // 	cerr << id_p << " " << fileName_p << endl;
    // 	if (ff.exists()) 
    // 	  {
    // 	    if (id_p != "") 
    // 	      cerr << id_p << " removing " << fileName_p << endl;
    // 	    ff.remove();
    // 	  }
    // 	else
    // 	  {
    // 	    if (id_p != "") 
    // 	      cerr << id_p << " did not find " << fileName_p << endl;
    // 	  }
    //   }

  };
  //
  //--------------------------------------------------------------
  //
  void AppRC::init(const string& filename, const Bool addPID,
		   const Bool deleteFile) 
  {
    addPID_p=addPID; deleteFile_p=deleteFile;
    if (rc_p == NULL)
      {
	ostringstream tt; 
	tt << filename;
	if (addPID_p) 
	  {
	    setPID();
	    tt << "_" << myPID_p;
	  }
	fileName_p=tt.str();
	Casarc::setDefaultPath(fileName_p);
	rc_p = &Casarc::instance();
      }
  };
  //
  //--------------------------------------------------------------
  //
  string AppRC::get(const string& name) 
  {return rc_p->get(name);}
  //
  //--------------------------------------------------------------
  //
  string AppRC::get(const string& name, Int& val) 
  {
    string strVal=rc_p->get(name); 
    if (strVal != "")
      val=atoi(strVal.c_str());
    return strVal;
  }
  //
  //--------------------------------------------------------------
  //
  string AppRC::get(const string& name, Float& val) 
  {
    string strVal=rc_p->get(name); 
    if (strVal != "")
      val=(Float)atof(strVal.c_str());
    return strVal;
  }
  //
  //--------------------------------------------------------------
  //
  string AppRC::get(const string& name, Double& val) 
  {
    string strVal=rc_p->get(name); 
    if (strVal != "")
      val=(Double)atof(strVal.c_str());
    return strVal;
  }
  //
  //--------------------------------------------------------------
  //
  void AppRC::put(const string& name, const string& val) 
  {rc_p->put(name,val);}
  //
  //--------------------------------------------------------------
  //
  void AppRC::put(const string& name, const Int& val) 
  {
    ostringstream tt; tt << val;
    put(name,tt.str());
  }
  //
  //--------------------------------------------------------------
  //
  void AppRC::put(const string& name, const Float& val) 
  {
    ostringstream tt; tt << val;
    put(name,tt.str());
  }
  //
  //--------------------------------------------------------------
  //
  void AppRC::put(const string& name, const Double& val) 
  {
    ostringstream tt; tt << val;
    put(name,tt.str());
  }

};
