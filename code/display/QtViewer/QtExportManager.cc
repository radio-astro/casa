//# QtDataManager.cc: Qt implementation of viewer data manager widget.
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


#include <display/QtViewer/QtExportManager.qo.h>
#include <display/QtViewer/QtViewer.qo.h>
#include <display/QtViewer/QtExportBrowser.qo.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
//#include <tables/Tables/Table.h>
//#include <tables/Tables/TableInfo.h>
#include <casa/BasicSL/String.h>
#include <casa/OS/File.h>
#include <casa/iostream.h>
#include <casa/fstream.h>
#include <casa/Exceptions/Error.h>
#include <images/Images/ImageFITSConverter.h>
#include <images/Images/FITSImage.h>
#include <synthesis/TransformMachines/Utils.h>

#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QMessageBox>
#include <QDirModel>
#include <QHash>
#include <QSettings>
#include <QTextStream>
#include <graphics/X11/X_exit.h>


namespace casa { //# NAMESPACE CASA - BEGIN



	QtExportManager::QtExportManager(QtDisplayPanelGui* panel,
	                                 const char *name,
	                                 QWidget *parent ) :
		QWidget(parent),
		parent_(parent),
		panel_(panel),
		selItem_(0),
		saveType_(-1) {

		setWindowTitle(name);

		setupUi(this);

		dataType_[UNKNOWN]              = String("unknown");
		dataType_[IMAGE]                = String("image");
		//dataType_[QUALIMG]              = String("image");
		dataType_[MEASUREMENT_SET]      = String("ms");
		dataType_[SKY_CATALOG]          = String("skycatalog");
		dataType_[RESTORE]              = String("restore");
		dataType_[CASAREGION]           = String("casa region");
		dataType_[DS9REGION]           = String("ds9 region");

		uiDataType_["unknown"]          = UNKNOWN;
		uiDataType_["image"]            = IMAGE;
		uiDataType_["measurement Set"]  = MEASUREMENT_SET;
		uiDataType_["sky catalog"]      = SKY_CATALOG;
		//uiDataType_["Directory"]        = DIRECTORY;
		//uiDataType_["FITS Image"]       = IMAGE;
		//uiDataType_["FITS Ext."]        = IMAGE;
		//uiDataType_["Quality Ext."]     = QUALIMG;
		//uiDataType_["Miriad Image"]     = IMAGE;
		//uiDataType_["Gipsy"]            = IMAGE;
		uiDataType_["restore file"]     = RESTORE;
		uiDataType_["CASA region file"] = CASAREGION;
		uiDataType_["DS9 region file"] = DS9REGION;

		uiDisplayType_["raster image"]  = RASTER;
		uiDisplayType_["contour map"]   = CONTOUR;
		uiDisplayType_["vector map"]    = VECTOR;
		uiDisplayType_["marker map"]    = MARKER;
		uiDisplayType_["sky catalog"]   = SKY_CAT;
		uiDisplayType_["old window"]    = OLDPANEL;
		uiDisplayType_["new window"]    = NEWPANEL;

		displayType_[RASTER]    = String("raster");
		displayType_[CONTOUR]   = String("contour");
		displayType_[VECTOR]    = String("vector");
		displayType_[MARKER]    = String("marker");
		displayType_[SKY_CAT]   = String("skycatalog");
		displayType_[OLDPANEL]  = String("oldpanel");
		displayType_[NEWPANEL]  = String("newpanel");

		// define the accepted export types
		exportTypes_ = QVector<int>(1, IMAGE);

		leaveOpen_->setToolTip("Uncheck to close this window after "
		                       "data and export action.\n"
		                       "Use 'Save as...' button/menu on Display Panel to show it again.");
		leaveOpen_->setChecked(false);

		dir_=QDir(QDir::current());
		dir_.setFilter(QDir::AllDirs | //QDir::NoSymLinks |
		               QDir::Files);
		dir_.setSorting(QDir::Name);

		connect(updateButton_, SIGNAL(clicked()), SLOT(buildFileList()));
		connect(dirLineEdit_,  SIGNAL(returnPressed()), SLOT(exportData()));
		connect(saveButton_,   SIGNAL(clicked()), SLOT(exportData()));
		connect(treeWidget_,   SIGNAL(itemSelectionChanged()), SLOT(changeItemSelection()));
		connect(browseButton_, SIGNAL(clicked()), this, SLOT(browseOutFilePath()));
	}


