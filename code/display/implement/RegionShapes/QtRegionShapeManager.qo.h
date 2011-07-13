//# QtRegionShapeManager.qo.h: Classes for managing/loading region shapes.
//# Copyright (C) 2008
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
#ifndef QTREGIONSHAPEMANAGER_QO_H_
#define QTREGIONSHAPEMANAGER_QO_H_

#include <QDialog>
#include <QStackedLayout>
#include <QtXml>

#include <display/RegionShapes/QtRegionShapeManager.ui.h>
#include <display/RegionShapes/QtRSFileLoader.ui.h>
#include <display/RegionShapes/QtRSFileSaver.ui.h>

#include <display/RegionShapes/RegionShape.h>

#include <casa/namespace.h>

namespace casa {

class QtDisplayPanel;
class RSFileReader;
class RSFileWriter;
class QtSingleRegionShape;

// Region Shape Manager.  Main interaction between region shapes and user.
// Provides functionality to:
// <ul><li>Load region shapes from a file</li>
//     <li>Display loaded region shapes</li>
//     <li>Show/hide loaded region shapes</li>
//     <li>Delete loaded region shapes</li>
//     <li>Edit region shapes</li></ul>
class QtRegionShapeManager : public QWidget, Ui::RegionShapeManager {
    Q_OBJECT

public:
    // Constant message used when a error was reported during a shape drawing.    
    static const String DRAWERRORMSG;
    
    
    // Constructor which takes the parent panel.
    QtRegionShapeManager(QtDisplayPanel* panel);

    // Destructor.
    ~QtRegionShapeManager();

    // Returns the parent panel.
    QtDisplayPanel* panel() const;
    
    // Appends an XML state representation of loaded shapes to the given
    // document.
    void saveState(QDomDocument& document);
    
    // Restores loaded shapes from the given document.
    void restoreState(QDomDocument& document);
    
    // Returns the number of shapes in the manager.  If includeComposites is
    // false, composites are not counted towards the return total.  Composite
    // children are not included.
    unsigned int numShapes(bool includeComposites = true) const;
    
    // Returns the shapes in the manager.  If include composites is false,
    // composites are not included.  Composite children are not included.
    vector<RegionShape*> shapes(bool includeComposites = true) const;
    
    // Returns the QtSingleRegionShape wrapper for the given shape, or NULL
    // for invalid.
    QtSingleRegionShape* shapeWidget(RegionShape* shape) const;
    
    // Shows the given error message (should be short).
    void showSimpleError(const String& message, bool warn = true) const;
    
    // Shows the given detailed error message.  "message" should contain a
    // short overview while "details" should contain longer information.
    void showDetailedError(const String& message, const String& details,
                           bool warn = true) const;
    
public slots:
    // Adds the given shape to the manager.  Should only be used for individual
    // shapes, since the display panel will refresh after every call which can
    // get very slow with many shapes.  For multiple shapes, use addShapes().
    // The given shape becomes owned by the manager which is responsible for
    // its deletion.
    void addShape(RegionShape* shape);
    
    // Adds the given shapes to the manager.  Holds the drawing until all
    // shapes have been added.  The given shapes become owned by the manager
    // which is responsible for their deletion.
    void addShapes(const vector<RegionShape*>& shapes);
    
    // Removes and (optionally) deletes the given shape from the manager.
    void removeShape(RegionShape* shape, bool deleteShape = true);    
    
    // Delete all loaded region shapes.
    void deleteAll();

    // Enables the manager.
    void enable() { setEnabled(true); }

private:
    QtDisplayPanel* m_panel;               // Parent panel.
    vector<QtSingleRegionShape*> m_shapes; // Loaded region shapes.
    String m_lastDirectory,                // Last loaded directory, file,
           m_lastFile,                     // and format.  Initially empty.
           m_lastFormat;
    
    // Adds the given shape with the given composite parent (or NULL if the
    // shape is not a composite child) to the manager.
    void addShape(RegionShape* shape, RegionShape* compositeParent);
    
