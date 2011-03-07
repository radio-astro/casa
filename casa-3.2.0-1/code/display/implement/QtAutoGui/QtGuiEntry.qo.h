//# QtGuiEntry.qo.h: Individual interface elements for general-purpose
//#               Qt options widget (QtAutoGui).
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
//# $Id$

#ifndef QTGUIENTRY_H
#define QTGUIENTRY_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/BasicMath/Math.h>

#include <graphics/X11/X_enter.h>
#  include <QDomDocument>
#  include <QCheckBox>
   //#dk Be careful to put ui_*.h within X_enter/exit bracket too,
   //#   because they'll have Qt includes.
   //#   E.g. <QApplication> needs the X11 definition of 'Display'
#  include "adjustmentBottom.ui.h"
#  include "sliderLabelItem.ui.h"
#  include "sliderEditorItem.ui.h"
#  include "lineEditorItem.ui.h"
#  include "comboItem.ui.h"
#  include "checkButtonItem.ui.h"
#  include "pushButtonItem.ui.h"
#  include "minMaxEditorItem.ui.h"
#  include "pairItem.ui.h"
#include <graphics/X11/X_exit.h>

#include <utility>

 
namespace casa { //# NAMESPACE CASA - BEGIN


class QtAutoGui;


//# ////////////////   QtAdjustmentBottom //////////////////////////////////

class QtAdjustmentTop : public QWidget, private Ui::AdjustmentBottom {
    Q_OBJECT
public:
    QtAdjustmentTop(QtAutoGui *parent=0, QString name="");
    void setText(QString txt) {dataName->setText(txt);}
    ~QtAdjustmentTop();
public slots:   
   void dataNameChanged(QString value); 
   void setOriginal();
   void setDefault();
   void setMemory();
   void setClear();
   void setCopy();
   void setPaste();    
   void apply();
   void load();
   void save();
   void restore();
   void close();
   
   void hideDismiss();		//#dk

private:
   QtAutoGui *parent; 
   bool blockSignal;
};


//# ///////////////////  QtSliderBase //////////////////////////////////////


// <summary>
// Base class for QtAutoGui slider interface elements.
// </summary>

// <synopsis>
// This is a base with common functionality for the two slider 'guientries':
// QtSliderEditor, which has a value edit box, and QtSliderLabel, with a
// non-editable value label.  This class is abstract; its constructor can only
// be called by a derived class.  That class should also call constructBase()
// after cxreating its interface elements, to complete construction.
// </synopsis>

class QtSliderBase : public QWidget {
  
  Q_OBJECT	//# Allows slot/signal definition.  Must only occur in
		//# implement/.../*.h files; also, makefile must include
		//# name of this file in 'mocs' section.
 
 public:
  
  // Set interface label text ('listname').
  void setLabelText(QString txt) { nameLabel_->setText(txt);  }
  
  // Internal name ('dlformat')
  QString name(){ return itemName; }

  // Set up main state (value, etc.) and external appearance of
  // this widget, according to options record (passed as a QDomElement).
  // Called via constructor, and from QtAutoGui::changeOptions().
  // Does not trigger (significant) signals; it is intended to set this
  // interface element to library's internal value (not vice versa).
  void reSet(QDomElement& ele);
 
 protected:

  // Only for use by derived classes.
  // <group>
  QtSliderBase(QWidget *parent=0) : QWidget(parent) {  }
  ~QtSliderBase() {  }
  // </group>
  
  // Derived class should call this within its constructor, after it
  // has a valid QSlider, name QLabel and menu QToolButton (usually,
  // after calling setUi()), passing them down in the parameters below.
  // ele is the QDomElement (an xml version of an option Record) used to
  // construct the derived class.
  void constructBase(QDomElement &ele,  QSlider* slider,
		     QLabel* nameLabel, QToolButton* menuBtn);

  // Fetch numeric value of attribute of ele named attnm, into val. 
  // Does nothing if attr doesn't exist or is not valid numerically.
  void getAttr(const QDomElement& ele, QString attnm, Double& val);

 protected slots:   
   
