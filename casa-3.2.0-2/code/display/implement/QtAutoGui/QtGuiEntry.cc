//# QtGuiEntry.cc: Individual interface elements for general-purpose
//#                Qt options widget (QtAutoGui).
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
//#

#include <casa/iostream.h>
#include "QtGuiEntry.qo.h"
#include "QtAutoGui.qo.h"

#include <graphics/X11/X_enter.h>
#  include <QtCore> 
#  include <QtGui>
#include <graphics/X11/X_exit.h>

#include <limits>

namespace casa { //# NAMESPACE CASA - BEGIN


//#dk extern QString clipBoard;



// ///////////////////// QtSliderBase ///////////////////////////////////


void QtSliderBase::constructBase(QDomElement& ele,  QSlider* slider,
				 QLabel* nameLabel, QToolButton* menuBtn) {
  // Derived class should call this within its constructor, after it
  // has a valid QSlider, name QLabel and menu QToolButton (usually,
  // after calling setUi()), passing them down in parameters below.

  slider_   = slider;
  nameLabel_= nameLabel;	// Relevant Qt widgets.
  menuBtn_  = menuBtn;		// derived class creates these.
  
  itemName = ele.tagName();
	// ("dlformat" -- widget name for internal identification
	//  and communication -- this won't change).
	
  slider_->setPageStep(1);	// (singleStep is already 1 by default).
  
  floatrng_ = ele.attribute("ptype") == "floatrange";
	// Whether widget should emit float or integer values.

  onrelease_ = ele.attribute("onrelease") == "1" ? true : false;

  connect(slider_, SIGNAL(valueChanged(int)),     SLOT(slChg(int)) );
  connect(slider_, SIGNAL(sliderReleased( )),     SLOT(release( )) );

  // Store main state.
 
  dVal_ = dMin_ = dMax_ = 0.;  dIncr0_ = 1.;
	// Default values in case nothing else is provided in ele.

  reSet(ele);	// validate and set main internal state,
  		// according to options record.
  
  origVal_=dVal_;  }
	// Save 'original' value, for restore via 'original' menu.
  
  
void QtSliderBase::reSet(QDomElement& ele) {
  // set up main state and external appearance of this widget,
  // according to options record (passed as a QDomElement).  For sliders,
  // this includes value, min, max, slider increment, label and help
  // text.  In ele, these will be the attributes "value", "pmin", "pmax",
  // "presolution", "listname" and "help", respectively.
  // State not specified in ele is left unchanged (if possible).
  // But whether passed or defaulted, numeric state (min, value, max, incr)
  // must be self-consistent; it will be altered to make it so if not.
  //
  // This method just sets the widget's state and appearance, _without_
  // triggering signals caught by this class, nor the class's output
  // signal (itemValueChanged).  It is intended to set this interface
  // to library's internal state (not vice versa).

  
  QString attr;
  attr = ele.attribute("listname");
  if(!attr.isNull()) nameLabel_->setText(attr);
  attr = ele.attribute("help");
  if(!attr.isNull()) nameLabel_->setToolTip(attr);
	// Widget label and help, as the gui user should see it.
  
  // fetch main numeric state from ele, if it exists there.
  getAttr(ele, "value",       dVal_);
  getAttr(ele, "pmin",        dMin_);
  getAttr(ele, "pmax",        dMax_);
  getAttr(ele, "presolution", dIncr0_);
   
  // Assure validity/robustness of that state (will do nothing
  // in most cases, given well-behaved input from ele).
  if(dIncr0_ <  0.) dIncr0_ = -dIncr0_;
  if(dIncr0_ == 0.) dIncr0_ = 1.;
  if(dMin_ > dMax_) { Double tmp=dMin_; dMin_=dMax_; dMax_=tmp;  }
  if(dMin_ > dVal_) dMin_ = dVal_;
  if(dMax_ < dVal_) dMax_ = dVal_; 

   
  adjSlIncr();	// Adjust integer slider range (slMax_) and the increment
		// that each unit on slider represents (dIncr_).
  updateText();		// Set initial values onto value entry/label
  updateSlider();  }	// and slider.
  

  
QString QtSliderBase::toText(Double val) {
  if(!floatrng_) return QString::number(round(val));
  if(fabs(val/dIncr_) < 1e-4) return "0";
  return QString::number(val);  }

  
Double QtSliderBase::toNumber(QString text, bool* ok) {
  if(floatrng_) { 
    text.toFloat(ok);		// (Validate for Float range,
    return text.toDouble();  }	// but retain Double accuracy).
  return Double(text.toInt(ok));  }

  
void QtSliderBase::adjSlIncr() {
  // Adjusts integer slider range (slMax_) and the increment each
  // unit on the slider represents (dIncr_), according to latest
  // increment and range request (dIncr0_,  dMin_, dMax_).
  // Preconditions:  dMax_>=dMin_,  dIncr0_>0.  
  //                 dIncr0_ is a whole number if !floatrng_
  
  dIncr_ = dIncr0_;
  while((slMax_ = ceil((dMax_-dMin_)/dIncr_)) > 1.e5) dIncr_*=10.;  
	// Assures no more than 100000 steps on the slider.

  if( (dMax_-dMin_) >= (floatrng_? 1.e-25 : 100.) ) {
     while((slMax_ = ceil((dMax_-dMin_)/dIncr_)) < 10.) dIncr_/=10.;  }  }
	// Assures at least 10 steps in some cases (but not, e.g.,
	// when single valid value or small integer values are needed).






void QtSliderBase::updateSlider() {
  // Sets slider to latest range and value (without
  // triggering slider signals or slChg slot below). 
  Bool restore = slider_->blockSignals(True);	// Prevent signal recursion.
   slider_->setMinimum(0);			// (Always at this value).
   slider_->setMaximum(Int(slMax_));
   slider_->setSliderPosition(sliderVal(dVal_));
  slider_->blockSignals(restore);  }	// signals (presumably) back on.
    
  
void QtSliderBase::emitVal() {
  // emit current value -- this is the widget's main output.
  emit itemValueChanged(itemName, textVal(),    QtAutoGui::Set, True);  }

 
void QtSliderBase::update(Double dval) {
  // Sets the new value (which should already be validated), 
  // updates user interface accordingly, (without retriggering any
  // internal slots), and emits the new value.
    dVal_ = dval;		// Accept new value.
    updateText();		// 'Normalize' text version on ui.
    updateSlider();		// update slider.
}


void QtSliderBase::setOriginal() {
  // Triggered when 'revert-to-original' is selected from 'wrench' menu.
  
  // In this version, slider limits cannot be changed via 'Original'.
  Double dval = min(dMax_, max(dMin_, origVal_));
  
  if(dval!=dVal_) {
      update(dval);
      emitVal( );
  }
}
  
  
  
void QtSliderBase::getAttr(const QDomElement& ele,
			   QString attnm, Double& val) {
  // Fetch numeric value of attribute of ele named attnm, into val. 
  // Does nothing if attr doesn't exist or is not valid numerically.
  Bool ok;  Double d;
  QString strval = ele.attribute(attnm);
  if(strval.isNull()) return; 
  d = toNumber(strval, &ok);
  
  if(ok) val = d;  }
  

  
// ///////////////////// QtSliderEditor ///////////////////////////////////
  
QtSliderEditor::QtSliderEditor(QDomElement& ele, QWidget *parent) 
	      : QtSliderBase(parent) {
  setupUi(this);		// Creates ui widgets such as slider.
  constructBase(ele, slider, nameLabel, 0);	// sets up base class.
  radioButton->hide();	//#dk
  connect(lineEdit, SIGNAL(editingFinished()),  SLOT(edited()) );  }


void QtSliderEditor::textChg(QString strval)  {
  
  // Validate and normalize strval, update interface accordingly,
  // emit value change signal, as necessary.
  
  Bool ok;
  Double dval = toNumber(strval, &ok);

  if(!ok) {
    updateText();	// Resets text box to last valid value.
    emit errMsg("Invalid numeric input -- please re-enter.");
    return;  }
  
  if(dval==dVal_) { 
    updateText();	// No value change: renormalize text, but no
    return;  }		// need for signals or other adjustments.

  if     (dval<dMin_) { dMin_=dval; adjSlIncr();  }
  else if(dval>dMax_) { dMax_=dval; adjSlIncr();  }
	// Re-adjust integer slider range according to
	// new external range, if it has expanded.
  
  // Accept new value.  Update interface accordingly,
  // and emit the new value.
  
  update(dval);
  emitVal( );
}



// /////////////////////// QtSliderLabel ///////////////////////////////////

QtSliderLabel::QtSliderLabel(QDomElement& ele, QWidget *parent) 
	     : QtSliderBase(parent) {
  setupUi(this);		    // Creates ui widgets such as slider.
  radioButton->hide();	//#dk
  constructBase(ele, slider, nameLabel, 0);  }  // sets up base class.

  
void QtSliderLabel::updateText() {
  // Sets value label to (normalized) current value.
  // indents to keep numeric label over slider button.
  
  Int indent = 0,  width = posLabel->width();
  if(dMax_>dMin_) {
    indent = round( width * (dVal_-dMin_)/(dMax_-dMin_) );  }
  
  posLabel->setText(textVal());
  
  if(2*indent <= width) {      
    posLabel->setAlignment(Qt::AlignLeft);
    posLabel->setIndent(indent);  }   
  else {
    posLabel->setAlignment(Qt::AlignRight);      
    posLabel->setIndent((width - indent));  }
      
  posLabel->repaint();  }	// (for less delay in following slider).


  

//////////////////// QtMinMaxEditor ////////////////////////////////////////

QtMinMaxEditor::QtMinMaxEditor(QWidget *parent) :
        QWidget(parent), blockSignal(false), orig("[0,1]")
{
    setupUi(this);
    radioButton->hide();	//#dk
}

QtMinMaxEditor::QtMinMaxEditor(QDomElement &ele, QWidget *parent)
              : QWidget(parent)
{

    setupUi(this);
    radioButton->hide();	//#dk
    itemName = ele.tagName();
    connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(display2()));
    //connect(lineEdit, SIGNAL(textChanged(QString)),
    //        this, SLOT(display2(QString)) );
    connect(hist, SIGNAL(clicked()), this, SLOT(setHistogram()) );
//  connect(tool, SIGNAL(clicked()), tool, SLOT(showMenu()) );
    
