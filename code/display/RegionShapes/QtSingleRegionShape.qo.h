//# QtSingleRegionShape.qo.h: Classes for viewing/editing a single RegionShape.
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
#ifndef QTSINGLEREGIONSHAPE_QO_H
#define QTSINGLEREGIONSHAPE_QO_H

#include <QWidget>

#include <display/RegionShapes/QtSingleRegionShape.ui.h>
#include <display/RegionShapes/QtEditRegionShape.ui.h>

#include <display/RegionShapes/RegionShape.h>
#include <display/RegionShapes/RSUtils.qo.h>

#include <casa/namespace.h>

namespace casa {

class QtRegionShapeManager;
class QtDisplayPanel;

// Widget for displaying a single region in the list of a QtRegionShapeManager.
// QtSingleRegionShape is the main interaction between the GUI classes and the
// underlying RegionShape.  Provides widgets for:
// <ul><li>showing/hiding the region shape</li>
//     <li>deleting the region shape</li>
//     <li>editing the region shape</li></ul>
// Is also responsible for registering/unregistering the region shape on the
// parent QtDisplayPanel as necessary.
class QtSingleRegionShape : public QWidget, Ui::SingleRegionShape {
    Q_OBJECT
    
    friend class QtEditRegionShape;
    
public:
    // Constructor which takes the RegionShape, the manager parent, and the
    // optional composite parent.  If a composite parent is given, some
    // behaviors are different.  The constructor will register the RegionShape
    // IF compositeParent is NULL.
    QtSingleRegionShape(RegionShape* shape, QtRegionShapeManager* parent,
                        bool reportDrawErrors = false,
                        RegionShape* compositeParent = NULL);

    // Destructor.
    ~QtSingleRegionShape();

    // Returns the RegionShape.
    RegionShape* shape();
    
    // Returns this shape's composite parent, or NULL if it has none.
    RegionShape* compositeParent();
    
    // Returns the parent manager.
    QtRegionShapeManager* manager();
    
    // Returns the display panel.
    QtDisplayPanel* panel();
    
    // Sets whether the QtSingleRegionShape should delete its RegionShape
    // during destruction or not.
    void setShouldDelete(bool shouldDelete = true);
    
    // Refreshes the GUI display.
    void refresh();
    
    // Returns whether this shape is currently showing (registered) on the
    // panel or not.
    bool isShown() const;
    
    // Show/hide the region shape by registering/unregistering.
    void showHide(bool show, bool reportErrors = true);

public slots:    
    // Show a QtEditRegionShape for editing.
    void edit();

private:
    RegionShape* m_shape;           // Shape (deletes on deconstruction).
    RegionShape* m_compositeParent; // Composite parent, or NULL.
    QtRegionShapeManager* m_parent; // Manager.
    QtDisplayPanel* m_panel;        // Panel.
    bool m_shouldDelete;            // Should delete shape on destruction.

    // Sets up the GUI widgets to display information from the underlying
    // region shape.
    void setupGUI();
    
    // Registers the shape on the underlying display panel, if applicable.
    void registerShape();

    // Unregisters the shape on the underlying display panel, if applicable.
    void unregisterShape();

private slots:
    // Slot for checkbox.
    void showHide_(bool show) { showHide(show, true); }

    // Deletes this shape by unregistering the region shape and then calling
    // the manager's delete method.    
    void deleteShape();
};


// Widget that can be used to input a String, String choice, double, bool,
// String array, or String choice array.
class QtRSOption : public QHBoxLayout {
    Q_OBJECT
    
public:
    // Constructor.
    QtRSOption(RegionShape::OptionType type, const String& name,
               const RSOption& value, const vector<String>& choices);
    
    // Destructor.
    ~QtRSOption();
    
    // Returns entered value.
    RSOption value() const;
    
private:
    // Type.
    RegionShape::OptionType m_type;
    
    // Value widgets.
    // <group>
    QLineEdit* m_string;
    QComboBox* m_choice;
    QDoubleSpinBox* m_double;
    QCheckBox* m_bool;
    vector<QLineEdit*> m_stringArray;
    vector<QComboBox*> m_choiceArray;
    vector<String> m_choices;
    QToolButton* m_lessButton, *m_moreButton;
    // </group>
    
private slots:
    // Add another array widget.
    void moreArray();
    
    // Remove an array widget.
    void lessArray();
};


// Dialog for editing a single region shape.  Can either be used in editing
// or creation mode.  Currently there are three sections:
// <ol><li>Coordinate editing.  Using the RegionShape's coordinate parameter
//         methods, the shape's current coordinates are displayed and can be
//         changed (both value and system/unit).</li>
//     <li>General options.  Line width/color, label text/font, linethrough,
//         etc.</li>
//     <li>Specific options.  Customization specific to each shape
//         type.</li></ol>
class QtEditRegionShape : public QWidget, Ui::EditRegionShape {
    Q_OBJECT
    
public:
    // Static Members //
    
    // System/Unit constants.
    // <group>
    static const String SEXAGESIMAL;
    static const String UNKNOWN;
    // </group>
    
