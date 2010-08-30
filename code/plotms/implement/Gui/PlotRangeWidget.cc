//# PlotRangeWidget.cc: Widget for plot axes ranges.
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
#include <plotms/Gui/PlotRangeWidget.qo.h>

#include <plotms/PlotMS/PlotMSConstants.h>

#include <limits>
#include <QDoubleValidator>

namespace casa {

/////////////////////////////////
// PLOTRANGEWIDGET DEFINITIONS //
/////////////////////////////////

PlotRangeWidget::PlotRangeWidget(bool customOnTwoLines, QWidget* parent) :
		QtEditingWidget(parent) {
    setupUi(this);
    
    if(customOnTwoLines) {
    	// double page
    	QVBoxLayout* vl = new QVBoxLayout();
    	vl->setContentsMargins(0, 0, 0, 0);
    	vl->setSpacing(3);
    	
    	QHBoxLayout* hl = new QHBoxLayout();
    	hl->setContentsMargins(0, 0, 0, 0);
    	hl->setSpacing(3);
    	hl->addWidget(doubleFrom);
    	hl->addWidget(doubleLabel);
    	vl->addLayout(hl);
    	vl->addWidget(doubleTo);
    	delete doublePage->layout();
    	doublePage->setLayout(vl);
    	
    	// time page
    	vl = new QVBoxLayout();
    	vl->setContentsMargins(0, 0, 0, 0);
    	vl->setSpacing(3);
    	
    	hl = new QHBoxLayout();
    	hl->setContentsMargins(0, 0, 0, 0);
    	hl->setSpacing(3);
    	hl->addWidget(timeFrom);
    	hl->addWidget(timeLabel);
    	vl->addLayout(hl);
    	vl->addWidget(timeTo);
    	delete timePage->layout();
    	timePage->setLayout(vl);
    }
    
    doubleFrom->setValidator(new QDoubleValidator(doubleFrom));
    doubleTo->setValidator(new QDoubleValidator(doubleTo));
    
    setRange(false, false, 0, 0);
    
    connect(automatic, SIGNAL(toggled(bool)), SLOT(rangeChanged()));
    connect(doubleFrom, SIGNAL(textChanged(const QString&)),
    		SLOT(rangeChanged()));
    connect(doubleTo, SIGNAL(textChanged(const QString&)),
    		SLOT(rangeChanged()));
    connect(timeFrom, SIGNAL(textChanged(const QString&)),
            SLOT(timeFromChanged()));
    connect(timeTo, SIGNAL(textChanged(const QString&)),
            SLOT(timeToChanged()));
}

PlotRangeWidget::~PlotRangeWidget() { }

bool PlotRangeWidget::isDate() const {
    return stackedWidget->currentIndex() == 1; }
void PlotRangeWidget::setIsDate(bool isDate) {
    stackedWidget->setCurrentIndex(isDate ? 1 : 0); }

bool PlotRangeWidget::isCustom() const { return custom->isChecked(); }





/*
	EvaluateDateTimeText() looks at the text from one of the QLineEdits,
	judges the goodness of the syntax, and if it's good, converts it 
	into a double for use outside the range widget.   
	We distinguish different kinds of non-good date-time strings,
	identified by the DateTimeStringQuality enum.
	 
	NOTE: this whole mess probably shouldn't be sitting loosely in 
	this src file, but put somewhere it's available more widely
	and easier to test.
*/
#pragma GCC optimize 0
#pragma GCC option ("-O0")
  
enum DateTimeStringQuality   {
	DATETIME_BadSyntax,      // wrong characters, gibberish
	DATETIME_Incomplete,     // good syntax so far
	DATETIME_ImproperValue,  // silly month  number, >59 minutes, etc.
	DATETIME_GOOD
};


static 
bool scandigits(const char *txt, int ndigits,  uInt *val)  {
	uInt v=0;
	while (ndigits--)   v = 10*v + (int)((*txt++)-'0');
	*val=v;
	return true;
}



static 
DateTimeStringQuality EvaluateDateTimeText(const char *txt,  double &mjulian_sec)
{
	if (!txt || *txt==0) return DATETIME_Incomplete;
	
	
	// Simple check of character categories
	// We expect a fixed format, but
	// the seconds may be omitted or have integer part only.
	// For our application, we want a sensible distinction
	// between a wrong character, such as 'W", vs. merely 
	// an incomplete string being typed in by the user.
	// I couldn't figure out how to do that with a regex
	// though maybe that would be a better approach.
	
	// First step: look for wrong kinds of characters
	// by matching against a template, up to length of the 
	// string so far.
	
	const char *datetime_template1 = "####/##/##&##:##:##.###";
	// const char *datetime_template2 = "???"; // for alternative formats TBD
	int n = strlen(txt);
	for (int i=0; i<n; i++)   {
		char c=txt[i];
		switch (datetime_template1[i])  {
			case '#':  if (isdigit(c)) continue; break;
			case '/':  if (c=='/' || c=='-') continue; break;
			case '&':  if (c==' ' ||c=='/' ||c=='T') continue; break;
			default:  if (c==datetime_template1[i]) continue; 
		}
		return DATETIME_BadSyntax;  
	}
	
	// Do we have at least yyyy/mm/dd/hh:mm?
	if (n<16)
		return DATETIME_Incomplete;

	
	// Okay, we have enough correct digits and punctuation.
	// Now scan the numbers so we can construct a casacore Time() 
	// While we could verify details like 0<=min<=59, we don't
	// want to reinvent the wheels for checking leap years.
	uInt yyyy, mm, dd;
	uInt hour,min;
	double sec;

	scandigits(&txt[0], 4, &yyyy);
	scandigits(&txt[5], 2, &mm);
	scandigits(&txt[8], 2, &dd);
	scandigits(&txt[11], 2, &hour);
	scandigits(&txt[14], 2, &min);
	
	// If seconds were given, read it, otherwise assume zero.
	if (n>=19)
			sec = atof(&txt[17]);
	else
			sec =0.0;
	

	// Check if numbers are in sensible ranges
	if (yyyy>9999)  return DATETIME_ImproperValue;
	if (mm<1 || mm>12) return DATETIME_ImproperValue;
	if (dd>Time::howManyDaysInMonth(mm,yyyy))  return DATETIME_ImproperValue;
	if (hour>23)  return DATETIME_ImproperValue;
	if (min>59)   return DATETIME_ImproperValue;
	if (sec>60.0) return DATETIME_ImproperValue;
	
	try   {
			// Conversion to Mod Julian seconds by using a casacore Date
			// is already coded for our convenience in PlotMSConstants
			mjulian_sec = PMS::dateDouble(yyyy,mm,dd,hour,min,sec, DATE_MJ_SEC);
	}
	catch (AipsError &err)   {
			return DATETIME_ImproperValue;
	}
	return DATETIME_GOOD;	
}



prange_t PlotRangeWidget::getRange() const {
    if(isDate()) {
 		double from,to;
		DateTimeStringQuality fquality, tquality;
		
		fquality=EvaluateDateTimeText(timeFrom->text().toAscii().constData(), from);
		tquality=EvaluateDateTimeText(timeTo->text().toAscii().constData(),   to);
		
		if (fquality!=DATETIME_GOOD || tquality!=DATETIME_GOOD)  
			{ /* so something if string isn't good? 
			     must return a prange_t in any case... */  
			}

			
#if (0) // dead code, keep for ref while building new code.  DELETE THIS SOON
        QDate date = timeFrom->date(); QTime time = timeFrom->time();
        double from = PMS::dateDouble(date.year(), date.month(), date.day(),
                time.hour(), time.minute(),
                time.second() + (time.msec() / 1000.0));
        
        date = timeTo->date(); time = timeTo->time();
        double to = PMS::dateDouble(date.year(), date.month(), date.day(),
                time.hour(), time.minute(),
                time.second() + (time.msec() / 1000.0));
#endif
        return prange_t(from, to);
        
    } else
        return prange_t(doubleFrom->text().toDouble(),
        		        doubleTo->text().toDouble());
}




void PlotRangeWidget::setRange(bool isDate, bool isCustom, double from,
        double to) {
    blockSignals(true);
    
    stackedWidget->setCurrentIndex(isDate ? 1 : 0);
    custom->setChecked(isCustom);
    
    bool changed = isDate != this->isDate() || isCustom != this->isCustom();
    if(!changed) {
        prange_t range = getRange();
        changed = from != range.first || to != range.second;
    }
    
    if(isDate) {
		
        unsigned int year, mon, day, hour, min; double sec;
        char txt[60];
        
        const char *datetime_display_format 
				= "%04d/%02d/%02d/%02d:%02d:%06.3f";

        PMS::dateDouble(from, year, mon, day, hour, min, sec);
        sprintf(txt, datetime_display_format, 
			year, mon, day, hour, min,  sec);
		timeFrom->setText(QString(txt));

        PMS::dateDouble(to, year, mon, day, hour, min, sec);
        sprintf(txt, datetime_display_format, 
			year, mon, day, hour, min,  sec);
		timeTo->setText(QString(txt));
        
        #if (0)  // OLD CODE - keep for ref while writing new code
        PMS::dateDouble(from, year, mon, day, hour, min, sec);
        timeFrom->setDate(QDate(year, mon, day));
        timeFrom->setTime(QTime(hour, min, (int)sec,
                          (int)((modf(sec, &temp) * 1000) + 0.5)));
        
        PMS::dateDouble(to, year, mon, day, hour, min, sec);
        timeTo->setDate(QDate(year, mon, day));
        timeFrom->setTime(QTime(hour, min, (int)sec,
                          (int)((modf(sec, &temp) * 1000) + 0.5)));
        #endif 
        
    } else {
        doubleFrom->setText(QString::number(from));
        doubleTo->setText(QString::number(to));
    }
    
    isCustom_ = isCustom;
    from_ = from;
    to_ = to;
    
    blockSignals(false);
    if(changed) emit this->changed();
}






void PlotRangeWidget::addRadioButtonsToGroup(QButtonGroup* group) const {
    if(group == NULL) return;
    group->addButton(automatic);
    group->addButton(custom);
}

void PlotRangeWidget::rangeChanged() {
    emit changed();
    prange_t r = getRange();
    if(isCustom() != isCustom_ || r.first != from_ || r.second != to_)
        emit differentFromSet();
}





static void ColorizeValidity(QLineEdit*edit)
{
    // With every edit of the text, set color to show if it's a valid datetime
	
	const char *txt = edit->text().toAscii().constData();
	double mjsec;
	DateTimeStringQuality  qual =EvaluateDateTimeText(txt, mjsec);
	// mjsec is not needed.  
	QColor color;
    switch (qual)   {
		case DATETIME_GOOD:   
				color = QColor(0,0,0);        /* black */
				break;
		
		case DATETIME_ImproperValue: 
				color = QColor(180,48,220);   /* violet */  
				break;
				
		case DATETIME_Incomplete:
				// seen during normal typing in of a date/time
				color = QColor(40,98,240);   /* turq-blue */  
				break;
			
		case DATETIME_BadSyntax:
				// when user types in a non-valid char: red for "stop"
				color = QColor(230,0,0);
				break;
		
	}
    
    
    QPalette pal = edit->palette();
    pal.setColor(QPalette::Text, color);
    edit->setPalette(pal);
}



void PlotRangeWidget::timeToChanged() {
	// Change colors depending on validity of date/time
	ColorizeValidity(timeTo);
	// Now do whatever is normally done when ranges are edited
	rangeChanged();
}



void PlotRangeWidget::timeFromChanged() {
	// Change colors depending on validity of date/time
	ColorizeValidity(timeFrom);
	// Now do whatever is normally done when ranges are edited
	rangeChanged();
}



} 