    hist->hide();	//#dk -- until this button does something...


    //#dk double d2 = ele.attribute("pmax").toDouble();
    //#dk double d1 = ele.attribute("pmin").toDouble();


    lineEdit->setText(ele.attribute("value"));
    nameLabel->setText(ele.attribute("listname"));
    nameLabel->setToolTip(ele.attribute("help"));
    
    orig = lineEdit->text();	// Save original value (for setOriginal()).
}

void QtMinMaxEditor::reSet(QString value)
{
    lineEdit->setText(value);	//#dk needs more testing (9/12/08)
    validate(value);		//#dk needs more testing (unnec. feedback?)
    repaint();
}

void QtMinMaxEditor::display2(int v1)
{  }

void QtMinMaxEditor::display2()
{
    QString value = lineEdit->text();
    display2(value);
}

void QtMinMaxEditor::display2(QString value)
{
    if (!validate(value))
        return;
}

bool QtMinMaxEditor::validate(QString value)
{

    bool ok1, ok2;
    QString str = value.replace(QString("["), "");
    str.replace(QString("]"), QString(""));
    QStringList list = str.split(",");
    if (list.size() != 2)
    {
//      labelOk->setPixmap(QPixmap(QString::fromUtf8(":/icons/cross.png")));
        return false;
    }
    double d1 = list[0].toDouble(&ok1) ;
    double d2 = list[1].toDouble(&ok2);

    if (ok1 == true && ok2 == true && d1 < d2)
    {
        lineEdit->setText("[" + QString::number(d1) + ", "
                          + QString::number(d2) + "]");
        emit  itemValueChanged(itemName, lineEdit->text(),
                               QtAutoGui::Set, radioButton->isChecked());
//      labelOk->setPixmap(QPixmap(":/icons/tick.png"));
        return true;
    }
    else
//      labelOk->setPixmap(QPixmap(QString::fromUtf8(":/icons/cross.png")));
    return false;
}