	QtExportManager::~QtExportManager() {
	}

	void QtExportManager::buildFileList() {
		treeWidget_->clear();
		qddList_.clear();

		QStringList lbl;
		lbl << "Name" << "Type" << "Displaytype";
		treeWidget_->setColumnCount(3);
		treeWidget_->setHeaderLabels(lbl);

		QTreeWidgetItem *fileItem;
		QtDisplayData* qdd;

		// retrieve a copy of the current DD list.
		//List<QtDisplayData*> qdds_ = panel_->dds();

		// iterate over all list members
		int type, dtype, qdd_index=0;
		//for(ListIter<QtDisplayData*> qdds(qdds_); !qdds.atEnd(); qdds++) {
		//qdd=qdds.getRight();
		DisplayDataHolder::DisplayDataIterator iter = panel_->beginDD();
		while ( iter != panel_->endDD()) {
			qdd = (*iter);
			iter++;

			type    = dataType_.key(qdd->dataType());
			if (exportTypes_.contains(type)) {
				dtype  = displayType_.key(qdd->displayType());

				QString fileName    = QString((qdd->name()).c_str());
				QString filePath    = QString((qdd->path()).c_str());

				if (filePath.endsWith("]") && filePath.lastIndexOf(".fits[") >0) {
					int lIndex=filePath.lastIndexOf(".fits[");
					filePath.replace(lIndex, filePath.size()-lIndex, ".fits");
				}


				fileItem = new QTreeWidgetItem(treeWidget_);
				fileItem->setText(0, fileName);
				fileItem->setToolTip(0, filePath);
				fileItem->setText(1, uiDataType_.key(type));
				fileItem->setText(2, uiDisplayType_.key(dtype));
				fileItem->setTextColor(1, getDirColor(type));

				qddList_[qdd_index]=qdd;
				qdd_index++;
			}
		}
		treeWidget_->resizeColumnToContents(0);

		// activate the last entry
		if (treeWidget_->topLevelItemCount()>0) {
			treeWidget_->setCurrentItem (treeWidget_->topLevelItem(treeWidget_->topLevelItemCount()-1));
		}
	}

	void QtExportManager::changeItemSelection() {
		//qDebug() << "treeWidget selection changed!";
		browseButton_->setEnabled(true);
		QList<QTreeWidgetItem *> lst = treeWidget_->selectedItems();
		if (!lst.empty()) {
			QTreeWidgetItem *item = (QTreeWidgetItem*)(lst.at(0));
			if (item!=selItem_) {
				selItem_ = item;
			}
			dirLineEdit_->setText(dir_.path()+"/"+selItem_->text(0));
			fillFormatBox(uiDataType_.value(selItem_->text(1)));
		} else {
			selItem_=0;
		}
		//qDebug() << "number of selected items: " << lst.size();
	}

	void QtExportManager::browseOutFilePath() {
		QString outFilePath;

		// make the dialog an add its properties
		QtExportBrowser browseDialog(this, dirLineEdit_->text());
		int ret = browseDialog.exec();
		if (ret==QDialog::Accepted) {
			outFilePath=browseDialog.exportFilePath();
		} else if (ret==QDialog::Rejected) {
			outFilePath.clear();
			return ;
		}

		// store the filename and start the export
		dirLineEdit_->setText(outFilePath);
		exportData();
	}

	void QtExportManager::browseFileSelection() {
		QString fileName;
		QString filter;

		switch (saveType_) {
		case IMAGE: {
			if (formatBox_->currentText().contains("FITS")) {
				filter=QString("FITS images (*.fits);;all files (*)");
				fileName = getOutFileName(filter);
			} else {
				//filter.clear();
				filter=QString("all files (*.*)");
				fileName = getOutDirName(filter);
			}
			break;
		}
		default:
		{}
		}

		// store the filename and start the export
		if (fileName.isEmpty())
			return ;
		dirLineEdit_->setText(fileName);
		exportData();
	}

