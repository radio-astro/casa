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


#ifndef TQPLOTTER_H
#define TQPLOTTER_H

#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Inputs/Input.h>
#include <casa/Arrays/IPosition.h>
#include <display/QtPlotter/QtDrawCanvas.qo.h>


#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QColor>
#include <QHash>
#include <QWidget>
#include <QMouseEvent>
#include <QToolButton>
#include <QDialog>
#include <QPixmap>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <map>
#include <vector>
#include <graphics/X11/X_exit.h>

inline void initQtDrawerResource() {
	Q_INIT_RESOURCE(QtDrawer);
}

namespace casa {


	class QtDrawer : public QWidget {
		Q_OBJECT
	public:
		QtDrawer(QWidget *parent = 0, const char *name = 0,
		         int nr = 1, int rc = 1);
		~QtDrawer();

		QtDrawCanvas* canvas(int i = 0, int j = 0);

	public slots:
		void zoomIn();
		void zoomOut();
		void print();
		void save();
		void printExp();
		void saveExp();
		void writeText();
		void openText();
		void up();
		void down();
		void left();
		void right();
		void updateZoomer();
		void focusChanged();
		//virtual void closeEvent ( QCloseEvent * event );

	signals:

	private:

		void printIt(QPrinter*);
		QToolButton *zoomInButton;
		QToolButton *zoomOutButton;
		QToolButton *leftButton;
		QToolButton *rightButton;
		QToolButton *upButton;
		QToolButton *downButton;

		QToolButton *printButton;
		QToolButton *saveButton;
		QToolButton *printExpButton;
		//QToolButton *saveExpButton;
		//QToolButton *writeButton;
		QToolButton *openButton;

		QtDrawCanvas *pc;

		QString fileName;

		QGridLayout *grid;
		QGroupBox *gridGroup;
		int numRow;
		int numCol;
	};


}
#endif