void QtMinMaxEditor::setOriginal() {
    lineEdit->setText(orig);
    validate(orig);  }

    
void QtMinMaxEditor::setDefault()
{
    emit  itemValueChanged(itemName, lineEdit->text(), QtAutoGui::Default,
                           radioButton->isChecked());
}

void QtMinMaxEditor::setCopy()
{
    //#dk clipBoard =  lineEdit->text();
}

void QtMinMaxEditor::setPaste()
{
    //#dk if(validate(clipBoard)) display2(clipBoard);
}

void QtMinMaxEditor::setHistogram()
{
    emit  itemValueChanged(itemName, "Show histogram plot", QtAutoGui::Command,
                           radioButton->isChecked());
}

QtMinMaxEditor::~QtMinMaxEditor()
{  }


// /////////////////////// QtLineEditor /////////////////////////////////////

QtLineEditor::QtLineEditor(QWidget *parent) :
        QWidget(parent), blockSignal(false)
{
    setupUi(this);
    radioButton->hide();	//#dk
}

QtLineEditor::QtLineEditor(QDomElement &ele, QWidget *parent)
            : QWidget(parent)
{
    setupUi(this);
    radioButton->hide();	//#dk
    itemName = ele.tagName();
    //connect(lineEdit, SIGNAL(textChanged(QString)),
    //                this,SLOT(display2(QString)));
    connect(lineEdit, SIGNAL(editingFinished()),
            this, SLOT(editingFinished()) );
//  connect(tool, SIGNAL(clicked()), tool, SLOT(showMenu()) );
    ptype = ele.attribute("ptype");
    nameLabel->setText(ele.attribute("listname"));
    nameLabel->setToolTip(ele.attribute("help"));

    reSet(ele.attribute("value"));
    
    origValue = lineEdit->text();
}


void QtLineEditor::reSet(QString value) {
  // Set Widget appearance in accordance with passed value.
  // Normalizes appearance somewhat in the case of arrays;
  // does not trigger significant outgoing signals.
  // Called from Autogui::ChangeOptions(), and internally.
  validate(value);  }

  
void QtLineEditor::editingFinished() { validateAndEmit(lineEdit->text());  }


void QtLineEditor::validateAndEmit(QString value) {
  // Validates value and sets onto Widget; emits only if value is valid.
  if(validate(value)) emit itemValueChanged(itemName, lineEdit->text(),
                             QtAutoGui::Set, radioButton->isChecked());  }


     