	void QtExportManager::browseFileSelectionOLD() {
		QString fn;

		if (formatBox_->currentText().contains("FITS")) {
			QString filter=QString("FITS images (*.fits);;all files (*)");
			fn = QFileDialog::getSaveFileName(this, tr("Export file as image"), dirLineEdit_->text(), filter, 0, QFileDialog::DontConfirmOverwrite);
		} else {
			QFileDialog dirDialog(this);
			dirDialog.setFileMode(QFileDialog::Directory);
			dirDialog.setWindowTitle(this->windowTitle());
			dirDialog.setDirectory(dir_.path());
			QStringList fileNames;
			if (dirDialog.exec())
				fileNames = dirDialog.selectedFiles();
			if (fileNames.size()>0)
				fn = fileNames.at(0);
			else
				fn.clear();
		}

		// store the filename and start the export
		if (fn.isEmpty())
			return ;
		dirLineEdit_->setText(fn);
		exportData();
	}

	QString QtExportManager::getOutFileName(QString &filter) {
		QString fileName;

		// all in one
		fileName = QFileDialog::getSaveFileName(this, this->windowTitle(), dirLineEdit_->text(), filter, 0,QFileDialog::DontConfirmOverwrite);

		return fileName;
	}

	QString QtExportManager::getOutDirName(QString &) {
		QString dirName;

		// make the dialog an add its properties
		QtExportBrowser browseDialog(this, dirLineEdit_->text());
		int ret = browseDialog.exec();
		if (ret==QDialog::Accepted) {
			//qDebug() << "Got accepted!";
			dirName=browseDialog.exportFilePath();
		} else if (ret==QDialog::Rejected) {
			//qDebug() << "Got rejected!";
			dirName.clear();
		}
		//qDebug() << "dirName: " << dirName;
		return dirName;
	}

	QString QtExportManager::getOutDirNameOLD(QString &filter) {
		QString dirName;

		// make the dialog an add its properties
		QFileDialog dirDialog(this);
		dirDialog.setFileMode(QFileDialog::Directory);
		dirDialog.setWindowTitle(this->windowTitle());
		//dirDialog.setDirectory(dir_.path());
		dirDialog.setDirectory(dirLineEdit_->text());
		//dirDialog.setOption(QFileDialog::DontConfirmOverwrite);
		dirDialog.setAcceptMode(QFileDialog::AcceptSave);
		//dirDialog.setAcceptMode(QFileDialog::AcceptOpen);
		dirDialog.selectNameFilter(filter);

		// get the selection
		QStringList dirNames;
		if (dirDialog.exec())
			dirNames = dirDialog.selectedFiles();
		if (dirNames.size()>0)
			dirName = dirNames.at(0);
		else
			dirName.clear();

		return dirName;
	}

	void QtExportManager::fillFormatBox(int type) {
		if (type!=saveType_) {
			saveType_ = type;
			formatBox_->clear();
			switch (type) {
			case IMAGE :
				formatBox_->addItem("FITS image");
				formatBox_->addItem("CASA image");
				//formatBox_->addItem("MIRIAD image");
				break;
			default:
				formatBox_->addItem("UNKNOWN format");
			}
		}
	}

	QColor QtExportManager::getDirColor(int ddtp) {
		QColor clr;
		switch (ddtp) {
		case IMAGE:
			clr = Qt::darkGreen;
			break;
		case MEASUREMENT_SET:
			clr = Qt::darkBlue;
			break;
		case SKY_CATALOG:
			clr = Qt::darkCyan;
			break;
		case RESTORE:
			clr = QColor(255,43,45);
			break;
		case DIRECTORY:
			clr = Qt::black;
			break;
		case QUALIMG:
			clr = Qt::darkRed;
			break;
		case CASAREGION:
			clr = Qt::darkYellow;
			break;
		case DS9REGION:
			clr = QColor(255,153,51);
			break;
		case UNKNOWN:
		default:
			clr = Qt::darkMagenta;
		}
		return clr;
	}