  // slider changed by user
  virtual void slChg(int slval) { update(externalVal(slval));
				  if ( ! onrelease_ ) emitVal( ); }
  virtual void release( ) 	{ if (   onrelease_ ) emitVal( ); }
  
  // 'revert-to-original' selected in menu.
  virtual void setOriginal();
   
 
 signals:
  
  // Main output signal.  Main parameters of interest are item name and
  // new value.
  void itemValueChanged(QString name, QString value, int action, bool apply);
  
  // class user may wish to connect to and display these.
  void errMsg(String errmsg);   

    
 protected: 
  
  // Derived class should implement: set value text box or label (if any)
  // to (normalized) current value.  (Should not trigger class's own slots).
  virtual void updateText() = 0;
    
  
  Int round(Double val) { return ifloor(val + .5);  }
  
  // Integer values with in [0, slMax_] on the internal slider are scaled
  // to external values within [dMin_, dMax_].
  // These two routines convert between the two scalings.
  // <group>
  Double externalVal(Int sldVal) {
    return min(dMax_, (dMin_ + max(0., dIncr_*sldVal)));  }
    
  Int sliderVal(Double extVal) {
    return round(max(0., min(slMax_, (extVal-dMin_)/dIncr_)));  }
  // </group>
  
  // These two routines convert 'external' values between Double and
  // text.
  // <group>
  QString toText(Double val);
  Double toNumber(QString text, bool* ok=0);
  // </group>
  
  // Current external value, as text.
  QString textVal() { return toText(dVal_);  }
  
  // Adjusts integer slider range (slMax_) and the increment that each
  // unit on the slider represents (dIncr_), according to latest
  // increment and range request (dIncr0_,  dMin_, dMax_).
  void adjSlIncr();

  // Sets slider to latest range and value (without
  // triggering slider signals or slChg slot below). 
  void updateSlider();

  // emit current value -- widget's main output.
  void emitVal();

  // Accepts new value (if any), updates interface (without
  // retriggering any internal slots).
  // New value should already have been validated.
  void update(Double dval);

    
  // main state.  Determines the float scaling for the (necessarily int)
  // QSlider.  dVal_ is the slider's 'external' value (which may be
  // floating-point).  dMin_ and dMax_ are its 'external' range.
  // dIncr0_ is the requested 'external' slider increment ('presolution'),
  // dIncr_ is the increment actually used (normally, these will be the same).
  //
  // Internally, the slider's range will be [0, slMax_]; slMax_
  // will be a non-negative integer value.  Both of the QSlider's
  // internal increments (singleStep, pageStep) will be 1.

  Double dVal_,  dMin_, dMax_,  dIncr_, dIncr0_,  slMax_;
  
  // Whether this element emits float values (ptype 'floatrange')
  // vs. ints (ptype 'intrange').
  Bool floatrng_;

  // Should the event be generated only upon releasing the slider?
  bool onrelease_;

  // For restoring 'original' value.
  Double origVal_;
  
  // Relevant user interface elements.  Must be created by
  // derived class and passed in via constructBase().
  // <group>
  QSlider* slider_;
  QLabel* nameLabel_;
  QToolButton* menuBtn_;
  // </group>
    
  // Internal name ('dlformat').
  QString itemName;  
  
};



//# ////////////////////  QtSliderEditor ///////////////////////////////////


class QtSliderEditor : public QtSliderBase, private Ui::SliderEditorItem {
  
  Q_OBJECT	//# Allows slot/signal definition.  Must only occur in
		//# implement/.../*.h files; also, makefile must include
		//# name of this file in 'mocs' section.

 public:
  
  QtSliderEditor(QDomElement& ele, QWidget *parent=0);
  ~QtSliderEditor() {  }

 protected:

  // (not intended for use at present).
  QtSliderEditor(QWidget *parent=0): QtSliderBase(parent) { 
    setupUi(this);
    radioButton->hide();  }
  
  // Sets text box to (normalized) current value.
  // (Should not trigger class's own slots either).
  void updateText() { lineEdit->setText(textVal());  }

  // main validation/update routine for changes to text version of value.
  // strval is the candidate text, which is still to be validated/normalized.
  void textChg(QString strval);
   
 protected slots:   
   