Bool QtLineEditor::validate(QString value) {
  // Sets checkmark/cross indicator according to validity, normalizes
  // text somewhat, and sets it onto the lineEdit (even if invalid).
  // Does not emit itemValueChanged (even if value is valid) -- caller
  // will do that if necessary.
    
  Bool ok = True;
  
  
  if(ptype == "array") {		// numeric vector.
      
    QRegExp rx("[,\\s\\[\\]]");
    QStringList vals = value.split(rx, QString::SkipEmptyParts);
			// separate on commas, whitespace or brackets.
    lineEdit->setText("[" + vals.join(", ") + "]");
			// normalize value separators, e.g. "[1, 2]".
    for(Int i=0; i<vals.size(); i++) {
      vals.at(i).toDouble(&ok);
			// Check that each value is numeric.
      if(!ok) break;  }  }
      
 
  
  else if(ptype == "scalar") {		// single (Float) value.

    Float fVal = value.trimmed().toFloat(&ok);
				// Check that value converts to Float.
    if(ok) lineEdit->setText(QString::number(fVal));
    else   lineEdit->setText(value.trimmed());  }
				// Normalize text.
  
    

  else lineEdit->setText(value);	// "string" -- any text is ok as is.
    
  
      
// labelOk->setPixmap(QPixmap(ok?  ":/icons/tick.png" : 
//                                 ":/icons/cross.png" ));
  return ok;  }

    

void QtLineEditor::setOriginal() { validateAndEmit(origValue);  }
    
// (unused at present)
void QtLineEditor::setDefault() { setOriginal();  }
void QtLineEditor::setCopy() {  }  //#dk clipBoard =  lineEdit->text();
void QtLineEditor::setPaste() {  } //#dk validateAndEmit(clipBoard);


QtLineEditor::~QtLineEditor() {  }




/////////////////////// QtCombo ////////////////////////////////////////////

QtCombo::QtCombo(QWidget *parent)
        : QWidget(parent), blockSignal(false)
{
    setupUi(this);
    orig = combo->currentText();
    radioButton->hide();	//#dk
}

QtCombo::QtCombo(QDomElement &ele, QWidget *parent) :
        QWidget(parent)
{
    setupUi(this);
    radioButton->hide();	//#dk
    itemName = ele.tagName();
    connect(combo, SIGNAL(activated(int)),  SLOT(emitValue()) );
//  connect(tool, SIGNAL(clicked()), tool,  SLOT(showMenu()) );
    QString popt = ele.attribute("popt");
    
    if(popt.startsWith("[")) popt.remove(0, 1);
    if(popt.endsWith  ("]")) popt.chop(1);
    QStringList poptList = popt.split(", ", QString::SkipEmptyParts);
    //#dk QStringList poptList = popt.remove('[').remove(']').split(",");
    
    QString current = ele.attribute("value", "*None#");
    if(current=="*None#") current = ele.attribute("default", "None");
    
    /* //#dk
    if (!ele.attribute("dependency_list").isNull())
    {
        radioButton->setChecked(false);
        radioButton->setCheckable(false);
        radioButton->hide();
    }
    */ //#dk

    for (int i = 0; i < poptList.size(); i++)
    {
        QString optItem = poptList.at(i);
        combo->addItem(optItem);
        if (current == optItem)
            combo->setCurrentIndex(i);
    }
    
    orig = combo->currentText();  // Save original value (for setOriginal()).
    
    nameLabel->setText(ele.attribute("listname"));
    nameLabel->setToolTip(ele.attribute("help"));
}


void QtCombo::reSet(QString value)
{
    for (int i = 0; i < combo->count(); i++)
    {
        if (combo->itemText(i) == value)
        {
            combo->setCurrentIndex(i);
            break;
        }
    }
    repaint();
}


void QtCombo::emitValue() {
    emit itemValueChanged(itemName, combo->currentText(), QtAutoGui::Set,
                          radioButton->isChecked());  }


void QtCombo::setOriginal() {
    QString old = combo->currentText();
    reSet(orig);
    if(combo->currentText() != old) emitValue();  }
    
void QtCombo::setDefault() { setOriginal();  }

void QtCombo::setCopy()  {  }   //#dk clipBoard =  combo->currentText();
void QtCombo::setPaste() {  }

QtCombo::~QtCombo() {  }


////////////////////////////// QtBoolean ///////////////////////////////////

QtBool::QtBool(QWidget *parent)
        : QWidget(parent), blockSignal(false)
{
    setupUi(this);
    orig = combo->currentText();
    radioButton->hide();	//#dk
}

QtBool::QtBool(QDomElement &ele, QWidget *parent) :
        QWidget(parent)
{
    setupUi(this);
    radioButton->hide();	//#dk
    itemName = ele.tagName();
    connect(combo, SIGNAL(activated(int)),  SLOT(emitValue()) );
//  connect(tool, SIGNAL(clicked()),  tool, SLOT(showMenu()) );
    combo->addItem("false");
    combo->addItem("true");
    combo->setCurrentIndex(1);
    if (ele.attribute("default") == "0")
    {
        combo->setCurrentIndex(0);
    }
    
    orig = combo->currentText();  // Save original value (for setOriginal()).
    
    nameLabel->setText(ele.attribute("listname"));
    nameLabel->setToolTip(ele.attribute("help"));
}

void QtBool::reSet(QString value)
{
    repaint();
}


void QtBool::emitValue() {
    QString val = (combo->currentText() == "false") ? "0" : "1";
    emit  itemValueChanged(itemName, val, QtAutoGui::Set,
                           radioButton->isChecked());  }