	void QtExportManager::exportData() {

		// for whatever reasons the treewidget is de-selected
		// when return is pressed on the line editor
		// here we establish the selection again
		QList<QTreeWidgetItem *> lst = treeWidget_->selectedItems();
		if (lst.size()<1 && selItem_) {
			QString oldText=dirLineEdit_->text();
			treeWidget_->setCurrentItem(selItem_);
			dirLineEdit_->setText(oldText);
		}

		// make sure there is a selected item
		if (!selItem_) {
			// it should not get to here
			QString msg = "There is NO selected item in the tree widget! Nothing to be done";
			messageFromEM(msg);
			return;
		}

		// make sure the selected item has a valid index
		int selIndex = treeWidget_->indexOfTopLevelItem(selItem_);
		if (selIndex<0) {
			// it should not get to here
			QString msg = "The selected item has an index <0. Nothing to be done";
			messageFromEM(msg);
			return;
		}

		// make sure the is a DD at the selected index
		QtDisplayData* qdd=qddList_.value(selIndex);
		if (!qdd) {
			// it should not get to here
			QString msg = "The selected does NOT correspond to a display data! Nothing to be done";
			messageFromEM(msg);
			return;
		}

		// get the save path and cut a trailing "/"
		QString lineText = dirLineEdit_->text();
		while (lineText.endsWith("/") && lineText.size()>1)
			lineText.chop(1);

		// make sure that the input and output data are NOT identical;
		// the tooltip has the file path with the extension name
		// stripped off
		QFileInfo fInfo=QFileInfo(lineText);
		QFileInfo fInfoOut=QFileInfo(selItem_->toolTip(0));
		if (fInfoOut==fInfo) {
			QString msg = "Writing the file:\n\"" + fInfoOut.absoluteFilePath() + "\"\n again to:\n\""+ fInfo.absoluteFilePath() + "\"\n does not make sense!";
			messageFromEM(msg);
			return;
		}

		if (fInfo.exists()) {
			// confirm an overwrite over an existing file
			QString msg = fInfo.fileName() + " already exists.\n Do you want to replace it?";
			int ret=QMessageBox::warning(this, this->windowTitle (), msg, QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
			switch (ret) {
			case QMessageBox::Yes:
				break;
			case QMessageBox::No:
				return;
				break;
			default:
				// should never be reached
				return;
				break;
			}
		} else {
			// make sure that writing into the selected directory is possible
			QFileInfo tmpFInfo=QFileInfo(fInfo.absolutePath());
			if (!(tmpFInfo.exists() && tmpFInfo.isDir())) {
				QString msg = "The base directory:\n\"" + fInfo.absolutePath() + "\"\neither does NOT exist or is NOT a directory! Can not write:\n\"" + fInfo.fileName() + "\"\n in there!";
				messageFromEM(msg);
				return;
			} else if (tmpFInfo.exists() && !tmpFInfo.isWritable()) {
				QString msg = "Can not write file:\"" + fInfo.fileName() + " to\n" + fInfo.absolutePath();
				messageFromEM(msg);
				return;
			}
		}

		// set the path and the application path
		dir_.setPath(fInfo.absolutePath());
		dir_.setCurrent(fInfo.absolutePath());

		switch (saveType_) {
		case IMAGE: {
			SHARED_PTR<ImageInterface<Float> > img = qdd->imageInterface();
			if (!img) {
				// it should not get to here
				QString msg = "The selected display does not contain a\n float image. Complex images can NOT be exported!";
				messageFromEM(msg);
				return;
			} else {
				String outFile((fInfo.absoluteFilePath()).toStdString());
				if (formatBox_->currentText().contains("FITS")) {
					expImageInterfaceToFITS(img.get(), outFile);
				} else if (formatBox_->currentText().contains("CASA")) {
					expImageInterfaceToCASA(img.get(), outFile);
				}
				//else if (formatBox_->currentText().contains("MIRIAD")){
				//	expImageInterfaceToMIRIAD(img, outFile);
				//}
				else {
				}
			}
			break;
		}
		default: {
			// it should not get to here
			QString msg = "Can not export the requested data type: "+ saveType_;
			messageFromEM(msg);
		}
		}

		if (leaveOpen_->isChecked())
			return;
		close();
		return;
	}

	void QtExportManager::updateEM(QtDisplayData*,  Bool) {
		buildFileList();
		//dirLineEdit_->setText(dir_.currentPath());
	}

	void QtExportManager::showDDCreateError_(String errMsg) {
		// For now, just send to cerr.  (To do: put this on a status line).
		cerr<<endl<<errMsg<<endl;
	}

	void QtExportManager::expImageInterfaceToFITS(ImageInterface<Float>* img, String outFile) {

		// thats the default values for the call "ImageFITSConverter::ImageToFITS"
		String error;
		uInt memoryInMB(64);
		Bool preferVelocity(True);
		Bool opticalVelocity(True);
		Int BITPIX(-32);
		Float minPix(1.0);
		Float maxPix(-1.0);
		Bool allowOverwrite(False);
		Bool degenerateLast(False);
		Bool verbose(True);
		Bool stokesLast(False);
		Bool preferWavelength(False);
		Bool preferAirWavelength(False);
		String origin("CASA Viewer / FITS export");

		// overwrite was confirmed
		allowOverwrite = True;
		getSectralCoordFlags(img, preferVelocity, opticalVelocity, preferWavelength, preferAirWavelength);

		// overwrite the default "origin" if already
		// in the metadata
		const TableRecord miscInfo=img->miscInfo();
		if (miscInfo.isDefined("origin") && miscInfo.dataType("origin")==TpString) {
			origin = miscInfo.asString(String("origin"));
		}

		// export the image to FITS
		ImageFITSConverter::ImageToFITS(
		    error,
		    *img,
		    outFile,
		    memoryInMB,
		    preferVelocity,
		    opticalVelocity,
		    BITPIX,
		    minPix,
		    maxPix,
		    allowOverwrite,
		    degenerateLast,
		    verbose,
		    stokesLast,
		    preferWavelength,
		    preferAirWavelength,
		    origin
		);

		//
		if (error.size()>0) {
			QString msg = "Error while exporting image: \""+ QString(error.c_str()) + "\"!";
			messageFromEM(msg);
		}
	}

	void QtExportManager::getSectralCoordFlags(const ImageInterface<Float>* img, Bool &preferVelocity, Bool &opticalVelocity, Bool &preferWavelength, Bool &preferAirWavelength) {
		// check for a spectral axis
		if (!(img->coordinates().hasSpectralAxis()))
			return;

		SpectralCoordinate spcCoord = img->coordinates().spectralCoordinate();
		SpectralCoordinate::SpecType spcType=spcCoord.nativeType();

		switch (spcType) {
		case SpectralCoordinate::FREQ:
			preferVelocity      = False;
			opticalVelocity     = False;
			preferWavelength    = False;
			preferAirWavelength = False;
			break;
		case SpectralCoordinate::VRAD:
			preferVelocity      = True;
			opticalVelocity     = False;
			preferWavelength    = False;
			preferAirWavelength = False;
			break;
		case SpectralCoordinate::VOPT:
			preferVelocity      = True;
			opticalVelocity     = True;
			preferWavelength    = False;
			preferAirWavelength = False;
			break;
		case SpectralCoordinate::BETA:
			preferVelocity      = False;
			opticalVelocity     = False;
			preferWavelength    = False;
			preferAirWavelength = False;
			break;
		case SpectralCoordinate::WAVE:
			preferVelocity      = False;
			opticalVelocity     = False;
			preferWavelength    = True;
			preferAirWavelength = False;
			break;
		case SpectralCoordinate::AWAV:
			preferVelocity      = False;
			opticalVelocity     = False;
			preferWavelength    = True;
			preferAirWavelength = True;
			break;
		default:
			preferVelocity      = False;
			opticalVelocity     = False;
			preferWavelength    = False;
			preferAirWavelength = False;
		}

	}

	void QtExportManager::expImageInterfaceToCASA(ImageInterface<Float> *img, String outFile) {
		try {
			// from: synthesis/TransformMachines/Utils.h
			// still to be copied over:
			// regions, history??
			PagedImage<Float> newPagedImage(img->shape(), img->coordinates(), outFile);
			LatticeExpr<Float> le(*img);
			newPagedImage.copyData(le);

			// check for a mask
			if (img->isMasked()) {
				// get the default mask name
				const String maskName = img->getDefaultMask();

				// create a mask in the output image
				if (maskName.size()>0)
					newPagedImage.makeMask(maskName, True, True);
				else
					newPagedImage.makeMask("default", True, True);

				// copy the mask over
				(newPagedImage.pixelMask()).copyData(img->pixelMask());
				//img->pixelMask().copyDataTo(newPagedImage.pixelMask());
			}

			// copy ImageInfo and MiscInfo
			newPagedImage.setImageInfo(img->imageInfo());
			newPagedImage.setMiscInfo(img->miscInfo());

		} catch (AipsError x) {
			QString msg = "Error: " + QString((x.getMesg()).c_str());
			messageFromEM(msg);
		}
	}
//void QtExportManager::expImageInterfaceToMIRIAD(ImageInterface<Float>* img, String outFile){
//	qDebug() << "Writing MIRIAD image: " << QString(outFile.c_str());
//}

	void QtExportManager::messageFromEM(QString &msg) {
		QMessageBox qMsg(this);
		qMsg.setText(msg);
		qMsg.exec();
	}

} //# NAMESPACE CASA - END
