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
#ifndef SPECFITSETTINGSFIXEDTABLECELL_QO_H
#define SPECFITSETTINGSFIXEDTABLECELL_QO_H

#include <QtGui/QWidget>
#include <display/QtPlotter/SpecFitSettingsFixedTableCell.ui.h>

class SpecFitSettingsFixedTableCell : public QWidget {
	Q_OBJECT

public:
	SpecFitSettingsFixedTableCell(QWidget *parent = 0);
	~SpecFitSettingsFixedTableCell();
	QString getFixedStr() const;
	void setFixedCenter( bool fixed );
	void setFixedPeak( bool fixed );
	void setFixedFwhm( bool fixed );
	bool isCenterFixed() const;
	bool isFWHMFixed() const;
	bool isPeakFixed() const;
	QSize sizeHint() const;
	void clear();

private:
	Ui::SpecFitSettingsFixedTableCell ui;
	const QString GAUSSIAN_ESTIMATE_FIXED_CENTER;
	const QString GAUSSIAN_ESTIMATE_FIXED_PEAK;
	const QString GAUSSIAN_ESTIMATE_FIXED_FWHM;
};

#endif // SPECFITSETTINGSFIXEDTABLECELL_H
