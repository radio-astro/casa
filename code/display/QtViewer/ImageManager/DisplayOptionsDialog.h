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

#ifndef DISPLAYOPTIONSDIALOG_H_
#define DISPLAYOPTIONSDIALOG_H_

#include <QDialog>

namespace casa {

class QtDisplayDataGui;
class QtDisplayData;

/**
 * Opens up a dialog showing the display options that can be
 * tweaked for a particular images.
 */
class DisplayOptionsDialog : public QDialog {
public:
	DisplayOptionsDialog(QWidget* parent );
	//Shows the dialog for the given display data.
	void showOptions( QtDisplayData* );
	virtual ~DisplayOptionsDialog();
private:
	DisplayOptionsDialog( const DisplayOptionsDialog& other );
	DisplayOptionsDialog operator=( const DisplayOptionsDialog& other );
	QtDisplayDataGui* displayedDataOptions;
};

} /* namespace casa */
#endif /* DISPLAYOPTIONSDIALOG_H_ */
