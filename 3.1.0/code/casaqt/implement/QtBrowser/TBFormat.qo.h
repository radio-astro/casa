//# TBFormat.qo.h: Rules used to format displayed values for fields.
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
#ifndef TBFORMAT_H_
#define TBFORMAT_H_

#include <casaqt/QtBrowser/TBFormat.ui.h>
#include <casaqt/QtBrowser/TBConstants.h>

#include <QtGui>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class TBData;

// <summary>
// QFont color is a convenience class containing a QFont and a QColor.
// <summary>

class QFontColor {
public:
    // Constructor that takes a QFont and a QColor.
    QFontColor(QFont f, QColor c);

    // Copy Constructor.
    QFontColor(const QFontColor& fc);

    ~QFontColor();

    
    // Font.
    QFont font;
    
    // Color.
    QColor color;
};

// <summary>
// Rules used to format displayed values for fields.
// <summary>
//
// <synopsis>
// A TBFormat contains all information necessary to format a displayed value.
// A format consists of different options depending on the type of the value
// being formatted.  For example, an integer value could be in scientific
// notation and could have a different font style for negative and positive
// numbers.  Generally speaking, a format has two parts: type-specific options
// such as scientific notation or date format, and font options.  Some types
// may have multiple font options (i.e., one for negative values and one for
// non-negative values).
// </synopsis>

class TBFormat {
public:
    // Default Constructor.
    TBFormat();

    ~TBFormat();

    
    // Returns the QFontColor that is applied to all values (i.e., regardless
    // of negative or positive etc.).  If this format has value-dependent
    // QFontColors, this will return NULL.
    QFontColor* getAllFont();

    // Sets the QFontColor to be used on all values.
    void setAllFont(QFontColor* fc);

    // Returns the QFontColors used for different values; the order depends on
    // the type.  For example, for a format applied to integers the
    // non-negative QFontColor is first.  If there is a format for all
    // values, this will return an empty vector.
    vector<QFontColor*>* getFonts();

    // Sets the value-dependent QFontColors to the values in the given vector.
    void setFonts(vector<QFontColor*>* f);

    // Adds a font to the end of the value-dependent QFontColor vector.
    void addFont(QFontColor* fc);

    // Returns the date format if this format applies to a date, a blank string
    // otherwise.  See TBConstants::dateFormatIsValid().
    String getDateFormat();

    // Sets the date format for this format.  See
    // TBConstants::dateFormatIsValid().
    void setDateFormat(String d);

    // Returns the boolean format.  If this format is not for a boolean value,
    // this operation is undefined.
    BooleanFormat getBoolFormat();

    // Sets the boolean format.
    void setBoolFormat(BooleanFormat f);

    // Returns true if this format's scientific format is on, false otherwise.
    bool getScientificFormat();

    // Sets the scientific format flag.
    void setScientificFormat(bool sf);

    // Returns the number of decimal places for this format.  If there is an
    // unlimited number of decimal places, or this format doesn't apply to
    // decimals, -1 is returned.
    int getDecimalPlaces();

    // Sets the number of decimal places for this format.
    void setDecimalPlaces(int dp);

    // Returns the vector threshold for this format.  A vector threshold, when
    // applied to one-dimensional arrays, will "hide" the values in the array
    // and display the shape instead IF the size is greater than the specified
    // threshold.  If the threshold is unlimited, or this format does not apply
    // to array types, -1 is returned.
    int getVectorThreshold();

    // Sets the vector threshold of this format.  See getVectorThreshold().
    void setVectorThreshold(int v);

    
    // Applies this format to the display value in the given QTableWidgetItem.
    // The pre-format data and the type must also be provided.
    void applyTo(QTableWidgetItem* item, TBData* data);

private:
    // The QFontColor to apply to all values, or NULL if there are
    // value-dependent QFontColors.
    QFontColor* allFont;

    // The vector of value-dependent QFontColors (can be empty).
    vector<QFontColor*> fonts;

    // The number of decimal places to display, or -1 for unlimited.
    int decimalPlaces;

    // Whether scientific format should be used.
    bool scientificFormat;

    // The format for boolean values.
    BooleanFormat boolFormat;

    // The format for displaying dates.  See TBConstants::dateFormatIsValid().
    String dateFormat;

    // The vector threshold, or -1 for unlimited.
    int vectorThreshold;
};

// <summary>
// Widget for entering format rules for a given field.
// <summary>
//
// <synopsis>
// A TBFormatter is a dialog that can be used to get format rules from the
// user.  The user can then command either to clear the format (and remove all
// formatting) or to set the format for a given field.  Important: this
// behavior is implemented through the use of signals, which means that it is
// the caller's/parent's responsibility for handling the signals and applying
// the formats as needed.
// </synopsis>

class TBFormatter : public QDialog, Ui::Format {
    Q_OBJECT

public:
    // Constructor that takes the field name, type, and index to be formatted,
    // along with the default (unformatted) QFontColor and an optional pointer
    // to the parent widget.  If the parent pointer is NULL, this will be
    // displayed as a dialog; otherwise it can be displayed inside the parent.
    TBFormatter(String field, String type, int index, QFontColor font,
                QWidget* parent = NULL);

    ~TBFormatter();

    
    // Sets the displayed format to the given format.
    void setFormat(TBFormat* f);

signals:
    // This signal is emitted when the user clicks the "Clear Format"
    // button.  The parent/caller should then remove the format for the field
    // with the specified index.
    void clearRequested(int index);

    // This signal is emitted when the user enters a format and then clicks
    // the "Set Format" button.  The parent/caller should then set the format
    // for the specified field to the specified format.
    void setRequested(int index, TBFormat* format);

private:
    // Field being formatted.
    String field;

    // Type of the field being formatted.
    String type;

    // Index of the field being formatted.
    int index;

    // Vector of QFontColors for value-dependent formats.
    vector<QFontColor*> fonts;

    // Flag indicating whether any GUI-generated events are "genuine."
    bool update;

    
    // Collects the QFontColor information currently set in the GUI and returns
    // it.
    QFontColor* getFont();

    // Sets the displayed font and color information to the given QFontColor.
    void setFontColor(QFontColor* color);
    
private slots:
    // Slot for when the user wants a color chooser.  Opens a QColorDialog.
    void changeColor();

    // Slot for when the user clicks the "Clear Format" button.  Emits the
    // clearFormat() signal and closes the window.
    void clearFormat();

    // Slot for when the user clicks the "Set Format" button.  Collects the
    // format information from the widget, emits the setRequested() signal,
    // and closes the window.
    void setFormat();

    // Slot for when the user clicks the "apply to all" checkbox.  If the all
    // box is checked, the list of value-dependent formats is disabled.
    void applyAllTurned(bool on);

    // Slot for when the user clicks in the value-dependent format list.  The
    // selected format is displayed in the widget.
    void applySelectionChanged(int newIndex);

    // Slot for when the user changes a font or color parameter.  The
    // QFontColor in the value-dependent vector is updated accordingly.
    void valuesChanged();
    
private:
    // Unlimited decimals constant.
    static const int UNLIMITED_DECIMALS;
};

}

#endif /* TBFORMAT_H_ */
