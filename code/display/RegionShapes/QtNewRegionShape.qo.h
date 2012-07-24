//# QtNewRegionShape.qo.h: Widgets for creating a new region shape(s).
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
#ifndef QTNEWREGIONSHAPE_QO_H
#define QTNEWREGIONSHAPE_QO_H

#include <QDialog>
#include <QStackedLayout>
#include <QListWidget>
#include <QRadioButton>
#include <QLabel>

#include <display/RegionShapes/QtNewRegionShape.ui.h>
#include <display/RegionShapes/RegionShapes.h>

#include <casa/namespace.h>

namespace casa {

class QtRegionShapeManager;
class QtEditRegionShape;

// Dialog for creating a new region shape.  For now basically a wrapper with
// a shape chooser around a QtEditRegionShape widget.  For more complex shapes
// (polygon, composite) this will have to be changed.
class QtNewRegionShape : public QWidget, Ui::NewRegionShape {
    Q_OBJECT
    
public:
    // Static Members //
    
    // Returns creation widgets.
    static vector<pair<String, RegionShape*> >
    creationShapes(bool includeComposite = true) {
        vector<pair<String, RegionShape*> > v(includeComposite ? 9 : 8);        
        for(unsigned int i = 0; i < v.size(); i++)
            v[i] = pair<String,RegionShape*>(creationName(i),creationShape(i));        
        return v;
    }
    
    
    // Non-Static Members //
    
    // Constructor that takes parent.
    QtNewRegionShape(QtRegionShapeManager* manager,
                     bool includeComposite = true, bool deleteOnClose = true);
    
    // Destructor.
    ~QtNewRegionShape();
    
    // Shows/hides the close button.
    void showCloseButton(bool show = true);
    
signals:
    // Emitted whenever the user creates a shape.  After this signal is
    // emitted the newly created shape is replaced in the widget with a
    // blank one.
    void shapeCreated(RegionShape* createdShape);
    
private:
    QtRegionShapeManager* m_manager; // Parent
    vector<RegionShape*> m_shapes;   // Shapes
    QStackedLayout* m_widgets;       // Edit widgets
    
    static RegionShape* creationShape(int i) {
        if(i < 0)       return NULL;
        else if(i == 0) return new RSEllipse(0,0,0,0);
        else if(i == 1) return new RSCircle(0,0,0);
        else if(i == 2) return new RSRectangle(0,0,0,0);
        else if(i == 3)return new RSPolygon(Vector<double>(),Vector<double>());
        else if(i == 4) return new RSLine(0,0,0,0,7);
        else if(i == 5) return new RSVector(0,0,0,0,7);
        else if(i == 6) return new RSMarker(0,0,Display::X,10);
        else if(i == 7) return new RSText(0,0,"");
        else if(i == 8) return new RSComposite();
        else            return NULL;
    }
    static RegionShape* creationShape(unsigned int i) {
        return creationShape((int)i);
    }
    
    static String creationName(int i) {
        if(i < 0)       return "";
        else if(i == 0) return "ellipse";
        else if(i == 1) return "circle";
        else if(i == 2) return "rectangle";
        else if(i == 3) return "polygon";
        else if(i == 4) return "line";
        else if(i == 5) return "vector";
        else if(i == 6) return "marker";
        else if(i == 7) return "text";
        else if(i == 8) return "composite";
        else            return "";
    }
    static String creationName(unsigned int i) {
        return creationName((int)i);
    }
    
private slots:
    // When the "create" button is clicked.
    void create();
};


// Specialized widget for creating a new polygon.
class QtNewRSPolygon : public QWidget {
    Q_OBJECT
    
public:
    // Constructor which takes the polygon to modify.
    QtNewRSPolygon(RSPolygon* poly, QtRegionShapeManager* manager);
    
    // Destructor.
    ~QtNewRSPolygon();
    
    // See QtEditRegionShape::enteredCoordinatesAreValid.
    bool enteredCoordinatesAreValid(String& reason) const;
    
public slots:
    // Applies the entered values to the RSPolygon.
    void apply();
    
private:
    RSPolygon* m_polygon; // Polygon.
    QtEditRegionShape* m_editor; // Region shape editor.
    QListWidget* m_coordList; // List widget for displaying entered coordinates
    QFrame* m_coordFrame;     // Frame that holds list widget and buttons.
    vector<pair<QString, QString> > m_enteredCoords; // Entered coordinates
    QLineEdit* m_coordXEdit, *m_coordYEdit; // Coordinate edits.
    
private slots:
    // Add the values entered in the line edits to the coord list.
    void addCoordinates();
    
    // Moves the selected coordinate up in the list.
    void listMoveUp();
    
    // Moves the selected coordinate down in the list.
    void listMoveDown();
    
    // Deletes the selected coordinate in the list.
    void listDelete();
};


// Specialized widget for creating a new composite.
class QtNewRSComposite : public QWidget {
    Q_OBJECT
    
public:
    // Constructor which takes the composite to modify.
    QtNewRSComposite(RSComposite* comp, QtRegionShapeManager* manager);  
    
    // Destructor.
    ~QtNewRSComposite();
    
    // Returns whether the entered values are valid or not.  Returns true if
    // at least one child is entered, and all children have valid coordinates.
    bool enteredValuesAreValid(String& reason) const;
    
public slots:
    // Applies the entered values to the RSComposite.
    void apply();
    
private:
    // Top-lebel members.
    // <group>
    QtRegionShapeManager* m_manager;
    RSComposite* m_composite;
    vector<RegionShape*> m_children;
    QStackedLayout* m_layout;
    // </group>
    
    // First screen
    // <group>
    QRadioButton* m_dependentChildren;
    QRadioButton* m_newShapes;
    QLabel* m_dependentLabel1, *m_dependentLabel2;
    // </group>
    
    // Second screen: new shapes
    // <group>
    QListWidget* m_newShapeList;
    QComboBox* m_editChooser;
    QStackedLayout* m_editWidgets;
    // </group>
    
    // Second screen: existing shapes
    // <group>
    QListWidget* m_existingList, *m_moveList;
    vector<RegionShape*> m_existingShapes, m_moveShapes;
    // </group>
    
    
    // GUI initialization methods.
    // <group>
    QWidget* initFirstScreen();
    QWidget* initSecondScreen1();
    QWidget* initSecondScreen2();
    // </group>
    
    // Updated m_existingList and m_moveList based on m_existingShapes and
    // m_movesshapes.
    void updateLists();
    
    // Returns the adjusted index (m_existingList => m_existingShapes).
    unsigned int adjustedIndex(int row);
    
private slots:
    // Switches to second page.
    void next();
    
    // New shapes
    
    // Adds a new shape to the composite.
    void nAddShape(RegionShape* shape);
    
    // Moves the selected shape up in the list.
    void nListUp();
    
    // Moves the selected shape down in the list.
    void nListDown();
    
    // Deletes the selected shape in the list.
    void nListDelete();
    
    // Existing shapes
    
    // Adds the selected existing shape to the composite.
    void eAddShape();
    
    // Remove the selected shape from the composite.
    void eRemoveShape();
    
    // Moves the selected shape up in the list.
    void eListUp();
    
    // Moves the selected shape down in the list.
    void eListDown();
};

}

#endif /* QTNEWREGIONSHAPE_QO_H */
