//# QPPanel.cc: Qwt implementation of generic PlotPanel and PlotWidget classes.
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
//# $Id: $
#ifdef AIPS_HAS_QWT

#include <casaqt/QwtPlotter/QPPanel.qo.h>

#include <QHBoxLayout>

namespace casa {

//////////////////////////
// QPBUTTON DEFINITIONS //
//////////////////////////

QPButton::QPButton(const String& text, bool toggleable) : m_text(text) {
    m_button = new QPushButton(text.c_str());
    m_button->setCheckable(toggleable);
    connect(m_button, SIGNAL(clicked()),   SLOT(buttonPushed()));
    connect(m_button, SIGNAL(destroyed()), SLOT(buttonDeleted()));
}

QPButton::~QPButton() {
    if(m_button != NULL) delete m_button;
}


bool QPButton::isEnabled() const { return m_button->isEnabled(); }
void QPButton::setEnabled(bool enabled) { m_button->setEnabled(enabled); }

bool QPButton::isVisible() const { return m_button->isVisible(); }
void QPButton::setVisible(bool visible) { m_button->setVisible(visible); }
String QPButton::tooltip() const { return m_button->toolTip().toStdString(); }

void QPButton::setTooltip(const String& text) {
    m_button->setToolTip(text.c_str()); }


bool QPButton::textShown() const { return !m_button->text().isEmpty(); }
void QPButton::showText(bool show) {
    m_button->setText(show ? m_text.c_str() : ""); }

String QPButton::text() const { return m_text; }
void QPButton::setText(const String& text) {
    m_text = text;
    m_button->setText(text.c_str());
}

bool QPButton::imageShown() const { return !m_button->icon().isNull(); }
void QPButton::showImage(bool show) {
    m_button->setIcon(QIcon(show ? m_imageLoc.c_str() : "")); }

void QPButton::setImagePath(const String& imgPath) {
    m_imageLoc = imgPath;
    m_button->setIcon(QIcon(imgPath.c_str()));
}

bool QPButton::isToggleable() const { return m_button->isCheckable(); }
void QPButton::setToggleable(bool toggleable) {
    m_button->setCheckable(toggleable); }

bool QPButton::isToggled() const { return m_button->isChecked(); }
void QPButton::setToggled(bool toggled) {
    if(m_button->isCheckable()) m_button->setChecked(toggled); }

void QPButton::registerHandler(PlotButtonEventHandlerPtr handler) {
    for(unsigned int i = 0; i < m_handlers.size(); i++)
        if(m_handlers[i] == handler) return;
    m_handlers.push_back(handler);
}

vector<PlotButtonEventHandlerPtr> QPButton::allHandlers() const {
    return m_handlers; }

void QPButton::unregisterHandler(PlotButtonEventHandlerPtr handler) {
    for(unsigned int i = 0; i < m_handlers.size(); i++) {
        if(m_handlers[i] == handler) {
            m_handlers.erase(m_handlers.begin() + i);
            break;
        }
    }
}


QPushButton* QPButton::asQPushButton() { return m_button; }
const QPushButton* QPButton::asQPushButton() const { return m_button; }


void QPButton::buttonPushed() {
    if(m_handlers.size() == 0) return;
    
    PlotButtonEvent e(this);
    for(unsigned int i = 0; i < m_handlers.size(); i++)
        m_handlers[i]->handleButton(e);
}

void QPButton::buttonDeleted() { m_button = NULL; }


////////////////////////////
// QPCHECKBOX DEFINITIONS //
////////////////////////////

QPCheckbox::QPCheckbox(const String& text) {
    m_checkbox = new QCheckBox(text.c_str());
    connect(m_checkbox, SIGNAL(toggled(bool)), SLOT(checkboxToggled()));
    connect(m_checkbox, SIGNAL(destroyed()),   SLOT(checkboxDeleted()));
}

QPCheckbox::~QPCheckbox() {
    if(m_checkbox != NULL) delete m_checkbox;
}


bool QPCheckbox::isEnabled() const { return m_checkbox->isEnabled(); }
void QPCheckbox::setEnabled(bool enabled) { m_checkbox->setEnabled(enabled); }

bool QPCheckbox::isVisible() const { return m_checkbox->isVisible(); }
void QPCheckbox::setVisible(bool visible) { m_checkbox->setVisible(visible); }

String QPCheckbox::tooltip() const { return m_checkbox->toolTip().toStdString(); }
void QPCheckbox::setTooltip(const String& text) {
    m_checkbox->setToolTip(text.c_str()); }

String QPCheckbox::text() const { return m_checkbox->text().toStdString(); }
void QPCheckbox::setText(const String& text) {
    m_checkbox->setText(text.c_str()); }

bool QPCheckbox::isChecked() const { return m_checkbox->isChecked(); }
void QPCheckbox::setChecked(bool checked) { m_checkbox->setChecked(checked); }

void QPCheckbox::registerHandler(PlotCheckboxEventHandlerPtr handler) {
    for(unsigned int i = 0; i < m_handlers.size(); i++)
        if(m_handlers[i] == handler) return;
    m_handlers.push_back(handler);
}

vector<PlotCheckboxEventHandlerPtr> QPCheckbox::allHandlers() const {
    return m_handlers; }

void QPCheckbox::unregisterHandler(PlotCheckboxEventHandlerPtr handler) {
    for(unsigned int i = 0; i < m_handlers.size(); i++) {
        if(m_handlers[i] == handler) {
            m_handlers.erase(m_handlers.begin() + i);
            break;
        }
    }
}


QCheckBox* QPCheckbox::asQCheckBox() { return m_checkbox; }
const QCheckBox* QPCheckbox::asQCheckBox() const { return m_checkbox; }


void QPCheckbox::checkboxToggled() {
    if(m_handlers.size() == 0) return;
    
    PlotCheckboxEvent e(this);
    for(unsigned int i = 0; i < m_handlers.size(); i++)
        m_handlers[i]->handleCheckbox(e);
}

void QPCheckbox::checkboxDeleted() { m_checkbox = NULL; }


/////////////////////////
// QPPANEL DEFINITIONS //
/////////////////////////

QPPanel::QPPanel() {
    m_frame = new QFrame();
    QHBoxLayout* l = new QHBoxLayout(m_frame);
#if QT_VERSION >= 0x040300
    l->setContentsMargins(0, 0, 0, 0);
#else
    l->setMargin(0);
#endif
    l->setSpacing(3);
    
    connect(m_frame, SIGNAL(destroyed()), SLOT(frameDeleted()));
}

QPPanel::~QPPanel() {
    if(m_frame != NULL) delete m_frame;
}


vector<PlotWidgetPtr> QPPanel::widgets() const { return m_widgets; }

int QPPanel::addWidget(PlotWidgetPtr widget) {
    // make sure it's a QPButton or QPCheckbox
    QPButton* qb = dynamic_cast<QPButton*>(&*widget);
    QPCheckbox* qc = dynamic_cast<QPCheckbox*>(&*widget);
    if(qb == NULL && qc == NULL) return -1;
    
    // make sure it's not already in the panel
    for(unsigned int i = 0; i < m_widgets.size(); i++)
        if(m_widgets[i] == widget) return i;
    
    QHBoxLayout* layout = dynamic_cast<QHBoxLayout*>(m_frame->layout());
    
    if(qb != NULL)
        layout->insertWidget(layout->count() - 1, qb->asQPushButton());
    else if(qc != NULL)
        layout->insertWidget(layout->count() - 1, qc->asQCheckBox());
    
    if(layout->count() == 1) layout->addStretch();
    
    m_widgets.push_back(widget);
    return m_widgets.size() - 1;
}

void QPPanel::clearWidgets() {
    QHBoxLayout* layout = dynamic_cast<QHBoxLayout*>(m_frame->layout());
    while(layout->count() > 0) layout->removeItem(layout->itemAt(0));
    m_widgets.clear();
}

void QPPanel::removeWidget(PlotWidgetPtr widget) {
    unsigned int i;
    for(i = 0; i < m_widgets.size(); i++)
        if(m_widgets[i] == widget) break;
    if(i == m_widgets.size()) return;
    else removeWidget(i);
}

void QPPanel::removeWidget(int i) {
    if(i < 0 || (unsigned int)i >= m_widgets.size()) return;
    
    QPButton* qb = dynamic_cast<QPButton*>(&*m_widgets[i]);
    QPCheckbox* qc = dynamic_cast<QPCheckbox*>(&*m_widgets[i]);
    if(qb == NULL && qc == NULL) return; // shouldn't happen
    
    QHBoxLayout* layout = dynamic_cast<QHBoxLayout*>(m_frame->layout());
    if(qb != NULL) layout->removeWidget(qb->asQPushButton());
    else if(qc != NULL) layout->removeWidget(qc->asQCheckBox());
    m_widgets.erase(m_widgets.begin() + i);
    
    if(layout->count() == 1) layout->removeItem(layout->itemAt(0)); // stretch
}


QFrame* QPPanel::asQFrame() { return m_frame; }
const QFrame* QPPanel::asQFrame() const { return m_frame; }


void QPPanel::frameDeleted() { m_frame = NULL; }

}

#endif