void QtBool::setOriginal() {
    QString old = combo->currentText();
    combo->setCurrentIndex((orig=="false")? 0:1);
    if(combo->currentText() != old) emitValue();  }
    
void QtBool::setDefault() { setOriginal();  }


void QtBool::setCopy()  {  }  //#dk clipBoard = combo->currentText();
void QtBool::setPaste() {  }

QtBool::~QtBool()
{}


// //////////////////////////// QtCheck /////////////////////////////////////

QtCheck::QtCheck(QWidget *parent) : QWidget(parent) {
    setupUi(this);
    radioButton->hide();  }	//#dk

QtCheck::QtCheck(QDomElement &ele, QWidget *parent) :  QWidget(parent) {
  setupUi(this);
  radioButton->hide();	//#dk
  itemName = ele.tagName();
//connect(tool, SIGNAL(clicked()), tool, SLOT(showMenu()) );
    
  QString popt = ele.attribute("popt");
  if(popt.startsWith("[")) popt.remove(0,1);
  if(popt.endsWith  ("]")) popt.chop(1);
  allopts_ = popt.split(", ", QString::SkipEmptyParts);
    
  for (int i = 0; i < allopts_.size(); i++) {
    QString opt = allopts_.at(i);
    QCheckBox* check = new QCheckBox(this);
    checkboxes_ << check;
    check->setObjectName(opt);
    check->setMinimumSize(QSize(100, 0));
    check->setText(opt);
    gridLayout->addWidget(check, i / 2, i %2, 1, 1);
    connect(check, SIGNAL(stateChanged(int)), SLOT(checkboxChanged(int)) );  }
    
  reSet(ele.attribute("value"));
    
  orig_ = opts_;
    
  nameLabel->setText(ele.attribute("listname"));
  nameLabel->setToolTip(ele.attribute("help"));
}


void QtCheck::checkboxChanged(int newState) {
  QString opt = dynamic_cast<QCheckBox*>(sender())->text();
    
  if(newState == Qt::Unchecked) opts_.removeAll(opt);
  else if(!opts_.contains(opt))  opts_ << opt;
    
  emitValue();  }

    
void QtCheck::emitValue() {
  emit itemValueChanged(itemName,  "[" + opts_.join(", ") + "]",
                        QtAutoGui::Set,  radioButton->isChecked());  }


void QtCheck::setOriginal() { reSet(orig_); emitValue();  }
void QtCheck::setDefault()  { setOriginal();  }



void QtCheck::reSet(QString value) {
  if(value.startsWith("[")) value.remove(0,1);
  if(value.endsWith  ("]")) value.chop(1);
  reSet( value.split(", ", QString::SkipEmptyParts) );  }
    

  
void QtCheck::reSet(QStringList options) {
  // [Re]sets state of checkboxes and internal record of that (opts_),
  // without emitting itemValueChanged.
  
  opts_ = options;	// names of boxes to be checked.
  reSet();  }

  
  
void QtCheck::reSet() {
  // [Re]sets state of checkboxes to internal record of that (opts_),
  // without emitting itemValueChanged.
    
  for (int i=0; i<allopts_.size(); i++) {
    QString    opt   = allopts_.at(i);
    QCheckBox* check = checkboxes_.at(i);
        
    Bool restore = check->blockSignals(True);  // Prevent signal recursion.
    check->setCheckState( opts_.contains(opt)? Qt::Checked : Qt::Unchecked );
    check->blockSignals(restore);  }	// signals (presumably) back on.
 
  repaint();  }


  
void QtCheck::setCopy() {  }
void QtCheck::setPaste() {  }

QtCheck::~QtCheck() {  }


//////////////////////// QtPushButton //////////////////////////////////////

QtPushButton::QtPushButton(QWidget *parent) :
        QWidget(parent), blockSignal(false)
{
    setupUi(this);
}

QtPushButton::QtPushButton(QDomElement &ele, QWidget *parent)
            : QWidget(parent)
{
    setupUi(this);
    itemName = ele.tagName();
    connect(pushButton, SIGNAL(clicked()), this, SLOT(emitValue()) );
    QString txt = ele.attribute("text");
    pushButton->setText(txt);
    nameLabel->setText(ele.attribute("listname"));
    nameLabel->setToolTip(ele.attribute("help"));
}

void QtPushButton::reSet(QString value)
{
    repaint();
}

void QtPushButton::emitValue()
{
    emit  itemValueChanged(itemName, "1", /*pushButton->text(),*/
                           QtAutoGui::Command, true);
}

QtPushButton::~QtPushButton()
{}


////////////////////////////// QtAdjustmentTop /////////////////////////////