  //# text edited by user
  virtual void edited()      { textChg(lineEdit->text());  }
  
  //# // Triggered when 'revert-to-original' selected in menu
  //# virtual void setOriginal() { textChg(toText(origVal_));  }
  //#	//# (This version _will expand slider limits, if necessary).
  
};



//# ///////////////////////  QtSliderLabel /////////////////////////////////


class QtSliderLabel : public QtSliderBase, private Ui::SliderLabelItem {
  
  Q_OBJECT	//# Allows slot/signal definition.  Must only occur in
		//# implement/.../*.h files; also, makefile must include
		//# name of this file in 'mocs' section.

 public:
  
  QtSliderLabel(QDomElement& ele, QWidget *parent=0);
  ~QtSliderLabel() {  }

 protected:

  // (not intended for use at present).
  QtSliderLabel(QWidget *parent=0): QtSliderBase(parent) { setupUi(this);  }
  
  // Sets value label to (normalized) current value.
  void updateText();
   
};




//# ////////////////////////  QtMinMaxEditor ///////////////////////////////

class QtMinMaxEditor : public QWidget, private Ui::MinMaxEditorItem {
    Q_OBJECT
public:
     QtMinMaxEditor(QWidget *parent=0);
     QtMinMaxEditor(QDomElement &ele, QWidget *parent=0);
    ~QtMinMaxEditor();
    bool validate(QString value);
    void setLabelText(QString txt) {nameLabel->setText(txt);}
    QString name(){ return itemName; } 
    void reSet(QString);
public slots:  
   void display2(); 
   void display2(int value);  
   void display2(QString value);
   void setOriginal();
   void setDefault();
   void setCopy();
   void setPaste();
   void setHistogram();   
signals:
   void itemValueChanged(QString name, QString value, int action, bool apply);
private:
   QString itemName;  
   double scale;
   bool blockSignal;
   QString orig;
};


//# ////////////////////////  QtLineEditor  ////////////////////////////////

class QtLineEditor : public QWidget, private Ui::LineEditorItem {
    Q_OBJECT
public:
     QtLineEditor(QWidget *parent=0);
     QtLineEditor(QDomElement &ele, QWidget *parent=0);
    ~QtLineEditor();
    Bool validate(QString value);
    void setLabelText(QString txt) {nameLabel->setText(txt);}
    QString name(){ return itemName; } 
    void reSet(QString);
public slots:   
   void validateAndEmit(QString value);    
   void editingFinished();
   void setOriginal();
   void setDefault();
   void setCopy();
   void setPaste();   
signals:
   void itemValueChanged(QString name, QString value, int action, bool apply);
private:
   QString ptype;
   QString itemName;  
   QString origValue;
   bool blockSignal;
};


//# ////////////////////////   QtCombo  ////////////////////////////////////

class QtCombo : public QWidget, private Ui::ComboItem {
    Q_OBJECT
public:
     QtCombo(QWidget *parent=0);
     QtCombo(QDomElement &ele, QWidget *parent=0);
    ~QtCombo();
    void setLabelText(QString txt) {nameLabel->setText(txt);}
    QString name(){ return itemName; } 
    void reSet(QString);
public slots:   
   void setOriginal();
   void setDefault();
   void setCopy();
   void setPaste();    
signals:
   void itemValueChanged(QString name, QString value, int action, bool apply);
protected slots:   
   void emitValue(); 
private:
   QString itemName;     
   bool blockSignal;
   QString orig;
};


//# ////////////////////////   QtBoolean  //////////////////////////////////

class QtBool : public QWidget, private Ui::ComboItem {
    Q_OBJECT
public:
     QtBool(QWidget *parent=0);
     QtBool(QDomElement &ele, QWidget *parent=0);
    ~QtBool();
    void setLabelText(QString txt) {nameLabel->setText(txt);}
    QString name(){ return itemName; } 
    void reSet(QString);
public slots:   
   void setOriginal();
   void setDefault();
   void setCopy();
   void setPaste();    
signals:
   void itemValueChanged(QString name, QString value, int action, bool apply);
protected slots:   
   void emitValue(); 
private:
   QString itemName;     
   bool blockSignal;
   QString orig;
};


//# ////////////////////////  QtPushButton  ////////////////////////////////

class QtPushButton : public QWidget, private Ui::PushButtonItem {
    Q_OBJECT
public:
     QtPushButton(QWidget *parent=0);
     QtPushButton(QDomElement &ele, QWidget *parent=0);
    ~QtPushButton();
    void setLabelText(QString txt) {nameLabel->setText(txt);}
    QString name(){ return itemName; } 
    void reSet(QString);
signals:
   void itemValueChanged(QString name, QString value, int action, bool apply);
protected slots:   
   void emitValue(); 
private:
   QString itemName;     
   bool blockSignal;
};


//# ////////////////////////  QtCheck  /////////////////////////////////////

class QtCheck : public QWidget, private Ui::CheckItem {
    Q_OBJECT
public:
     QtCheck(QWidget *parent=0);
     QtCheck(QDomElement &ele, QWidget *parent=0);
    ~QtCheck();
    void setLabelText(QString txt) {nameLabel->setText(txt);}
    QString name(){ return itemName; } 
    // [Re]sets state of checkboxes and internal record of that,
    // without emitting itemValueChanged.
    void reSet(QString);
    
public slots:   
   void setOriginal();
   void setDefault();
   void setCopy();
   void setPaste();     
signals:
   void itemValueChanged(QString name, QString value, int action, bool apply);
protected slots:   
   void checkboxChanged(int newState);
   void emitValue(); 
   void reSet(QStringList);
   void reSet();
private:
   QList<QCheckBox*> checkboxes_;	 //# all checkboxes
   QStringList allopts_;		//# Names of all checkboxes
   QStringList opts_;		//# Names of currently checked boxes
   QStringList orig_;		//# Names of boxes checked originally
   QString itemName;     
};


//# //////////////////////QtRegion//////////////////////////////////////////

class QtRegionEditor : public QWidget, private Ui::LineEditorItem {
    Q_OBJECT
public:
     QtRegionEditor(QWidget *parent=0);
     QtRegionEditor(QDomElement &ele, QWidget *parent=0);
    ~QtRegionEditor();
    bool validate(QString value);
    void setLabelText(QString txt) {nameLabel->setText(txt);}
    QString name(){ return itemName; } 
    void reSet(QString);
public slots:   
   void display2(QString value);    
   void editingFinished();
   void setOriginal();
   void setDefault();
   void fromImg();
   void createRegion();
    void unset();
   void setCopy();
   void setPaste();
signals:
   void itemValueChanged(QString name, QString value, int action, bool apply);
   
private:
   QString ptype;
   QString itemName;  
   QString iamunset;
   bool blockSignal;
};


//# ////////////////////// QtPair ///////////////////////////////////////////

class QtPairEditor : public QWidget, private Ui::PairItem {
    Q_OBJECT  
    
public:
    QtPairEditor(QDomElement& ele, QWidget* parent = NULL);
    
    ~QtPairEditor();
    
    void reSet(QString value);
    
signals:
    void itemValueChanged(QString name, QString value, int action, bool apply);
    
protected slots:
    void editingFinished();
    
    void setOriginal();
    
private:
    QString itemName;
    bool isDouble;
    QAbstractSpinBox* val1, *val2;
    std::pair<int, int> origVal1;
    std::pair<double, double> origVal2;
    
    std::pair<int, int> readIntPair(QString value);
    
    std::pair<double, double> readDoublePair(QString value);
};


//# ///////////////////// QtNumber ////////////////////////////////////////

// can use PairItem because it's just a frame
class QtNumberEditor : public QWidget, private Ui::PairItem {
    Q_OBJECT
    
public:
    QtNumberEditor(QDomElement& ele, QWidget* parent = NULL);
    
    ~QtNumberEditor();
    
    void reSet(QString value);
    
signals:
    void itemValueChanged(QString name, QString value, int action, bool apply);
    
protected slots:
    void editingFinished();
    
    void setOriginal();
    
private:
    QString itemName;
    bool isDouble;
    QAbstractSpinBox* val;
    int origVal1;
    double origVal2;
};


} //# NAMESPACE CASA - END



#endif
