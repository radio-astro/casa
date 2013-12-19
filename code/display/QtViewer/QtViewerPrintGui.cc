//# QtViewerPrintGui.cc:  Printing dialog for QtViewer
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

#include <stdio.h>
#include <graphics/X11/X_enter.h>
#include   <QtGui/QtGui>
#include   <QtCore/QDebug>
#include   <QFile>
#include   <QFileInfo>
#include <graphics/X11/X_exit.h>

#include <display/QtViewer/QtViewerPrintGui.qo.h>
#include <display/QtViewer/QtViewerBase.qo.h>
#include <display/QtViewer/QtPixelCanvas.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtAutoGui/QtGuiEntry.qo.h>
#include <display/QtAutoGui/QtLayout.h>
#include <display/Display/PanelDisplay.h>
#include <display/Display/WorldCanvas.h>
#include <display/DisplayDatas/DisplayData.h>
#include <display/DisplayEvents/PCITFiddler.h>
#include <display/DisplayEvents/MWCRTZoomer.h>
#include <display/Display/AttributeBuffer.h>
#include <casa/BasicSL/String.h>

namespace casa { //# NAMESPACE CASA - BEGIN

////////////////////////  QtViewerPrintGui /////////////////////////////////

	QtViewerPrintGui::QtViewerPrintGui(QtDisplayPanel *dp)
		: m_doc(), printer(0), pDP(dp) {
		setWindowTitle("Viewer Print Manager");
		setObjectName(QString::fromUtf8("PrintManager"));
		//Q_INIT_RESOURCE(QtAutoGui);
		setGeometry(QRect(10, 0, 560, 260));

		vboxLayout = new QVBoxLayout;
		vboxLayout->setSpacing(6);
		vboxLayout->setMargin(0);
		vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));

		connect(pDP, SIGNAL(resized(QSize, QSize)),
		        this, SLOT(dpResized(QSize, QSize)));

		vboxLayout->setSizeConstraint(QLayout::SetFixedSize);
		this->setLayout(vboxLayout);
		m_doc.setContent(QtViewerPrintGui::printRecord);
		load(m_doc);
	}

	QtViewerPrintGui::~QtViewerPrintGui()
	{}

	void QtViewerPrintGui::loadRecord(Record rec) {
		QtXmlRecord xmlRecord;
		xmlRecord.recordToDom(&rec, m_doc);
		load(m_doc);
		//cout << "\nRecord:\n";
		//xmlRecord.printRecord(&rec);
		//cout << endl;
		update();
	}

	bool QtViewerPrintGui::load(QDomDocument &doc) {

		//cout << "------------doc: " << doc.toString().toStdString() << endl;
		if (doc.isNull())
			return false;
		QDomElement root = doc.firstChildElement();
		if (root.isNull())
			return false;
		QWidget *wgt = new QWidget();
		QVBoxLayout *vLayout = new QVBoxLayout;
		vLayout->setMargin(10);
		vLayout->setSpacing(6);
		vLayout->setSpacing(1);

		QDomElement widget_ele = root.firstChildElement();
		QSettings settings("CASA", "Viewer");
		QString mda = settings.value("Print/media").toString();
		for (; !widget_ele.isNull(); widget_ele = widget_ele.nextSiblingElement()) {
			if (widget_ele.tagName() == "printmedia") {
				widget_ele.setAttribute("default", mda);
				widget_ele.setAttribute("value", mda);
				break;
			}
		}

		QHBoxLayout *titleLayout = new QHBoxLayout;
		QWidget *ttl = new QWidget;
		QLabel *pwd = new QLabel("Working Directory: " + QDir::currentPath());
		titleLayout->addWidget(pwd);
		ttl->setLayout(titleLayout);
		vboxLayout->addWidget(ttl);

		QHBoxLayout *sizeLayout = new QHBoxLayout;
		QWidget *tt2 = new QWidget;
		sizeLabel = new QLabel;
		sizeLayout->addWidget(sizeLabel);
		tt2->setLayout(sizeLayout);
		vboxLayout->addWidget(tt2);
		dpResized(pDP->size(), pDP->canvasSize());

		widget_ele = root.firstChildElement();
		for (; !widget_ele.isNull();
		        widget_ele = widget_ele.nextSiblingElement()) {
			//cout << "item=" << widget_ele.tagName().toStdString() << endl;
			QString ptype = widget_ele.attribute("ptype", "noType");

			if (ptype == "intrange" || ptype == "floatrange") {
				if (// ele.attribute("editable") == "1" &&
				    widget_ele.attribute("provideentry") == "1") {
					QtSliderEditor *item = new  QtSliderEditor(widget_ele);
					vLayout->addWidget(item);
					connect(item,
					        SIGNAL(itemValueChanged(
					                   QString, QString, int, bool)),
					        this,
					        SLOT(itemValueChanged(
					                 QString,QString, int, bool)));
				}

				else {
					QtSliderLabel *item = new  QtSliderLabel(widget_ele);
					vLayout->addWidget(item);
					connect(item, SIGNAL(itemValueChanged(
					                         QString, QString, int, bool)),
					        this, SLOT(itemValueChanged(
					                       QString, QString, int, bool)));
				}
			}

			else if (ptype == "choice" || ptype == "userchoice") {
				QtCombo *item = new  QtCombo(widget_ele);
				// item->setObjectName(widget_ele.tagName());
				vLayout->addWidget(item);
				connect(item, SIGNAL(itemValueChanged(
				                         QString, QString, int, bool)),
				        this, SLOT(itemValueChanged(
				                       QString, QString, int, bool)));
			}

			else if (ptype == "intpair" || ptype == "doublepair") {
				QtPairEditor* item = new QtPairEditor(widget_ele);
				vLayout->addWidget(item);
				connect(item, SIGNAL(itemValueChanged(
				                         QString, QString, int, bool)),
				        this, SLOT(itemValueChanged(
				                       QString, QString, int, bool)));
			}

			else if (ptype == "int" || ptype == "double") {
				QtNumberEditor* item = new QtNumberEditor(widget_ele);
				vLayout->addWidget(item);
				connect(item, SIGNAL(itemValueChanged(
				                         QString, QString, int, bool)),
				        this, SLOT(itemValueChanged(
				                       QString, QString, int, bool)));
			}

			else if (ptype == "string") {
				QtLineEditor *item = new  QtLineEditor(widget_ele);
				//item->setObjectName(widget_ele.tagName());
				vLayout->addWidget(item);
				connect(item, SIGNAL(itemValueChanged(
				                         QString, QString, int, bool)),
				        this, SLOT(itemValueChanged(
				                       QString, QString, int, bool)));
			}

			else if (ptype == "boolean") {
				QtBool *item = new  QtBool(widget_ele);
				vLayout->addWidget(item);
				connect(item, SIGNAL(itemValueChanged(
				                         QString, QString, int, bool)),
				        this, SLOT(itemValueChanged(
				                       QString, QString, int, bool)));
			}

			else
			{}
			emit itemValueChanged(widget_ele.tagName(), widget_ele.attribute("value"),
			                      0, false);

			bool block = blockSignals(true);
			blockSignals(block);

		}

		wgt->setLayout(vLayout);
		vboxLayout->addWidget(wgt);

		QHBoxLayout *cmdLayout = new QHBoxLayout;
		QWidget *cmd = new QWidget;

		QStringList saveType;
		saveType << "PS" << "EPS" << "PDF"
		         << "JPG" << "PNG"
		         << "XBM" << "XPM"
		         << "PPM" ;

		QComboBox *cbSaveType = new QComboBox( );
		cbSaveType->addItems( saveType );
		cbSaveType->setEditable( false );
		cbSaveType->setToolTip( "format of the file to be saved" );
		QSize cbsize(cbSaveType->sizeHint());
		cbsize.setWidth(50);
		cbSaveType->setFixedSize(cbsize);
		connect(cbSaveType, SIGNAL(currentIndexChanged(const QString&)),
		        this, SLOT(selectOutputType(const QString&)));
		cmdLayout->addWidget(cbSaveType);
		savefiletype = cbSaveType->currentText( );

		QPushButton *bSaveXPM = new QPushButton("Save");
		bSaveXPM->setToolTip("save image of file-type to left on disk");
		connect(bSaveXPM, SIGNAL(clicked()), this, SLOT(saveXPM()));
		cmdLayout->addWidget(bSaveXPM);

		QPushButton *bPrint = new QPushButton("Print");
		bPrint->setToolTip("Press this button to open a window which "
		                   "will allow you to send a PostScript image "
		                   "to a printer");
		connect(bPrint, SIGNAL(clicked()),  this, SLOT(print()));
		QPushButton *bDismiss = new QPushButton("Dismiss");
		connect(bDismiss, SIGNAL(clicked()),  this, SLOT(dismiss()));

		cmdLayout->addWidget(bPrint);
		cmdLayout->addWidget(bDismiss);

		cmd->setLayout(cmdLayout);
		vboxLayout->addWidget(cmd);

		return true;
	}

	QString QtViewerPrintGui::printFileName() const {
		return printfilename;
	}
	QString QtViewerPrintGui::printerName() const {
		if (printer)
			return printer->printerName();
		else
			return "";
	}
	/*
	void QtViewerPrintGui::setOriginal()
	{
	    std::cout << "set original options" << std::endl;
	    QDomElement ele = m_doc.firstChildElement().firstChildElement()
	                      .firstChildElement();
	    for (; !ele.isNull(); ele = ele.nextSiblingElement())
	    {
	        ele.setAttribute("value", ele.attribute("saved"));
	    }
	}
	*/
	void QtViewerPrintGui::dismiss() {
		close();
	}

	void  QtViewerPrintGui::adjustEpsBoundingBox( const char *from, const char *to,
	        const QSize &wcmax,
	        const QRect &viewport ) {
		FILE *in = fopen( from, "r" );
		FILE *out = fopen( to, "w" );
		bool found = false;
		char buf[2049];
		while ( ! feof(in) ) {
			char *line = fgets( buf, 2049, in );
			if ( line ) {
				if ( ! found && ! strncmp( "%%BoundingBox: ", line, 15 ) ) {

					float xmin, xmax, ymin, ymax;
					int g = sscanf( line, "%%%%BoundingBox: %f %f %f %f", &xmin, &ymin, &xmax, &ymax );

					if ( g != 4 ) {
						fputs( line, out );
					} else {
						float ratio_y = ymax / float(viewport.height());
						float ratio_x = xmax / float(viewport.width());

						fprintf( out, "%%%%BoundingBox: 0 %d %d %d\n",
						         int((ymax - (wcmax.height() * ratio_y)) + 1),
						         int((wcmax.width() * ratio_x) + 1),
						         int(ymax + 1) );
					}

					found = true;

				} else {
					fputs( line, out );
				}
			}
		}
		fclose( out );
		fclose( in );
	}

	void  QtViewerPrintGui::printPS(QPrinter *printer, const QString printerType) {

		if ( !printer || !pDP ) return;

		if ( printerType == "PS" || printerType == "EPS" ) {
			printer->setOutputFormat(QPrinter::PostScriptFormat);
		} else if ( printerType == "PDF" ) {
			printer->setOutputFormat(QPrinter::PdfFormat);
		}

		printer->setResolution(printresolution);
		if (printorientation == "Landscape") {
			printer->setOrientation(QPrinter::Landscape);
		} else if (printorientation == "2-Up") {
			printer->setOrientation(QPrinter::Portrait);
		} else {
			printer->setOrientation(QPrinter::Portrait);
		}

		if (printmedia == "A4") {
			printer->setPageSize(QPrinter::A4);
		} else {
			printer->setPageSize(QPrinter::Letter);
		}

		char eps_file_name[40];
		if ( printerType == "EPS" ) {
			pid_t pid = getpid( );
			sprintf( eps_file_name, "/tmp/eps-out.%06d", pid );
			printer->setFullPage(true);
			printer->setOutputFileName(eps_file_name);
		} else if ( printerType == "Ghostview" || printerType == "PS" || printerType == "PDF" ) {
			printer->setOutputFileName(printfilename);
		} else {
			printer->setPrinterName(printerType);
		}

		QPainter painter(printer);
		QRect viewport = painter.viewport();
		QRect rect = viewport;
		rect.adjust(72, 72, -72, -72);

		//qDebug() << "painter width=" << rect.width();
		//qDebug() << "painter height=" << rect.height();

		QSize sz = pDP->size();

		int xs = sz.width();
		int ys = sz.height();
		QSize pSz(xs, ys);

		//qt bug, Qt::KeepAspectRatio does expending
		//pSz.scale(rect.size(), Qt::KeepAspectRatio);
		//use the following instead
		double ratio = 1;
		if (printorientation == "landscape") {
			//cout << "landscape" << endl;
			double rx = (double)rect.height() / xs;
			double ry = (double)rect.width() / ys;
			ratio = min(rx, ry);
		} else {
			double rx = (double)rect.width() / xs;
			double ry = (double)rect.height() / ys;
			ratio = min(rx, ry);
		}

		pSz.setWidth(int(xs * ratio));
		pSz.setHeight(int(ys * ratio));

		pDP->pushCurrentDrawingState( );
		pDP->setLabelLineWidth(pDP->getLabelLineWidth( ) * ratio);
		pDP->setTickLength(pDP->getTickLength( ) * ratio);
		QPixmap pmp(pSz);
		QApplication::setOverrideCursor(Qt::WaitCursor);
		pDP->setAllowBackToFront(false);
		String backColor = "white";
		String foreColor = "black";
		pDP->setBackgroundPS(backColor, foreColor);
		pDP->beginLabelAndAxisCaching( );
		pDP->resize(pSz);
		pmp = pDP->getBackBuffer()->copy();
		painter.drawPixmap(0,0,pmp);
		pDP->endLabelAndAxisCaching( painter );
		painter.end();
		pDP->popCurrentDrawingState( );
		pDP->setBackgroundPS(backColor, foreColor);
		pDP->setAllowBackToFront(true);
		pDP->resize(sz);

		if ( printerType == "EPS" ) {
			adjustEpsBoundingBox( eps_file_name, printfilename.toAscii( ), pmp.size(), viewport );
			remove( eps_file_name );
		}

		QApplication::restoreOverrideCursor();

		if ( printerType == "Ghostview") {
			QString program = "ghostview"; //usr/X11R6/bin/ghostview
			QStringList arguments;
			arguments << printfilename;
			QProcess *ghostProcess = new QProcess(this);
			ghostProcess->start(program, arguments);
			return;
		}
	}

	void QtViewerPrintGui::print() {
		printer = new QPrinter;
		//printer = new QPrinter(QPrinter::ScreenResolution);

		//cout << " printer before init: " << (*printer);

		QtViewerPrintCtl *ctl = new QtViewerPrintCtl(this);
		if (ctl->exec() == QDialog::Accepted) {
			printfilename = ctl->fileName();
			printmedia = ctl->paper();
			printorientation = ctl->orientation();
			printPS(printer, ctl->printerName());
		}

		//cout << " printer setting: " << (*this);
		delete printer;
	}

	void QtViewerPrintGui::selectOutputType( const QString & text ) {
		savefiletype = text;
	}

	void QtViewerPrintGui::saveXPM() {
		QString ext = savefiletype;
		QString saved_printfilename = printfilename;

		if (printfilename == "unset" || printfilename.isEmpty()) {

			QDateTime qdt = QDateTime::currentDateTime();
			printfilename = "viewer-" + qdt.toString(Qt::ISODate)
			                + "." + ext.toLower();

		} else {

			QFileInfo file(printfilename);
			QString suffix = file.suffix( ).toLower( );
			QString ext_ = ext.toLower();

			if (ext_ == "jpg") {
				if (suffix != "jpg" && suffix != "jpeg") {
					printfilename += ".jpg";
				}
			} else {
				if (suffix != ext_) {
					printfilename += "." + ext_;
				}
			}

		}

		if (ext == "EPS" || ext == "PS" || ext == "PDF" ) {

			printer = new QPrinter;
			printPS(printer, ext);
			delete printer;

		} else if ( pDP ) {

			QSize s = pDP->canvasSize();
			int width = s.width();
			int height = s.height();

			if(printimgresolution.first > 0 && printimgresolution.second > 0) {
				width = printimgresolution.first;
				height = printimgresolution.second;
			}

			width = (int)((width * printimgsizefactor) + 0.5);
			height = (int)((height * printimgsizefactor) + 0.5);

			/*
			if (printmagnification > 0)
			{
			    width  = int(printmagnification * width);
			    height = int(printmagnification * height);
			}
			*/

			QByteArray format = ext.toAscii();
			//char* t= (char*)ext.toLocal8Bit().constData();<--bad, for some reason

			if(s.width() == width && s.height() == height) {
				// screenshot
				pDP->hold();
				QPixmap* mp = pDP->contents();
				if(!mp->save(printfilename, format))
					QMessageBox::warning(this, "Save Error",
					                     "Unable to save to " + printfilename + "!");
				delete mp;
				pDP->release();
			} else {
				// resized
				QApplication::setOverrideCursor(Qt::WaitCursor);

				QSize oldSize = pDP->size();
				QSize scaledSize = s;
				int dw = oldSize.width() - scaledSize.width(),
				    dh = oldSize.height() - scaledSize.height();
				scaledSize.scale(width, height, Qt::KeepAspectRatio);

				//pDP->setAllowBackToFront(false); <-- for back buffer
				// have to add pixels for the border that the QtDisplayPanel adds
				// around the QtPixelCanvas
				pDP->setUpdateAllowed(False);
				// (Prevent display widget flashing during temporary resize.)
				pDP->resize(scaledSize.width() + dw, scaledSize.height() + dh);
				QPixmap* mp = pDP->contents();
				pDP->setUpdateAllowed(True);
				pDP->resize(oldSize);
				QCoreApplication::processEvents();
				//pDP->getBackBuffer()->save(printfilename, format);<-- back buffer
				if(!mp->save(printfilename, format))
					QMessageBox::warning(this, "Save Error",
					                     "Unable to save to " + printfilename + "!");
				//pDP->setAllowBackToFront(true); <-- for back buffer
				delete mp;

				QApplication::restoreOverrideCursor();
			}
		}
		/****
		***** We mess with the printfilename, but we must now preserve the original
		***** because it is no longer returned to 'unset'...
		****/
		printfilename = saved_printfilename;
	}

	void QtViewerPrintGui::savePS(QPixmap *pmp, int px, int py) {
		//cout << "savePS clicked" << endl;
		//QMainWindow *wgt = new QMainWindow;
		//QLabel* imageLabel = new QLabel;
		//wgt->setCentralWidget(imageLabel);
		//imageLabel->setPixmap(*pmp);
		//wgt->show();

		if (!pmp) return;

		if (px > 468 || px < 1) px = 468;
		if (py > 648 || py < 1) py = 648;

		int w = pmp->width();
		int h = pmp->height();
		//qDebug() << "image width=" << w;
		//qDebug() << "image height=" << h;

		if (w < 1 || h < 1) return;

		double rw = (double)w / px;
		double rh = (double)h / py;
		double r = max(rw, rh);
		double sx = w / r;
		double sy = h / r;
		//qDebug() << "ps width=" << sx;
		//qDebug() << "ps height=" << sy;

////////////////////////split image/////////////////////

		QString hd =
		    "%!PS-Adobe-3.0\n"
		    "% PostScript generated by CASA\n";

		QString gsv = "gsave\n";

		QString wd =
		    QString("%1 ").arg(w);

		QString bs =
		    QString("%1 ").arg(8);

		QString tl =
		    "true "
		    "3 "
		    "colorimage "
		    "grestore ";

		QString sp = "showpage\n";

		QString fileContent = hd;

		double dy = sy / h;
		//qDebug() << "dy=" << dy;
		QString sc =
		    QString("%1 %2 scale ")
		    .arg(sx, 0, 'f', 8).arg(dy, 0, 'f', 8);

		QString ht =
		    QString("%1 ").arg(1);

		QString fm =
		    QString("[%1 0 0 %2 0 0] ")
		    .arg(w).arg(1);

		QImage img = pmp->toImage();
		for (int i = 0; i < h; i++) {

			double k = 792. - 72. - (i + 1.) * dy;
			//qDebug() << "k=" << k;
			QString tr = QString("72 %1 translate \n").arg(k, 0, 'f', 8);
			QString sec =  gsv + tr + sc + wd + ht + bs + fm;

			QString a = "{<";
			QString b = "{<";
			QString c = "{<";

			for (int j = 0; j < w; j++) {
				QRgb qrgb = img.pixel(j, i);
				QString str = QString().setNum(qrgb, 16);
				a += str.mid(2, 2);
				b += str.mid(4, 2);
				c += str.mid(6, 2);
			}
			a += ">}\n";
			b += ">}\n";
			c += ">}\n";

			sec.append(a).append(b).append(c).append(tl);
			fileContent.append(sec);
		}

		fileContent.append(sp);

		//qDebug() << "printfilename=" << printfilename;
		QFile psfile(printfilename);

		if (!psfile.open(QIODevice::WriteOnly)) {
			cout << "could not open file to write!" << endl;
			return;
		}
		//qDebug() << " size=" << psfile.size()
		//           << " atEnd=" << psfile.atEnd();
		QTextStream ts(&psfile);
		ts << fileContent;
		psfile.close();
/////////////////////////////////////////
		/*
		    QString hd =
		       "%!PS-Adobe-3.0\n"
		       "% PostScript generated by CASA\n";

		    QString gsv = "gsave\n";

		    QString sc =
		       QString("%1 %2 scale ")
		             .arg(sx).arg(sy);

		    QString tr =
		       QString("72 %1 translate \n")
		             .arg(792 - 72 - sy);

		    QString wd =
		       QString("%1 ").arg(w);

		    QString ht =
		       QString("%1 ").arg(h);

		    QString bs =
		       QString("%1 ").arg(8);

		    QString fm =
		       QString("[%1 0 0 %2 0 0] ")
		            .arg(w).arg(h);

		    QString tl =
		       "true "
		       "3 "
		       "colorimage "
		       "grestore ";

		    QString sp = "showpage\n";

		    QString fileContent = hd;

		    QString sec =  gsv + tr + sc + wd + ht + bs + fm;

		    QString a = "{<";
		    QString b = "{<";
		    QString c = "{<";

		    QImage img = pmp->toImage();

		    for (int i = h - 1; i > 267; i--) {

		       for (int j = 0; j < w; j++) {
		          QRgb qrgb = img.pixel(j, i);
		          QString str = QString().setNum(qrgb, 16);
		          a += str.mid(2, 2);
		          b += str.mid(4, 2);
		          c += str.mid(6, 2);
		        }

		    }
		    a += ">}\n";
		    b += ">}\n";
		    c += ">}\n";

		    sec.append(a).append(b).append(c).append(tl);

		    fileContent.append(sec).append(sp);

		    QTextStream ts(&psfile);
		    ts << fileContent;
		    psfile.close();
		*/
		/*
		   Limitcheck error

		If you get a PostScript error “limitcheck” offending command “image”, an image in your document is too large, its resolution is too high or it cannot be rotated. Reduce the size or resolution, rotate the image at a different angle or rotate it in an application like Photoshop.

		Some older level 2 versions of PostScript RIPs as well as Acrobat Distiller 4.0 (and 4.05 and probably 3.x) cannot handle copydot files in which the number of pixels exceeds 32000 in either direction. Using such big copydot files (eg larger than about 33 centimeters for a 2400 dpi copydot) can lead to a PostScript error “limitcheck” offending command “image”.

		If you get a PostScript error “limitcheck” offending command “image” when printing from INdesign 1.0, the document probably contains a multitone EPS (duotone, tritone,.. ) that uses a spot color. To get around the error, you can either perform the colour separation in INdesign itself (deselect “In-RIP” in the separations tab) or you should upgrade your RIP to Adobe PostScript version 3011 or later.
		*/
	}


	void QtViewerPrintGui::itemValueChanged(QString name, QString value,
	                                        int /*action*/, bool /*autoApply*/) {
		//std::cout << "received 4 parameters: name=" << name.toStdString()
		//<< " value=" << value.toStdString()
		//<< " action=" << action
		//<< " apply=" << autoApply
		//<< std::endl;
		QSettings settings("CASA", "Viewer");
		if (name == "printfilename") {
			printfilename = value;
		} else if (name == "printmedia") {
			printmedia = value;
			settings.setValue("Print/media", value);
		} else if (name == "printorientation") {
			printorientation = value;
		} else if (name == "printresolution") {
			printresolution = value.toInt();
		} else if(name == "printmagnification") {
			printmagnification = value.toFloat();
		} else if (name == "printepsformat") {
			printepsformat = value;
		} else if (name == "imgresolution") {
			if(value.contains(" ")) {
				QTextStream ss(&value);
				ss >> printimgresolution.first;
				ss >> printimgresolution.second;

			} else {
				printimgresolution.first = value.toInt();
				printimgresolution.second = printimgresolution.first;
			}
		} else if (name == "imgsizefactor") {
			printimgsizefactor = value.toDouble();
		} else
		{}

		//cout << "finished itemValueChanged" << std::endl;


	}

	void QtViewerPrintGui::ps2eps(const QString &filename, QRect rect) {
		//qDebug() << "current dir=" << QDir::currentPath();
		//QString fname = QDir::currentPath() + "/" + filename;
		//qDebug() << "epsfile " << fname << " exist=" << QFile::exists(fname);

		QFile epsfile(filename);

		if (! epsfile.open(QIODevice::ReadOnly)) {
			return;
		}
		//qDebug() << " size=" << epsfile.size() << " atEnd=" << epsfile.atEnd();

		QTextStream ts(&epsfile);
		QString fileContent= ts.readAll();
		epsfile.close();

		//qDebug() << fileContent;

		if (fileContent.indexOf("EPSF") > 0)
			return;

		QRegExp rx("%%BoundingBox:\\s*(-?[\\d\\.:]+)\\s*(-?[\\d\\.:]+)\\s*(-?[\\d\\.:]+)\\s*(-?[\\d\\.:]+)");
		const int pos = rx.indexIn(fileContent);
		if (pos < 0) {
			//qDebug() << "QtViewerPrintGui::ps2eps(" << filename
			//         << "): cannot find %%BoundingBox";
			return;
		}

		if (! epsfile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			//qDebug() << "QtViewerPrintGui::ps2eps(" << filename
			// << "): cannot open file for writing";
			return;
		}

		const double epsleft = rx.cap(1).toFloat();
		const double epstop = rx.cap(4).toFloat();
		const int left = int(floor(epsleft));
		const int right = int(ceil(epsleft)) + rect.width();
		const int top = int(ceil(epstop)) + 1;
		const int bottom = int(floor(epstop)) - rect.height() + 1;

		fileContent.replace(pos,rx.cap(0).length(),
		                    QString("%%BoundingBox: %1 %2 %3 %4")
		                    .arg(left).arg(bottom).arg(right).arg(top));

		ts << fileContent;
		epsfile.close();
	}

	void QtViewerPrintGui::printToFile(const QString &filename,bool isEPS) {
		// because we want to work with postscript
		// user-coordinates, set to the resolution
		// of the printer (which should be 72dpi here)
		printresolution = 72;
		QPrinter *printer;

		if (isEPS == false) {
			printer = new QPrinter(QPrinter::PrinterResolution);
		} else {
			printer = new QPrinter(QPrinter::ScreenResolution);
		}

		printer->setOutputFileName(filename);
		printer->setColorMode(QPrinter::Color);

		QPainter *painter = new QPainter(printer);
		printer->setResolution(printresolution);

		pDP->hold();
		QPixmap * mp = pDP->contents();

		QRect rect = painter->viewport();
		//it may be necessary to remove preset left and top by translate
		rect.adjust(72, 72, -72, -72);
		QSize size = mp->size();
		size.scale(rect.size(), Qt::KeepAspectRatio);
		painter->setViewport(rect.x(), rect.y(), size.width(), size.height());
		painter->setWindow(mp->rect());
		painter->drawPixmap(0, 0, *mp);
		pDP->release();
		delete mp;
		mp=0;
		painter->end();
		int resolution = printer->resolution();

		delete painter;
		delete printer;
		if (isEPS) {
			rect.setWidth( int(ceil(rect.width() * 72.0/resolution)) );
			rect.setHeight( int(ceil(rect.height() * 72.0/resolution)) );
			ps2eps(filename,rect);
		}
	}

	void QtViewerPrintGui::dpResized(QSize /*panelSize*/, QSize canvasSize) {
		stringstream ss;
		ss << "Current Canvas Size: " << canvasSize.width() << " x ";
		ss << canvasSize.height() << " pixels.";
		sizeLabel->setText(ss.str().c_str());
	}

	ostream& operator << (ostream &os, const QtViewerPrintGui &obj) {
		os << "print parameters: printfilename="
		   << obj.printfilename.toStdString()
		   <<  " printmedia="
		   <<  obj.printmedia.toStdString()
		   << " printorientation="
		   <<  obj.printorientation.toStdString()
		   << " printresolution="
		   << obj.printresolution
		   << " printmagnification="
		   << obj.printmagnification
		   //<< " printepsforma="
		   //<< obj.printepsformat.toStdString()
		   << " printername="
		   << obj.printerName().toStdString()
		   << endl;
		return os;
	}

	ostream& operator << (ostream &os, const QPrinter &printer) {
		os << " creator=" << printer.creator().toStdString()
		   << " docName=" << printer.docName().toStdString()
		   //<< " fromPage=" << printer.fromPage().toStdString() //4.1
		   << " fullPage=" << printer.fullPage()
		   //<< " newPage=" << printer.newPage()
		   << " numCopies=" << printer.numCopies()
		   << " orientation=" << printer.orientation ()
		   << " outputFileName=" << printer.outputFileName().toStdString()
		   //<< " outputFormat=" << printer.outputFormat() //4.1
		   << " pageOrder=" << printer.pageOrder()
		   << " pageRect=" << printer.pageRect().x() << "," << printer.pageRect().y() << ","
		   << printer.pageRect().width() << "," << printer.pageRect().height()
		   << " pageSize=" << printer.pageSize()
		   << " paperRect=" << printer.paperRect().x()  << "," << printer.paperRect().y() << ","
		   << printer.paperRect().width() << "," << printer.paperRect().height()
		   << " paperSource=" << printer.paperSource()
		   << " printProgram=" << printer.printProgram().toStdString()
		   //<< " printRange=" << printer.printRange() //4.1
		   << " printerName=" << printer.printerName().toStdString()
		   << " printerSelectionOption=" << printer.printerSelectionOption().toStdString()
		   << " printerState=" << printer.printerState()
		   << endl;
		return os;
	}

	const QString QtViewerPrintGui::printRecord =
	    "<casa-Record>\n"
	    "<printfilename "
	    "dlformat=\"printfilename\" "
	    "listname=\"Output file\" "
	    "ptype=\"string\" "
	    "default=\"unset\" "
	    "value=\"unset\" "
	    "allowunset=\"T\" "
	    "autoapply=\"F\" "
	    "/>\n"
	    "<printmedia "
	    "dlformat=\"printmedia\" "
	    "listname=\"[PS/PDF] Output media\" "
	    "ptype=\"choice\" "
	    "popt=\"[LETTER, A4]\" "
	    "default=\"A4\" "
	    "value=\"A4\" "
	    "allowunset=\"F\" "
	    "autoapply=\"F\" "
	    "/>\n"
	    "<printorientation "
	    "dlformat=\"printorientation\" "
	    "listname=\"[PS/PDF] Orientation\" "
	    "ptype=\"choice\" "
	    "popt=\"[portrait, landscape]\" "
	    "default=\"portrait\" "
	    "value=\"portrait\" "
	    "allowunset=\"F\" "
	    "autoapply=\"F\" "
	    "/>\n"
	    "<printresolution "
	    "dlformat=\"printresolution\" "
	    "listname=\"[PS/PDF] Resolution (dpi)\" "
	    "ptype=\"intrange\" "
	    "pmin=\"72\" "
	    "pmax=\"600\" "
	    "default=\"150\" "
	    "value=\"150\" "
	    "allowunset=\"F\" "
	    "autoapply=\"F\" "
	    "/>\n"
	    "<imgresolution "
	    "dlformat=\"imgresolution\" "
	    "listname=\"[Image] Output size (pixels)\" "
	    "ptype=\"intpair\" "
	    "pmin=\"-1\" "
	    "pmax=\"99999\" "
	    "value=\"-1 -1\" "
	    "default=\"-1 -1\" "
	    "separator=\"x\" "
	    "help=\"Specify the width and height of the image to save, or -1 for "
	    "screen resolution.\""
	    "allowunset=\"F\" "
	    "autoapply=\"F\" "
	    "/>\n"
	    "<imgsizefactor "
	    "dlformat=\"imgsizefactor\" "
	    "listname=\"[Image] Output size multiplicative factor\" "
	    "ptype=\"double\" "
	    "pmin=\"0.01\" "
	    "pmax=\"99999\" "
	    "value=\"1\" "
	    "default=\"1\" "
	    "help=\"Multiplicative factor for the output size.\" "
	    "allowunset=\"F\" "
	    "autoapply=\"F\" "
	    "/>\n"
	    //"<printmagnification "
	    //"dlformat=\"printmagnification\" "
	    //"listname=\"[PS/PDF] Magnification\" "
	    //"ptype=\"floatrange\" "
	    //"pmin=\"0.1\" "
	    //"pmax=\"1.0\" "
	    //"presolution=\"0.02\" "
	    //"default=\"1.0\" "
	    //"value=\"1.0\" "
	    //"allowunset=\"F\" "
	    //"autoapply=\"F\" "
	    //"/>\n"
	    //"<printepsformat "
	    //"dlformat=\"printepsformat\" "
	    //"listname=\"[PS/PDF] Write EPS format?\" "
	    //"ptype=\"boolean\" "
	    //"default=\"F\" "
	    //"value=\"F\" "
	    //"allowunset=\"F\" "
	    //"autoapply=\"F\" "
	    //"/>\n"
	    "</casa-Record>";


	QtViewerPrintCtl::QtViewerPrintCtl(QtViewerPrintGui *parent)
		: QDialog(0) {
		setupUi(this);

		QSettings settings("CASA", "Viewer");

		cbOrientation->addItem("Portrait");
		cbOrientation->addItem("Landscape");
		//cbOrientation->addItem("2-Up");

		QString media = settings.value("Print/paper").toString();
		QStringList paperType;
		paperType << "US Letter" <<  "A4" << "A3";
		for (int i = 0; i < paperType.size(); i++) {
			QString item = paperType.at(i).toLocal8Bit().constData();
			cbPaper->addItem(item);
			if (item == media)
				cbPaper->setCurrentIndex(i);
		}

		rbPrinter->setChecked(true);

		QString name = parent->printFileName();
		//cout << "parent=" <<  (*parent);
		if (name.isEmpty() || name == "unset") {
			QDateTime qdt = QDateTime::currentDateTime();
			name = "viewer-" + qdt.toString(Qt::ISODate) + ".ps";
		}
		lPrinter->setText("");
		rbPrinter->setText("Printer Name: ");

		leFileName->setText(name);
		lePrinter->setText(settings.value("Print/printer").toString());

		QObject::connect(bPrint, SIGNAL(clicked()), this, SLOT(checkPrinter()));
		QObject::connect(this, SIGNAL(printIt()), this, SLOT(accept()));

	}

	QtViewerPrintCtl::~QtViewerPrintCtl()
	{}

	QString QtViewerPrintCtl::fileName() const {
		return leFileName->text();
	}

	QString QtViewerPrintCtl::printerName() const {
		if (rbPrinter->isChecked())
			return lePrinter->text();
		else
			return "Ghostview";
	}

	QString QtViewerPrintCtl::orientation() const {
		return cbOrientation->currentText();
	}

	QString QtViewerPrintCtl::paper() const {
		return cbPaper->currentText();
	}

	void QtViewerPrintCtl::checkPrinter() {
		if (lePrinter->text().isEmpty()) {
			//lePrinter->setBackgroundRole(QPalette::Highlight);
			QPalette palette( lePrinter->palette() );
			palette.setColor( QPalette::Base, Qt::red);
			lePrinter->setPalette(palette);
		} else {
			QSettings settings("CASA", "Viewer");
			settings.setValue("Print/paper",  cbPaper->currentText());
			settings.setValue("Print/printer", lePrinter->text());
			emit printIt();
		}
	}

} //# NAMESPACE CASA - END

