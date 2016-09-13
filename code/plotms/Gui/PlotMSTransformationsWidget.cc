//# PlotMSTransformationsWidget.cc: GUI widget for editing a PlotMSTransformations.
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
#include <plotms/Gui/PlotMSTransformationsWidget.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/GuiTabs/PlotMSTab.qo.h>

namespace casa {

PlotMSTransformationsWidget::PlotMSTransformationsWidget(QWidget* parent) :
        QtEditingWidget(parent), itsFlag_(true) {    

  // Setup the gui
  setupUi(this);

  // First add a "no change" frame
  frame->addItem("");
  // Now add the general set of freq frames
  for (uInt i=MFrequency::LSRK;i<MFrequency::N_Types;++i)
    frame->addItem(MFrequency::showType(i).c_str());

  // Add supported veldefs (these are the most meaningful for _plotting_)
  velDef->addItem("RADIO");
  velDef->addItem("TRUE");
  velDef->addItem("OPTICAL");
  // someday we'll permit all types supported by MDoppler?
  //  for (uInt i=0;i<MDoppler::N_Types;++i)
  //    velDef->addItem(MDoppler::showType(i).c_str());

  // Connect widgets
  connect(frame, SIGNAL(currentIndexChanged(const QString&)),
	  SIGNAL(changed()));
  connect(velDef, SIGNAL(currentIndexChanged(const QString&)),
	  SIGNAL(changed()));
  connect(restFreq, SIGNAL(textChanged(const QString&)),
	  SIGNAL(changed()));
  connect(pcXval, SIGNAL(textChanged(const QString&)),
	  SIGNAL(changed()));
  connect(pcYval, SIGNAL(textChanged(const QString&)),
	  SIGNAL(changed()));

}

PlotMSTransformationsWidget::~PlotMSTransformationsWidget() { }


PlotMSTransformations PlotMSTransformationsWidget::getValue() const {
    PlotMSTransformations trans;

    trans.setFrame(frame->currentText().toStdString());
    trans.setVelDef(velDef->currentText().toStdString());
    trans.setRestFreq(restFreq->text().toDouble());
    trans.setXpcOffset(pcXval->text().toDouble());
    trans.setYpcOffset(pcYval->text().toDouble());
    
    return trans;
}

void PlotMSTransformationsWidget::setValue(const PlotMSTransformations& trans ) {

    // Set widget values
    PlotMSTab::setChooser(frame,trans.frameStr());
    PlotMSTab::setChooser(velDef,trans.veldefStr());
    restFreq->setText(QString::number(trans.restFreq(),'g',12));
    pcXval->setText(QString::number(trans.xpcOffset(),'g'));
    pcYval->setText(QString::number(trans.ypcOffset(),'g'));

}

}