QtAdjustmentTop::QtAdjustmentTop(QtAutoGui *parent, QString name)
        :/* QWidget(parent),*/ parent(parent), blockSignal(false)
{
    setupUi(this);
    applyButton->setToolTip("Apply the whole set of display options");
    connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()) );
    saveButton->setToolTip("Save the display options to a file");
    connect(saveButton, SIGNAL(clicked()), this, SLOT(save()) );
    restoreButton->setToolTip("Load the display options from a file");
    connect(restoreButton, SIGNAL(clicked()), this, SLOT(load()) );
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(close()) );
//  connect(tool, SIGNAL(clicked()), tool, SLOT(showMenu()) );
    connect(dataName, SIGNAL(textChanged(QString)),
            this, SLOT(dataNameChanged(QString)) );
    dataName->setText(name);
    dataName->setToolTip("enter the file name of the display options");
    
    //#dk save-restore is now on the overall displaypanel level; no need to
    // expose such interface on the level of each options panel any longer.
    saveButton->hide();
    restoreButton->hide();
    dataName->hide();
    
}

void QtAdjustmentTop::dataNameChanged(QString value)
{
    dataName->setText(value);
    if (value.size() > 0)
        parent->setFileName(value);
}

void QtAdjustmentTop::setOriginal()
{
    //if you just want to set fileName to original
    dataName->setText(parent->fileName());
    //or if you want to set the whole thing to original
    parent->setOriginal();
}

void QtAdjustmentTop::setDefault()
{
    dataName->setText(parent->fileName());
    parent->setDefault();
}
void QtAdjustmentTop::setMemory()
{
    dataName->setText(parent->fileName());
    parent->setMemory();
}

void QtAdjustmentTop::setCopy()  {  }  //#dk  clipBoard = dataName->text();
void QtAdjustmentTop::setPaste() {  }  //#dk  dataName->setText(clipBoard);

void QtAdjustmentTop::setClear()
{
    dataName->setText("");
    //#dk  clipBoard = "";
}

void QtAdjustmentTop::apply()
{
    parent->apply();
}

void QtAdjustmentTop::save()
{
    parent->save();
}

void QtAdjustmentTop::load()
{
    parent->load();
}

void QtAdjustmentTop::close()
{
    parent->dismiss();
}

void QtAdjustmentTop::restore()
{
    parent->restore();
}

void QtAdjustmentTop::hideDismiss()
{
    dismissButton->hide();
}		//#dk

QtAdjustmentTop::~QtAdjustmentTop()
{}


///////////////////////// QtRegionEditor////////////////////////////////////

QtRegionEditor::QtRegionEditor(QWidget *parent) :
        QWidget(parent), blockSignal(false)
{
    setupUi(this);
    radioButton->hide();	//#dk
}

QtRegionEditor::QtRegionEditor(QDomElement &ele, QWidget *parent)
              : QWidget(parent)
{
    setupUi(this);
    radioButton->hide();	//#dk
    itemName = ele.tagName();
    //connect(lineEdit, SIGNAL(textChanged(QString)),
    //                this,SLOT(display2(QString)));
    connect(lineEdit, SIGNAL(editingFinished()),
            this, SLOT(editingFinished()) );
//  connect(tool, SIGNAL(clicked()), tool, SLOT(showMenu()) );
    QString value = ele.attribute("value");

    if (value.isNull() && ele.attribute("datatype") == "Bool")
    {
        QDomElement el1 = ele.firstChildElement();
        QString v;
        if (!el1.isNull())
        {
            v = el1.attribute("i_am_unset");
            if (!v.isNull())
            {
                value = "<unset>";
            }
        }
    }
    ele.setAttribute("i_am_unset", "i_am_unset");
    lineEdit->setText(value);
    iamunset = "I_am_unset";
    ptype = ele.attribute("ptype");
    nameLabel->setText(ele.attribute("listname"));
    nameLabel->setToolTip(ele.attribute("help"));
}

void QtRegionEditor::reSet(QString value)
{
    repaint();
}

void QtRegionEditor::display2(QString value)
{
    if (!validate(value))
        return;

    emit  itemValueChanged(itemName, lineEdit->text(), QtAutoGui::Set,
                           radioButton->isChecked());
}

void QtRegionEditor::editingFinished()
{
    if (!validate(lineEdit->text()))
        return;

    emit  itemValueChanged(itemName, lineEdit->text(), QtAutoGui::Set,
                           radioButton->isChecked());
}

bool QtRegionEditor::validate(QString value)
{
    if (value.isNull())
    {
//      labelOk->setPixmap(QPixmap(QString::fromUtf8(":/icons/cross.png")));
        return false;
    }
    if (ptype == "region")
    {
        if (value.contains(QRegExp("\\[.+\\]")) == 0)
        {
//          labelOk->setPixmap(QPixmap(QString::fromUtf8(":/icons/cross.png")));
            return false;
        }
        QStringList poptList = value.remove('[').remove(']').split(",");
        bool ok = true;
//      labelOk->setPixmap(QPixmap(":/icons/tick.png"));
        for (int i = 0; i < poptList.size(); i++)
        {
            QString opt = poptList.at(i).simplified();
            ok = false;
            if (ok == false)
            {
//              labelOk->setPixmap(QPixmap(QString::fromUtf8(
//                                             ":/icons/cross.png")));
                return false;
            }
        }
        return ok;
    }
    return true;
}

void QtRegionEditor::setOriginal()
{
    emit  itemValueChanged(itemName, lineEdit->text(), QtAutoGui::Original,
                           radioButton->isChecked());
}

