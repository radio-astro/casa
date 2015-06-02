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


#include <casa/aips.h>
#include <casa/Containers/Record.h>

#include <display/QtPlotter/QtDrawCanvas.qo.h>
#include <display/QtPlotter/QtDrawer.qo.h>

#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QColor>
#include <QHash>
#include <QWidget>
#include <QMouseEvent>
#include <cmath>
#include <QtGui>
#include <iostream>
#include <graphics/X11/X_exit.h>


namespace casa {

	QtDrawer::~QtDrawer() {
		delete pc;
		delete zoomInButton;
		delete zoomOutButton;
		delete printButton;
		delete saveButton;
		delete printExpButton;
		//delete saveExpButton;
		//delete writeButton;
		delete openButton;
	}

	QtDrawer::QtDrawer(QWidget *parent, const char *name,
	                   int nr, int nc)
		:QWidget(parent), fileName(name), numRow(nr), numCol(nc) {
		initQtDrawerResource();
		setWindowTitle(name);
		setWindowIcon(QIcon(":/images/nrao.png"));
		setBackgroundRole(QPalette::Dark);

		QHBoxLayout *buttonLayout = new QHBoxLayout;
		zoomInButton = new QToolButton(this);
		zoomInButton->setIcon(QIcon(":/images/zoomin.png"));
		zoomInButton->setToolTip("step in the zoom stack = control+ key");
		zoomInButton->adjustSize();
		connect(zoomInButton, SIGNAL(clicked()), this, SLOT(zoomIn()));

		zoomOutButton = new QToolButton(this);
		zoomOutButton->setIcon(QIcon(":/images/zoomout.png"));
		zoomOutButton->setToolTip("step out the zoom stack = control- key");
		zoomOutButton->adjustSize();
		connect(zoomOutButton, SIGNAL(clicked()), this, SLOT(zoomOut()));

		printButton = new QToolButton(this);
		printButton->setIcon(QIcon(":/images/print.png"));
		printButton->setToolTip("select a printer and print to");
		printButton->adjustSize();
		connect(printButton, SIGNAL(clicked()), this, SLOT(print()));

		saveButton = new QToolButton(this);
		saveButton->setIcon(QIcon(":/images/save.png"));
		saveButton->setToolTip("select a file and save to,\n"
		                       "view with 'casapictureviewer'");
		saveButton->adjustSize();
		connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));

		printExpButton = new QToolButton(this);
		printExpButton->setIcon(QIcon(":/images/printExp.png"));
		printExpButton->setToolTip("print to the default printer");
		printExpButton->adjustSize();
		connect(printExpButton, SIGNAL(clicked()), this, SLOT(printExp()));

		//saveExpButton = new QToolButton(this);
		//saveExpButton->setIcon(QIcon(":/images/saveExp.png"));
		//saveExpButton->setToolTip("save to a image file,\n"
		//                          "view with 'casapictureviewer'");
		//saveExpButton->adjustSize();
		//connect(saveExpButton, SIGNAL(clicked()), this, SLOT(saveExp()));

		//writeButton = new QToolButton(this);
		//writeButton->setIcon(QIcon(":/images/write.png"));
		//writeButton->setToolTip("write to a text file,\n"
		//                          "view with 'casaplotter'");
		//writeButton->adjustSize();
		//connect(writeButton, SIGNAL(clicked()), this, SLOT(writeText()));

		openButton = new QToolButton(this);
		openButton->setIcon(QIcon(":/images/open.png"));
		openButton->setToolTip("open a data file in the following format:\n"
		                       "0     1\n"
		                       "10    10\n"
		                       "\n"
		                       "1     1\n"
		                       "2     4\n"
		                       "3     9\n");
		openButton->adjustSize();
		connect(openButton, SIGNAL(clicked()), this, SLOT(openText()));

		leftButton = new QToolButton(this);
		leftButton->setIcon(QIcon(":/images/leftarrow.xbm"));
		leftButton->setToolTip("scroll zoomed frame left = left arrow key");
		leftButton->adjustSize();
		connect(leftButton, SIGNAL(clicked()), this, SLOT(left()));

		rightButton = new QToolButton(this);
		rightButton->setIcon(QIcon(":/images/rightarrow.xbm"));
		rightButton->setToolTip("scroll zoomed frame right = right arrow key");
		rightButton->adjustSize();
		connect(rightButton, SIGNAL(clicked()), this, SLOT(right()));

		upButton = new QToolButton(this);
		upButton->setIcon(QIcon(":/images/uparrow.xbm"));
		upButton->setToolTip("scroll zoomed frame up = up arrow key");
		upButton->adjustSize();
		connect(upButton, SIGNAL(clicked()), this, SLOT(up()));

		downButton = new QToolButton(this);
		downButton->setIcon(QIcon(":/images/downarrow.xbm"));
		downButton->setToolTip(
		    "scroll zoomed frame down = down arrow key");
		downButton->adjustSize();
		connect(downButton, SIGNAL(clicked()), this, SLOT(down()));

		buttonLayout->addWidget(zoomInButton);
		buttonLayout->addWidget(zoomOutButton);
		buttonLayout->addWidget(leftButton);
		buttonLayout->addWidget(rightButton);
		buttonLayout->addWidget(upButton);
		buttonLayout->addWidget(downButton);
		buttonLayout->addItem(
		    new QSpacerItem(40, zoomInButton->height(),
		                    QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
		buttonLayout->addWidget(openButton);
		//buttonLayout->addWidget(writeButton);
		buttonLayout->addWidget(printExpButton);
		//buttonLayout->addWidget(saveExpButton);
		buttonLayout->addWidget(printButton);
		buttonLayout->addWidget(saveButton);

		if (numRow < 1)
			numRow = 1;
		if (numCol < 1)
			numCol = 1;

		if (!layout())
			new QVBoxLayout(this);

		gridGroup = new QGroupBox;
		grid = new QGridLayout;
		gridGroup->setLayout(grid);

		for (int i = 0; i < numRow; i++) {
			for (int j = 0; j < numCol; j++) {
				QtDrawCanvas *c = new QtDrawCanvas(this);
				c->setObjectName(QString("%1|%2").arg(i).arg(j));
				QPalette pal = c->palette();
				pal.setColor(QPalette::Background, Qt::white);
				c->setPalette(pal);
				c->setFocusPolicy(Qt::StrongFocus);
				connect(c, SIGNAL(gotFocus()),
				        this, SLOT(focusChanged()));
				grid->addWidget(c, i, j);
			}
		}

		pc = canvas();

		layout()->addWidget(gridGroup);
		layout()->addItem(buttonLayout);

		zoomInButton->hide();
		zoomOutButton->hide();
		upButton->hide();
		downButton->hide();
		leftButton->hide();
		rightButton->hide();

		QSettings settings("CASA", "Viewer");
		QString pName = settings.value("Print/printer").toString();
		printExpButton->setVisible(!pName.isNull());
		//saveExpButton->hide();

		connect(pc, SIGNAL(zoomChanged()), this, SLOT(updateZoomer()));

		if (name != 0) {
			pc->plotPolyLines(name);
		}

	}

	QtDrawCanvas* QtDrawer::canvas(int i, int j) {
		QList<QObject *> widgets = gridGroup->children();
		//qDebug() << "widgets:" << widgets;
		for (int k = 0; k < widgets.size(); k++) {
			QWidget* pw = (QWidget*)widgets.at(k);
			QString item = pw->objectName();
			//qDebug() << "item=" << item;
			if (item == QString("%1|%2").arg(i).arg(j)) {
				return (QtDrawCanvas*)pw;
			}
		}
		return new QtDrawCanvas;
	}

	void QtDrawer::zoomOut() {
		pc->zoomOut();
	}

	void QtDrawer::zoomIn() {
		pc->zoomIn();
	}

	void QtDrawer::left() {
		QApplication::sendEvent(pc,
		                        new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, 0, 0));
	}

	void QtDrawer::right() {
		QApplication::sendEvent(pc,
		                        new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, 0, 0));
	}

	void QtDrawer::up() {
		QApplication::sendEvent(pc,
		                        new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, 0, 0));
	}

	void QtDrawer::down() {
		QApplication::sendEvent(pc,
		                        new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, 0, 0));
	}

	void QtDrawer::print() {
#ifndef QT_NO_PRINTER
		QPrinter printer(QPrinter::ScreenResolution);
		//printer.setFullPage(true);
		QPrintDialog *dlg = new QPrintDialog(&printer, this);
		if (dlg->exec() == QDialog::Accepted) {
			QSettings settings("CASA", "Viewer");
			settings.setValue("Print/printer", printer.printerName());
			printIt(&printer);
		}
		delete dlg;
#endif
	}

	void QtDrawer::printIt(QPrinter* printer) {
		QPainter painter(printer);
		QRect rect = painter.viewport();
		rect.adjust(72, 72, -72, -72);
		QPixmap *mp = pc->graph();
		QSize size = mp->size();
		size.scale(rect.size(), Qt::KeepAspectRatio);
		painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
		painter.setWindow(mp->rect());
		painter.drawPixmap(0, 0, *mp);
		painter.end();
	}

	void QtDrawer::printExp() {
#ifndef QT_NO_PRINTER
		QPrinter printer(QPrinter::ScreenResolution);
		QSettings settings("CASA", "Viewer");
		printer.setPrinterName(settings.value("Print/printer").toString());
		printIt(&printer);
#endif
	}

	void QtDrawer::save() {
		QString fn = QFileDialog::getSaveFileName(this,
		             tr("Save as..."),
		             QString(), tr(
		                 "(*.png);;(*.xpm);;(*.jpg);;"
		                 "(*.xbm);;(*.ppm);;(*.jpeg)"));

		if (fn.isEmpty())
			return ;
		fn = fn.section('/', -1);

		QFile file(fn);
		if (!file.open(QFile::WriteOnly))
			return ;

		char* t = (char*)("png");
		QString ext = fn.section('.', -1);
		if (ext == "xpm" || ext == "jpg" || ext == "png" ||
		        ext == "xbm" || ext == "ppm" || ext == "jpeg")
			t = (char*)ext.toLocal8Bit().constData();
		else
			fn.append(".png");

		int width = pc->graph()->width();
		int height = pc->graph()->height();

		//qDebug() << "before save" ;
		pc->graph()->scaled(width, height,
		                    Qt::KeepAspectRatio).save(fn, t);
		//qDebug() << "after save" ;
		return ;
	}

	void QtDrawer::saveExp() {
		//qDebug() << "fileName:" << fileName;
		QFile file(fileName.append(".png"));
		if (!file.open(QFile::WriteOnly))
			return ;
		//qDebug() << "open ok";

		pc->graph()->save(fileName, "PNG");
		//qDebug() << "save ok";
		return ;
	}

	void QtDrawer::writeText() {
		//qDebug() << "fileName:" << fileName;
		QString fn = QFileDialog::getSaveFileName(this,
		             tr("Write plot data as text"),
		             QString(), tr( "(*.txt);;(*.plt)"));

		if (fn.isEmpty())
			return ;
		fn = fn.section('/', -1);

		QFile file(fn);
		if (!file.open(QFile::WriteOnly | QIODevice::Text))
			return ;

		char* t = (char*)("plt");
		QString ext = fn.section('.', -1);
		if (ext == "txt" || ext == "plt")
			t = (char*)ext.toLocal8Bit().constData();
		else
			fn.append(".plt");

		QTextStream ts(&file);

		ts << "#Title:" << fileName << "\n";
		ts << "#xLabel:\n";
		ts << "#yLabel:\n";

		return ;
	}

	void QtDrawer::openText() {
		QFileDialog *fdlg = new QFileDialog(0,
		                                    "Choose a file to open",
		                                    ".",
		                                    "Plots (*.plt)");
		QStringList fNames;
		if (fdlg->exec())
			fNames = fdlg->selectedFiles();
		if (!fNames.isEmpty()) {
			pc->plotPolyLines(fNames.at(0));
		}
		delete fdlg;
	}

	void QtDrawer::updateZoomer() {
		if (pc->getCurZoom() > 0) {
			zoomOutButton->setEnabled(true);
			zoomOutButton->show();
		}
		if (pc->getCurZoom() == 0) {
			zoomOutButton->setEnabled(false);
		}
		if (pc->getCurZoom() < pc->getZoomStackSize() - 1) {
			zoomInButton->setEnabled(true);
			zoomInButton->show();
		}
		if (pc->getCurZoom() == pc->getZoomStackSize() - 1) {
			zoomInButton->setEnabled(false);
		}
		upButton->setVisible(1);
		downButton->setVisible(1);
		leftButton->setVisible(1);
		rightButton->setVisible(1);
	}

	void QtDrawer::focusChanged() {
		//qDebug() << "focus changed";
		QtDrawCanvas* c = dynamic_cast<QtDrawCanvas*>(sender());
		QString item = c->objectName();
		//qDebug() << "item=" << item;
		QStringList list = item.split("|");
		int i = list.at(0).toInt();
		int j = list.at(1).toInt();
		pc = canvas(i, j);
		updateZoomer();
	}
//void QtDrawer::closeEvent (QCloseEvent* event) {
	//qDebug() << "closeEvent";
//}

}
