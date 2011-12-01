//# QtComboCheckBox.qo.h: Extension of QComboBox to use QCheckBoxes.
//# Copyright (C) 2009
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
#ifndef QTCOMBOCHECKBOX_QO_H_
#define QTCOMBOCHECKBOX_QO_H_

#include <QComboBox>
#include <QItemDelegate>

//#include <casa/namespace.h>
using namespace std;

namespace casa {

// Subclass of QItemDelegate for use with QtComboCheckBox.
// Based on http://da-crystal.net/GCMS/blog/checkboxlist-in-qt/
class QtComboCheckBoxDelegate : public QItemDelegate {
public:
    // Constructor which takes optional parent.
    QtComboCheckBoxDelegate(QObject* parent = NULL);
    
    // Destructor.
    ~QtComboCheckBoxDelegate();

    // Overrides QItemDelegate::paint().
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
            const QModelIndex &index) const;

    // Overrides QItemDelegate::createEditor().
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
            const QModelIndex& index) const;

    // Overrides QItemDelegate::setEditorData().
    void setEditorData(QWidget* editor, const QModelIndex& index) const;

    // Overrides QItemDelegate::setModelData().
    void setModelData(QWidget* editor, QAbstractItemModel* model,
            const QModelIndex& index) const;
};


// Subclass of QComboBox that uses the item delegate above.
// Based on http://da-crystal.net/GCMS/blog/checkboxlist-in-qt/
class QtComboCheckBox : public QComboBox {
    Q_OBJECT

public:
    // Constructor which takes optional parent.
    QtComboCheckBox(QWidget* parent = NULL);

    // Destructor.
    ~QtComboCheckBox();

    // Overrides QObject::eventFilter().
    bool eventFilter(QObject* object, QEvent* event);

    // Gets/Sets the display text.
    // <group>
    QString getDisplayText() const;
    void setDisplayText(const QString& text);
    // </group>
    
    // Gets/Sets whether the checkbox at the given index is toggled.
    // <group>
    bool indexIsToggled(int index) const;
    void toggleIndex(int index, bool toggled);
    // </group>    
    
signals:
    // This signal is emitted when the user has finished changing the
    // checkboxes.
    void stateChanged();
    
protected:
    // Overrides QWidget::paintEvent().
    void paintEvent(QPaintEvent* event);

private:
    // Display text.
    QString itsDisplayText_;
};

}

#endif /* QTCOMBOCHECKBOX_QO_H_ */