    // Adds the given shapes with the given composite parents (or NULL if the
    // shapes do not not parents) to the manager.
    void addShapes(const vector<RegionShape*>& shapes,
                   const vector<RegionShape*>& compositeParents);
    
    
    // XML attributes.
    // <group>
    static const QString HIDDEN;
    static const QString LAST_DIRECTORY;
    static const QString LAST_FILE;
    static const QString LAST_FORMAT;
    static const QString WINDOW_VISIBLE;
    // </group>
    
private slots:
    // Show or hide all loaded region shapes.
    void showHideAll(bool checked);

    // Load region shapes from a file.
    void load();
    
    // Saves loaded region shapes from a file.
    void save();

    // Create a new shape.
    void newShape();

    // Dismiss/close window.
    void dismiss();
};


// Class for loading region shape files.  The main use is
// QtRSFileLoader::getFileReader(), which prompts the user for a filename
// (with file chooser dialog) and a file format and then returns a RSFileReader
// appropriate for reading that file.
class QtRSFileLoader : public QDialog, Ui::RSFileLoader {
    Q_OBJECT

public:
    // Constructor.  The initial file and format will be set to the given, and
    // if a filechooser is opened the starting directory will be set to the
    // given.
    QtRSFileLoader(String file = "", String format = "", String dir = "");

    // Destructor.
    ~QtRSFileLoader();


    // Opens a new QtRSFileLoader so that the user can input a filename and
    // file format, and then returns a RSFileReader appropriate for reading
    // that file.  Returns NULL if the user cancels or if an error occurs
    // (shouldn't happen).  If the String* arguments are given, they will be
    // used for the initial settings and then updated to show the opened
    // file, format, and directory, respectively.
    static RSFileReader* getFileReader(String* file = NULL,
                                       String* format = NULL,
                                       String* directory = NULL);

private:
    QString m_lastDir;   // directory to start filechooser
    
    // Gets the current filename chosen by the user.
    String getFilename();

    // Gets the directory of the filename chosen by the user.
    String getDirectory();
    
    // Gets the current region file format chosen by the user.
    // Guaranteed to be one of the values in
    // RegionFileReader::supportedTypesStrings().
    String getFormat();
    
private slots:
    // Show a file chooser dialog.
    void browse();
    
    // Check that the entered file is valid, then accept().
    void ok();
};


// Class for saving region files.  The main use is
// QtRSFileSaver::getFileWriter(), which prompts the user for a filename
// (with file chooser dialog), a file format, and options specific to the file
// format, and then returns a RSFileWriter appropriate for writing that file.
class QtRSFileSaver : public QDialog, Ui::RSFileSaver {
    Q_OBJECT
    
public:
    // Constructor.  The initial file and format will be set to the given, and
    // if a filechooser is opened the starting directory will be set to the
    // given.
    QtRSFileSaver(String file = "", String format = "", String dir = "");
    
    // Destructor.
    ~QtRSFileSaver();

    
    // Opens a new QtRegionFileSaver so that the user can input a filename,
    // file format, and format options, and then returns a RegionFileWriter
    // appropriate for writing that file.  Returns NULL if the user cancels or
    // an error occurs (shouldn't happen).  If the String* arguments are given,
    // they will be used for the initial settings and then updated to show the
    // saved file, format, and directory, respectively.
    static RSFileWriter* getFileWriter(String* file = NULL,
                                       String* format = NULL,
                                       String* directory = NULL);
    
private:
    QString m_lastDir;  // directory to start filechooser
    
    // Layout holding widgets for specialized format options.
    QStackedLayout* m_formatOptions;
    
    // Gets the current filename chosen by the user.
    String getFilename();
    
    // Gets the directory of the filename chosen by the user.
    String getDirectory();

    // Gets the current region file format chosen by the user.
    // Guaranteed to be one of the values in
    // RegionFileReader::supportedTypesStrings().
    String getFormat();
    
    // Gets the current options widget.
    QWidget* getOptions();
    
private slots:
    // Show a file chooser dialog.
    void browse();
    
    // Show/hide the region-specific options frame.
    void showHideOptions(bool show);
    
    // Check that the entered file is valid, then accept().
    void ok();
};

}

#endif /* QTREGIONSHAPEMANAGER_QO_H_ */
