//# QtComboCheckBox.cc: Extension of QComboBox to use QCheckBoxes.
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
#include <casaqt/QtUtilities/QtComboCheckBox.qo.h>

#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QStylePainter>

namespace casa {

/////////////////////////////////////////
// QTCOMBOCHECKBOXDELEGATE DEFINITIONS //
/////////////////////////////////////////

QtComboCheckBoxDelegate::QtComboCheckBoxDelegate(QObject* parent) :
        QItemDelegate(parent) { }

QtComboCheckBoxDelegate::~QtComboCheckBoxDelegate() { }

void QtComboCheckBoxDelegate::paint(QPainter* painter,
        const QStyleOptionViewItem& option, const QModelIndex& index) const {
    // Get item data
    bool value = index.data(Qt::UserRole).toBool();
    QString text = index.data(Qt::DisplayRole).toString();

    // Fill style options with item data
    const QStyle* style = QApplication::style();
    QStyleOptionButton opt;
    opt.state |= value ? QStyle::State_On : QStyle::State_Off;
    opt.state |= QStyle::State_Enabled;
    opt.text = text;
    opt.rect = option.rect;

    // Draw item data as CheckBox
    style->drawControl(QStyle::CE_CheckBox, &opt, painter);
}

QWidget* QtComboCheckBoxDelegate::createEditor(QWidget* parent,
        const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QCheckBox* b = new QCheckBox(parent);
    b->resize(option.rect.size());
    b->setFocusPolicy(Qt::StrongFocus);
    b->setAutoFillBackground(true);
    return b;
}

void QtComboCheckBoxDelegate::setEditorData(QWidget* editor,
        const QModelIndex& index) const {
    // Set editor data
    QCheckBox* myEditor = static_cast<QCheckBox*>(editor);
    myEditor->setText(index.data(Qt::DisplayRole).toString());
    myEditor->setChecked(index.data(Qt::UserRole).toBool());
}

void QtComboCheckBoxDelegate::setModelData(QWidget* editor,
        QAbstractItemModel* model, const QModelIndex& index) const {
    // Get the value from the editor (QCheckBox)
    QCheckBox* myEditor = static_cast<QCheckBox*>(editor);
    bool value = myEditor->isChecked();

    // Set model data
    QMap<int, QVariant> data;
    data.insert(Qt::DisplayRole, myEditor->text());
    data.insert(Qt::UserRole, value);
    model->setItemData(index, data);
}


/////////////////////////////////
// QTCOMBOCHECKBOX DEFINITIONS //
/////////////////////////////////

QtComboCheckBox::QtComboCheckBox(QWidget* parent) : QComboBox(parent) {
    // Set delegate items view
    QtComboCheckBoxDelegate* d = new QtComboCheckBoxDelegate(this);
    view()->setItemDelegate(d);
    connect(d, SIGNAL(commitData(QWidget*)), SIGNAL(stateChanged()));
    
    // Enable editing on items view
    view()->setEditTriggers(QAbstractItemView::CurrentChanged);

    // Set "QtComboCheckBox::eventFilter" as event filter for items view
    view()->viewport()->installEventFilter(this);
    
    // Set properties.
    setEditable(false);
} 

QtComboCheckBox::~QtComboCheckBox() { }

bool QtComboCheckBox::eventFilter(QObject* object, QEvent* event) {
    // don't close items view after we release the mouse button
    // by simply eating MouseButtonRelease in viewport of items view
    if(event->type() == QEvent::MouseButtonRelease &&
       object == view()->viewport()) return true;

    return QComboBox::eventFilter(object, event);
}

QString QtComboCheckBox::getDisplayText() const { return itsDisplayText_; }
void QtComboCheckBox::setDisplayText(const QString& text) {
    itsDisplayText_ = text; }

bool QtComboCheckBox::indexIsToggled(int index) const {
    QVariant data = itemData(index, Qt::UserRole);
    return data.isValid() && data.toBool();
}

void QtComboCheckBox::toggleIndex(int index, bool toggled) {
    if(index >= 0 && index < count())
        setItemData(index, toggled, Qt::UserRole);
}


void QtComboCheckBox::paintEvent(QPaintEvent* event) {
    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    // Draw the combobox frame, focusrect and selected etc.
    QStyleOptionComboBox opt;
    initStyleOption(&opt); 

    // If no display text been set , use "..." as default
    if(itsDisplayText_.isEmpty()) opt.currentText = "...";
    else opt.currentText = itsDisplayText_;

    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

    // Draw the icon and text
    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

}
