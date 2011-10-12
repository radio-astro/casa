//# PlotMSTabs.cc: Tab GUI widgets.
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
#include <plotms/GuiTabs/PlotMSTab.qo.h>

#include <plotms/Gui/PlotMSPlotter.qo.h>

namespace casa {


// Color for drawing highlighted text.
// Text is highlighted to indicate a change in some parameter's value,
// that hasn't yet been incorporated into the plot shown.
const int g_HighlightColorRGB[3] = { 255,  0,  0 };
const QColor g_HighlightQColor(
			g_HighlightColorRGB[0],
			g_HighlightColorRGB[1],
			g_HighlightColorRGB[2]
			);



///////////////////////////
// PLOTMSTAB DEFINITIONS //
///////////////////////////


PlotMSTab::PlotMSTab(PlotMSPlotter* parent) : itsParent_(parent->getParent()),
        itsPlotter_(parent) { }

PlotMSTab::~PlotMSTab() { }


 
void PlotMSTab::highlightWidgetText(QLabel* label, bool highlight) {
    label->setText( highlightifyText( label->text(), highlight) );
}


void PlotMSTab::highlightWidgetText(QAbstractButton* but, bool highlight)   {
    but->setText( highlightifyText( but->text(), highlight) );
}


void PlotMSTab::highlightWidgetText(QGroupBox* box, bool highlight)   {
    
    //not valid:  box->setTitle( highlightifyText( box->title(), highlight) );
    
    QPalette pal = box->palette();
    if (highlight)   {
		// which one - ButtonText BrightText Text ??
		pal.setColor(QPalette::WindowText, g_HighlightQColor);
	} else {
		pal.setColor(QPalette::Text, Qt::black);
	}
    box->setPalette(pal);
    // We need to force box to repaint itself using this new colro
    // doesnt' work:  box->setTitle(box->title());
    // doesnt' work:  box->repaint(0,0,-1,-1);
    box->repaint(0,0,-1,-1);   box->update();
}




QString PlotMSTab::highlightifyText(const QString& text, bool highlight) {
	// note (dsw, aug. 2010): older code used to replace spaces with &nbsp;
	// but this was removed for unknown reasons
	
	// Is the string already highlighted?  Presence of HTML closing
	// bracket is assumed to be sign of highlighted by previous
	// use of highlightifyText
	int igt = text.indexOf(QChar('>'));
	bool already = (igt >2);
	if (already==highlight)  {
		return text;
	}
	else
		if (highlight)	 {
			// text lacks tags, so add them
			QString s;
			s.sprintf("<font color=\"#%02X%02X%02X\">%s</font>",
				g_HighlightColorRGB[0],
				g_HighlightColorRGB[1],
				g_HighlightColorRGB[2],
				(const char*)text.toAscii());
			return s;
		}
		else   {
			// text has html to be stripped off
			// find start of </font> 
			int iclose = text.lastIndexOf(QChar('<'));
			
			// Check for insanse results.  If so,
			// don't know what to do, just parrot back the input
			if (iclose <= igt) return text;  
			
			// Return the part between the tags.
			return text.left(iclose).mid(igt+1);
		}
}


bool PlotMSTab::setChooser(QComboBox* chooser, const QString& value) {
    for(int i = 0; i < chooser->count(); i++) {
        if(chooser->itemText(i) == value) {
            chooser->setCurrentIndex(i);
            return true;
        }
    }
    return false;
}

}