void QtRegionEditor::setDefault()
{
    emit  itemValueChanged(itemName, lineEdit->text(), QtAutoGui::Default,
                           radioButton->isChecked());
}

void QtRegionEditor::fromImg()
{
    emit  itemValueChanged(itemName, lineEdit->text(), QtAutoGui::Command,
                           radioButton->isChecked());
}

void QtRegionEditor::createRegion()
{
    emit  itemValueChanged(itemName, lineEdit->text(), QtAutoGui::Command,
                           radioButton->isChecked());
}

void QtRegionEditor::unset()
{
    lineEdit->setText("<unset>");
//  labelOk->setPixmap(QPixmap(":/icons/tick.png"));
}

void QtRegionEditor::setCopy()  {  }  //#dk  clipBoard =  lineEdit->text();
void QtRegionEditor::setPaste() {  }
			//#dk  if(validate(clipBoard)) display2(clipBoard);

QtRegionEditor::~QtRegionEditor() {  }



///////////////////////// QtPairEditor ////////////////////////////////////

// Public Methods //

QtPairEditor::QtPairEditor(QDomElement& ele, QWidget* parent) :
        QWidget(parent) {
    setupUi(this);
    radioButton->hide();
    itemName = ele.tagName();
    
    QHBoxLayout* l = new QHBoxLayout(frame);
#if QT_VERSION >= 0x040300
    l->setContentsMargins(0, 0, 0, 0);
#else
    l->setMargin(0);
#endif
    l->setSpacing(3);
    
    QString separator = ",";
    if(ele.hasAttribute("separator")) separator = ele.attribute("separator");
    
    // find out if double or int
    isDouble = ele.attribute("ptype") != "intpair";
    if(ele.attribute("ptype") == "intpair") {
        QSpinBox* v1 = new QSpinBox(frame);
        QSpinBox* v2 = new QSpinBox(frame);
        val1 = v1; val2 = v2;
        
        if(ele.hasAttribute("pmin")) {
            if(ele.attribute("pmin").contains(' ')) {
                std::pair<int, int> i = readIntPair(ele.attribute("pmin"));
                v1->setMinimum(i.first);
                v2->setMinimum(i.second);
            } else {
                int i = ele.attribute("pmin").toInt();
                v1->setMinimum(i);
                v2->setMinimum(i);
            }
        } else {
            v1->setMinimum(std::numeric_limits<int>::min());
            v2->setMinimum(std::numeric_limits<int>::min());
        }
        
        if(ele.hasAttribute("pmax")) {
            if(ele.attribute("pmax").contains(' ')) {
                std::pair<int, int> i = readIntPair(ele.attribute("pmax"));
                v1->setMaximum(i.first);
                v2->setMaximum(i.second);
            } else {
                int i = ele.attribute("pmax").toInt();
                v1->setMaximum(i);
                v2->setMaximum(i);
            }
        } else {
            v1->setMaximum(std::numeric_limits<int>::max());
            v2->setMaximum(std::numeric_limits<int>::max());
        }
        
        reSet(ele.attribute("value"));
        origVal1.first = v1->value();
        origVal1.second = v2->value();
    } else {
        QDoubleSpinBox* v1 = new QDoubleSpinBox(frame);
        QDoubleSpinBox* v2 = new QDoubleSpinBox(frame);
        val1 = v1; val2 = v2;
        
        if(ele.hasAttribute("pmin")) {
            if(ele.attribute("pmin").contains(' ')) {
                std::pair<double, double> d = readDoublePair(
                                              ele.attribute("pmin"));
                v1->setMinimum(d.first);
                v2->setMinimum(d.second);
            } else {
                double d = ele.attribute("pmin").toDouble();
                v1->setMinimum(d);
                v2->setMinimum(d);
            }
        } else {
            v1->setMinimum(std::numeric_limits<double>::min());
            v2->setMinimum(std::numeric_limits<double>::min());
        }
        
        if(ele.hasAttribute("pmax")) {
            if(ele.attribute("pmax").contains(' ')) {
                std::pair<double, double> d = readDoublePair(
                                              ele.attribute("pmax"));
                v1->setMaximum(d.first);
                v2->setMaximum(d.second);
            } else {
                double d = ele.attribute("pmax").toDouble();
                v1->setMaximum(d);
                v2->setMaximum(d);
            }
        } else {
            v1->setMaximum(std::numeric_limits<double>::max());
            v2->setMaximum(std::numeric_limits<double>::max());
        }
        
        reSet(ele.attribute("value"));
        origVal2.first = v1->value();
        origVal2.second = v2->value();
    }
    l->addWidget(val1);
    l->addWidget(new QLabel(separator));
    l->addWidget(val2);
    l->addStretch();
    connect(val1, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(val2, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    
    nameLabel->setText(ele.attribute("listname"));
    nameLabel->setToolTip(ele.attribute("help"));
    
}

QtPairEditor::~QtPairEditor() { }

void QtPairEditor::reSet(QString value) {
    if(value.isEmpty()) return;
    
    if(isDouble) {
        std::pair<double, double> p;
        if(value.contains(' ')) {
            p = readDoublePair(value);
        } else {
            p.first = value.toDouble();
            p.second = value.toDouble();
        }
        dynamic_cast<QDoubleSpinBox*>(val1)->setValue(p.first);
        dynamic_cast<QDoubleSpinBox*>(val2)->setValue(p.second);
        
    } else {
        std::pair<int, int> p;
        if(value.contains(' ')) {
            p = readIntPair(value);
        } else {
            p.first = value.toInt();
            p.second = value.toInt();
        }
        dynamic_cast<QSpinBox*>(val1)->setValue(p.first);
        dynamic_cast<QSpinBox*>(val2)->setValue(p.second);
    }
}


// Protected Methods //

void QtPairEditor::editingFinished() {
    stringstream ss;
    
    if(isDouble) {
        ss << dynamic_cast<QDoubleSpinBox*>(val1)->value() << ' ';
        ss << dynamic_cast<QDoubleSpinBox*>(val2)->value();
    } else {
        ss << dynamic_cast<QSpinBox*>(val1)->value() << ' ';
        ss << dynamic_cast<QSpinBox*>(val2)->value();
    }
    
    emit itemValueChanged(itemName, ss.str().c_str(),
                          QtAutoGui::Set, radioButton->isChecked());
}

void QtPairEditor::setOriginal() {
    if(isDouble) {
        dynamic_cast<QDoubleSpinBox*>(val1)->setValue(origVal2.first);
        dynamic_cast<QDoubleSpinBox*>(val2)->setValue(origVal2.second);
    } else {
        dynamic_cast<QSpinBox*>(val1)->setValue(origVal1.first);
        dynamic_cast<QSpinBox*>(val2)->setValue(origVal1.second);
    }
}


// Private Methods //

std::pair<int, int> QtPairEditor::readIntPair(QString value) {
    QTextStream ss(&value);
    std::pair<int, int> p;
    ss >> p.first;
    ss >> p.second;
    return p;
}

std::pair<double, double> QtPairEditor::readDoublePair(QString value) {
    QTextStream ss(&value);
    std::pair<double, double> p;
    ss >> p.first;
    ss >> p.second;
    return p;
}



///////////////////////// QtNumberEditor ////////////////////////////////////

// Public Methods //

QtNumberEditor::QtNumberEditor(QDomElement& ele, QWidget* parent) :
        QWidget(parent) {
    setupUi(this);
    radioButton->hide();
    itemName = ele.tagName();
    
    QHBoxLayout* l = new QHBoxLayout(frame);
#if QT_VERSION >= 0x040300
    l->setContentsMargins(0, 0, 0, 0);
#else
    l->setMargin(0);
#endif
    l->setSpacing(3);
    
    // find out if double or int
    isDouble = ele.attribute("ptype") != "int";
    if(!isDouble) {
        QSpinBox* v = new QSpinBox(frame);
        val = v;
        
        if(ele.hasAttribute("pmin"))
            v->setMinimum(ele.attribute("pmin").toInt());
        else
            v->setMinimum(std::numeric_limits<int>::min());
        
        if(ele.hasAttribute("pmax"))
                v->setMaximum(ele.attribute("pmax").toInt());
        else
            v->setMaximum(std::numeric_limits<int>::max());
        
        reSet(ele.attribute("value"));
        origVal1 = v->value();
    } else {
        QDoubleSpinBox* v = new QDoubleSpinBox(frame);
        val = v;
        
        if(ele.hasAttribute("pmin"))
            v->setMinimum(ele.attribute("pmin").toDouble());
        else
            v->setMinimum(std::numeric_limits<double>::min());
        
        if(ele.hasAttribute("pmax"))
            v->setMaximum(ele.attribute("pmax").toDouble());
        else
            v->setMaximum(std::numeric_limits<double>::max());
        
        reSet(ele.attribute("value"));
        origVal2 = v->value();
    }
    l->addWidget(val);
    l->addStretch();
    connect(val, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    
    nameLabel->setText(ele.attribute("listname"));
    nameLabel->setToolTip(ele.attribute("help"));
    
}

QtNumberEditor::~QtNumberEditor() { }

void QtNumberEditor::reSet(QString value) {
    if(value.isEmpty()) return;
    
    if(isDouble) 
        dynamic_cast<QDoubleSpinBox*>(val)->setValue(value.toDouble());
        
    else
        dynamic_cast<QSpinBox*>(val)->setValue(value.toInt());
}


// Protected Methods //

void QtNumberEditor::editingFinished() {
    stringstream ss;
    
    if(isDouble)
        ss << dynamic_cast<QDoubleSpinBox*>(val)->value();
    else
        ss << dynamic_cast<QSpinBox*>(val)->value();
    
    emit itemValueChanged(itemName, ss.str().c_str(),
                          QtAutoGui::Set, radioButton->isChecked());
}

void QtNumberEditor::setOriginal() {
    if(isDouble)
        dynamic_cast<QDoubleSpinBox*>(val)->setValue(origVal2);
    else
        dynamic_cast<QSpinBox*>(val)->setValue(origVal1);
}

} //# NAMESPACE CASA - END
