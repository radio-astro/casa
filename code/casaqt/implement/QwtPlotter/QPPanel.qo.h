//# QPPanel.h: Qwt implementation of generic PlotPanel and PlotWidget classes.
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
#ifndef QPPANEL_H_
#define QPPANEL_H_

#ifdef AIPS_HAS_QWT

#include <graphics/GenericPlotter/PlotPanel.h>

#include <QCheckBox>
#include <QFrame>
#include <QPushButton>

#include <casa/namespace.h>

namespace casa {

// Implementation of QPButton for Qwt plotter, using a QPushButton.
class QPButton : public QObject, public virtual PlotButton {
    Q_OBJECT
    
public:
    // Constructor which takes the button text and whether or not the button
    // should be toggleable.
    QPButton(const String& text, bool toggleable = false);
    
    // Destructor.
    ~QPButton();
    
    
    // PlotWidget Methods //
    
    // Implements PlotWidget::isEnabled().
    bool isEnabled() const;
    
    // Implements PlotWidget::setEnabled().
    void setEnabled(bool enabled = true);
    
    // Implements PlotWidget::isVisible().
    bool isVisible() const;
    
    // Implements PlotWidget::setVisible().
    void setVisible(bool visible = true);
    
    // Implements PlotWidget::tooltip().
    String tooltip() const;
    
    // Implements PlotWidget::setTooltip().
    void setTooltip(const String& text);
    
    
    // PlotButton Methods //
    
    // Implements PlotButton::textShown().
    bool textShown() const;
    
    // Implements PlotButton::showText().
    void showText(bool show = true);
    
    // Implements PlotButton::text().
    String text() const;
    
    // Implements PlotButton::setText().
    void setText(const String& text);
    
    // Implements PlotButton::imageShown().
    bool imageShown() const;
    
    // Implements PlotButton::showImage().
    void showImage(bool show = true);
    
    // Implements PlotButton::setImagePath().
    void setImagePath(const String& imgPath);
    
    // Implements PlotButton::isToggleable().
    bool isToggleable() const;
    
    // Implements PlotButton::setToggleable().
    void setToggleable(bool toggleable = true);
    
    // Implements PlotButton::isToggled().
    bool isToggled() const;
    
    // Implements PlotButton::setToggled().
    void setToggled(bool toggled = true);
    
    // Implements PlotButton::registerHandler().
    void registerHandler(PlotButtonEventHandlerPtr handler);
    
    // Implements PlotButton::allHandlers().
    vector<PlotButtonEventHandlerPtr> allHandlers() const;
    
    // Implements PlotButton::unregisterHandler().
    void unregisterHandler(PlotButtonEventHandlerPtr handler);
    
    
    // QPButton Methods //
    
    // Provides access to the underlying QPushButton.
    // <group>
    QPushButton* asQPushButton();
    const QPushButton* asQPushButton() const;
    // </group>
    
private:
    QPushButton* m_button; // Button
    String m_text;         // Text
    String m_imageLoc;     // Image location
    
    vector<PlotButtonEventHandlerPtr> m_handlers; // Event handlers
    
private slots:
    // For when the button is pushed/toggled.
    void buttonPushed();
    
    // For when the button is deleted.
    void buttonDeleted();
};


// Implementation of PlotCheckbox for Qwt plotter, using a QCheckBox.
class QPCheckbox : public QObject, public virtual PlotCheckbox {
    Q_OBJECT
    
public:
    // Constructor that takes the checkbox text.
    QPCheckbox(const String& text);
    
    // Destructor.
    ~QPCheckbox();

    
    // PlotWidget Methods //
    
    // Implements PlotWidget::isEnabled().
    bool isEnabled() const;
    
    // Implements PlotWidget::setEnabled().
    void setEnabled(bool enabled = true);
    
    // Implements PlotWidget::isVisible().
    bool isVisible() const;
    
    // Implements PlotWidget::setVisible().
    void setVisible(bool visible = true);
    
    // Implements PlotWidget::tooltip().
    String tooltip() const;
    
    // Implements PlotWidget::setTooltip().
    void setTooltip(const String& text);
    
    
    // PlotCheckbox Methods //
    
    // Implements PlotCheckbox::text().
    String text() const;
    
    // Implements PlotCheckbox::setText().
    void setText(const String& text);
    
    // Implements PlotCheckbox::isChecked().
    bool isChecked() const;
    
    // Implements PlotCheckbox::setChecked().
    void setChecked(bool checked = true);
    
    // Implements PlotCheckbox::registerHandler().
    void registerHandler(PlotCheckboxEventHandlerPtr handler);
    
    // Implements PlotCheckbox::allHandlers().
    vector<PlotCheckboxEventHandlerPtr> allHandlers() const;
    
    // Implements PlotCheckbox::unregisterHandler().
    void unregisterHandler(PlotCheckboxEventHandlerPtr handler);
    
    
    // QPCheckbox Methods //
    
    // Provides access to the underlying QCheckBox.
    // <group>
    QCheckBox* asQCheckBox();
    const QCheckBox* asQCheckBox() const;
    // </group>
    
private:
    QCheckBox* m_checkbox; // Checkbox
    vector<PlotCheckboxEventHandlerPtr> m_handlers; // Event handlers
    
private slots:
    // For when the checkbox has been toggled.
    void checkboxToggled();
    
    // For when the checkbox has been deleted.
    void checkboxDeleted();
};


// Implementation of PlotPanel for Qwt plotter, using a QFrame.
class QPPanel : public QObject, public virtual PlotPanel {
    Q_OBJECT
    
public:
    // Constructor.
    QPPanel();
    
    // Destructor.
    ~QPPanel();

    
    // PlotPanel Methods //
    
    // Implements PlotPanel::widgets().
    vector<PlotWidgetPtr> widgets() const;
    
    // Implements PlotPanel::addWidget().
    int addWidget(PlotWidgetPtr widget);
    
    // Implements PlotPanel::clearWidgets().
    void clearWidgets();
    
    // Implements PlotPanel::removeWidget().
    // <group>
    void removeWidget(PlotWidgetPtr widget);
    void removeWidget(int index);
    // </group>
    
    
    // QPPanel Methods //
    
    // Provides access to the underlying QFrame.
    // <group>
    QFrame* asQFrame();    
    const QFrame* asQFrame() const;
    // </group>
    
private:
    QFrame* m_frame;                 // Frame
    vector<PlotWidgetPtr> m_widgets; // Widgets
    
private slots:
    // For when the frame is deleted.
    void frameDeleted();
};

}

#endif

#endif /*QPPANEL_H_*/
