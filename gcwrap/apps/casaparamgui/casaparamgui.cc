//# casaparamgui.cc: Standalone GUI for inputting task parameters and running.
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
#include <casaqt/QtParamGui/QtParamGUI.qo.h>
#include <xmlcasa/xerces/stdcasaXMLUtil.h>
#include <xmlcasa/StdCasa/CasacSupport.h>

#include <QApplication>
#include <QDir>
#include <fstream>

#include <casa/namespace.h>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    
    String file;
    bool noarg = argc < 2;
    if(!noarg) file = argv[1];
    
    // Common task XML location
    String path = getenv("CASAPATH");
    unsigned int i = path.find(' ');
    path = path.substr(0, i) += "/share/xml/";
    
    if(!noarg) {
        // check that file exists
        fstream fin;
        fin.open(file.c_str(), ios::in);
        bool valid = fin.is_open();
        fin.close();
        
        if(!valid) {
            String xmlDir = path + file + ".xml";

            // check if file exists
            fin.open(xmlDir.c_str(), ios::in);
            valid = fin.is_open();
            fin.close();

            if(valid) file = xmlDir;
        }
        if(!valid) noarg = true;
    }
    
    casac::stdcasaXMLUtil xmlUtils;
    casac::record r;
    if(noarg) {
        // run for all tasks
        QDir dir(path.c_str());
        QStringList files = dir.entryList(QStringList("*.xml"),
                                          QDir::Files);
        for(int i = 0; i < files.size(); i++) {
            String s = path + qPrintable(files[i]);
            xmlUtils.readXMLFile(r, s);
        }
    } else {
        // run for a specific task
        xmlUtils.readXMLFile(r, file);
    }
    
    Record* rec = toRecord(r);
    QtParamGUI gui(*rec, QtParamGUI::PYTHON, NULL); // python, standalone mode
    delete rec;
    
    return app.exec();
}