    // Returns available systems for the system chooser.
    static vector<String> systems() {
        static vector<String> v(6);
        v[0] = MDirection::showType(MDirection::B1950);
        v[1] = MDirection::showType(MDirection::J2000);
        v[2] = MDirection::showType(MDirection::GALACTIC);
        v[3] = MDirection::showType(MDirection::SUPERGAL);
        v[4] = MDirection::showType(MDirection::ECLIPTIC);
        v[5] = RSUtils::PIXEL;
        return v;
    }
    
    static QComboBox* systemsChooser() {
        vector<String> v = systems();
        QComboBox* b = new QComboBox();
        for(unsigned int i = 0; i < v.size(); i++) b->addItem(v[i].c_str());
        return b;
    }
    
    
    // Non-Static Members //
    
    // Editing mode constructor.
    QtEditRegionShape(QtSingleRegionShape* shape);
    
    // Creation mode constructor.  showPosition, showSize, and applyButtonText
    // are only used if coordWidget is not NULL.
    QtEditRegionShape(RegionShape* shape, QtRegionShapeManager* manager,
                      QWidget* coordWidget = NULL, bool showPosition = true,
                      bool showSize = true, String applyButtonText = "");
    
    // Destructor.
    ~QtEditRegionShape();
    
    
    // Methods used for accessing what coordinate system/unit the user has
    // chosen.
    // <group>
    String chosenCoordinateSystem() const;    
    String chosenPositionUnit() const;    
    String chosenSizeUnit() const;
    // </group>
    
    // Returns true if the entered coordinates are valid, false otherwise.
    // Has no effect if a custom coordinate widget is provided.
    // Entered coordinates are valid if:
    // 1) Non-empty
    // 2) For non-sexagesimal, contains a number
    // 3) For sexagesimal, contains three numbers separated by :'s.
    // If invalid, a reason is given.
    bool enteredCoordinatesAreValid(String& reason) const;
    
public slots:
    // Apply changes to the underlying region shape.
    void apply();
    
signals:
    // Only emitted when widget is in creation mode, the apply button is shown,
    // and the apply button is clicked.
    void applyClicked();
    
private:
    // Indicates whether the widget is being used in creation or editing mode.
    bool m_creation;
    
    // Shape being edited.
    QtSingleRegionShape* m_shape;
    
    // Shape being created.
    RegionShape* m_cShape;
    
    // Shape manager.
    QtRegionShapeManager* m_manager;
    
    // Coordinate editing fields.
    vector<QLineEdit*> m_coordEdits;
    
    // Coord types.
    vector<RegionShape::CoordinateParameterType> m_coordTypes;
    
    // Last chosen system, position unit, and size unit.
    String m_lastSystem, m_lastPosUnit, m_lastSizeUnit;
    
    // Line color widget.
    QtColorWidget* m_lineColor;
    
    // Linethrough color widget.
    QtColorWidget* m_ltColor;
    
    // Label color widget.
    QtColorWidget* m_labelColor;
    
    // Option widgets.
    vector<QtRSOption*> m_optWidgets;
    
    // Option types.
    vector<RegionShape::OptionType> m_optTypes;
    
    
    // Initial GUI setup, depending on creation/edit mode.
    void init(QWidget* coordWidget = NULL, bool showPosition = true,
              bool showSize = true, String applyButtonText = "");
    
    // Sets up the GUI to reflect values/properties of the underlying region
    // shape.
    void setupGui();
    
    // Takes the value entered in the coordinate at the given index of
    // m_coordEdits and converts it into a number that can be sent to the
    // shape's coordinate parameter methods.
    double convertToRS(int index, bool& ok) const;
    
    // Displays the given coordinates with the proper unit.  Assumes that the
    // world system matches the GUI and the unit matches RegionShape::UNIT.
    void displayCoordinates(const vector<double>& coords);
    
    
    // Private Static Methods //
    
    // Returns available units for the position unit chooser.
    static vector<String> positionUnits() {
        static vector<String> v(5);
        v[0] = RSValue::DEG; v[1] = RSValue::RAD;
        v[2] = SEXAGESIMAL;  v[3] = RSValue::HMS;
        v[4] = RSValue::DMS;
        return v;
    }
    
    // Returns available units for the size unit chooser.
    static vector<String> sizeUnits() {
        static vector<String> v(4);
        v[0] = RSValue::DEG;    v[1] = RSValue::RAD;
        v[2] = RSValue::ARCSEC; v[3] = RSValue::ARCMIN;
        return v;
    }
    
private slots:
    // Update the displayed values to reflect the coordinate system, position
    // unit, and size unit chosen by the user.
    void coordSystemChanged();
    
    // Resets the values, system, and units to that of the underlying shape.
    void coordReset();
    
    // Enable/disable the linethrough options.
    void linethrough();
    
    // Call apply() if in edit mode, otherwise emit applyClicked() signal.
    void applySlot() {
        if(m_creation) emit applyClicked();
        else           apply();
    }
};

}

#endif /* QTSINGLEREGIONSHAPE_QO_H */
