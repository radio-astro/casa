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
#include "FileLoader.qo.h"
#include <QFileSystemModel>
#include <QMessageBox>
#include <QDebug>
#include <QKeyEvent>

namespace casa {

FileLoader::FileLoader(QWidget *parent)
    : QDialog(parent), saveOutput( true ){

	ui.setupUi(this);
	this->setWindowTitle( "Feather Image Loader");

	//Initialize the file browsing tree
	fileModel = new QFileSystemModel( ui.treeWidget );
	QString initialDir = QDir::currentPath();
	ui.treeWidget->setModel( fileModel );
	QString rootDir = QDir::rootPath();
	fileModel->setRootPath(rootDir );
	QModelIndex initialIndex = fileModel->index( initialDir );
	ui.treeWidget->setCurrentIndex( initialIndex );
	ui.treeWidget->setColumnHidden( 1, true );
	ui.treeWidget->setColumnHidden( 2, true );
	ui.treeWidget->setColumnHidden( 3, true );
	ui.directoryLineEdit->setText( initialDir );
	connect( ui.treeWidget, SIGNAL(clicked(const QModelIndex&)), this, SLOT(directoryChanged(const QModelIndex&)));
	connect( ui.directoryLineEdit, SIGNAL(textEdited(const QString&)), this, SLOT(validateDirectory(const QString&)));

	connect( ui.saveOutputCheckBox, SIGNAL(stateChanged(int)), this, SLOT(saveStateChanged(int)));

	connect( ui.lowResButton, SIGNAL(clicked()), this, SLOT(fileLowResolutionChanged()));
	connect( ui.highResButton, SIGNAL(clicked()), this, SLOT(fileHighResolutionChanged()));
	connect( ui.outputButton, SIGNAL(clicked()), this, SLOT( outputDirectoryChanged()));
	connect( ui.dirtyImageButton, SIGNAL(clicked()), this, SLOT( dirtyImageChanged()));


	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(filesChanged()));
	connect( ui.cancelButton, SIGNAL( clicked()), this, SLOT(filesReset()));
	ui.saveOutputCheckBox->setChecked( false );

	//Basic images for testing
	/*ui.lowResolutionLineEdit->setText("/home/uniblab/casa/trunk/test/orion_gbt.im" );
	ui.highResolutionLineEdit->setText("/home/uniblab/casa/trunk/test/orion_vlamem.im");
	ui.dirtyImageLineEdit->setText("/home/uniblab/casa/trunk/test/orion_vlamem.im2");
*/

	//Multiplane images for testing
	/*ui.lowResolutionLineEdit->setText("/home/uniblab/casa/trunk/test/featherMultiPlane/n4826_bima.im" );
	ui.highResolutionLineEdit->setText("/home/uniblab/casa/trunk/test/featherMultiPlane/n4826_t12m_regrid.im");
	*/

	/*ui.lowResolutionLineEdit->setText("/lustre/aoc/users/jott/susan/gbt43.im_stokes" );
	ui.highResolutionLineEdit->setText("/lustre/aoc/users/jott/susan/vla43.im_stokes");
	ui.dirtyImageLineEdit->setText("/lustre/aoc/users/jott/susan/vla43.im_stokes");*/

	//VLA+GBT Data N49 from D. Shepherd
	/*ui.lowResolutionLineEdit->setText("/home/uniblab/casa/trunk/test/Feather-n49/new_gbt_jyMask.image" );
	ui.highResolutionLineEdit->setText("/home/uniblab/casa/trunk/test/Feather-n49/n49combo-8final-pbcor-trans.image");
	ui.dirtyImageLineEdit->setText("/home/uniblab/casa/trunk/test/Feather-n49/n49comboDirty.image");
	ui.saveOutputCheckBox->setChecked( true );
	ui.outputImageDirectoryLineEdit->setText( "/users/slovelan/tmp");
	ui.outputImageFileLineEdit->setText( "output.image");
*/

	//Simulated ALMA Data
	/*ui.lowResolutionLineEdit->setText("/home/uniblab/casa/trunk/test/FeatherALMASIM/m51faintBestStableNew.aca.tp.noisy.sf.sd.image" );
	ui.highResolutionLineEdit->setText("/home/uniblab/casa/trunk/test/FeatherALMASIM/m51faintBestStableNew.12m7m.noisy.cleaned.image");
	ui.dirtyImageLineEdit->setText("/home/uniblab/casa/trunk/test/FeatherALMASIM/m51faintBestStableNew.12m7m.noisy.dirty.image");
	ui.saveOutputCheckBox->setChecked( true );
	ui.outputImageDirectoryLineEdit->setText( "/users/slovelan/tmp");
	ui.outputImageFileLineEdit->setText( "output.image");*/

	//Real ALMA Data (M100 Science Verification data)
	//ui.lowResolutionLineEdit->setText("/home/uniblab/casa/trunk/test/Feather-M100/TP_CO_cube_JyOldGjinc" );
	//ui.highResolutionLineEdit->setText("/home/uniblab/casa/trunk/test/Feather-M100/M100_Intcombo_0.193_cube.image");

	//Non-stokes Axis low res
	//ui.lowResolutionLineEdit->setText("/home/uniblab/casa/trunk/test/Feather43/gbt43Jy.im");
	//ui.highResolutionLineEdit->setText("/home/uniblab/casa/trunk/test/Feather43/vla43.im");

	//Stable 4.2 Testing
	//ui.lowResolutionLineEdit->setText("/home/uniblab/casa/trunk/test/FeatherJuergan/gbt43Jy.im");
	//ui.highResolutionLineEdit->setText("/home/uniblab/casa/trunk/test/FeatherJuergan/vla43.im");
	//ui.dirtyImageLineEdit->setText("/home/uniblab/casa/trunk/test/FeatherJuergan/dirty.im");
}



void FileLoader::keyPressEvent( QKeyEvent* event ){
	int keyCode = event->key();
	//This was written here because pressing a return on a line edit inside
	//the dialog was closing the dialog.
	if ( keyCode != Qt::Key_Return ){
		QDialog::keyPressEvent( event );
	}
}


void FileLoader::validateDirectory( const QString& str ){
	QFile file( str );
	bool valid = file.exists();
	if ( valid ){
		QAbstractItemModel* model = ui.treeWidget->model();
		QFileSystemModel* fileModel = dynamic_cast<QFileSystemModel*>(model);
		QModelIndex pathIndex = fileModel->index( str );
		ui.treeWidget->setCurrentIndex( pathIndex );
	}
}

void FileLoader::saveStateChanged( int checked ){
	ui.outputButton->setEnabled( checked );
	ui.outputImageDirectoryLineEdit->setText("");
	ui.outputImageDirectoryLineEdit->setEnabled( checked );
	ui.outputImageFileLineEdit->setText("");
	ui.outputImageFileLineEdit->setEnabled( checked );
}

bool FileLoader::isOutputSaved() const {
	return saveOutput;
}

QString FileLoader::getFilePathLowResolution() const {
	return lowResolutionImageFile;
}

QString FileLoader::getFilePathHighResolution() const {
	return highResolutionImageFile;
}

QString FileLoader::getFilePathOutput() const {
	return outputDirectory + outputFile;
}

QString FileLoader::getFileDirty() const{
	return dirtyImageFile;
}

void FileLoader::directoryChanged(const QModelIndex& modelIndex ){
	QString path = fileModel->filePath( modelIndex );
	//ui.treeWidget->setCurrentIndex( modelIndex );
	ui.directoryLineEdit->setText( path );
}

void FileLoader::dirtyImageChanged(){
	QString emptyWarning;
	fileChanged( ui.dirtyImageLineEdit, emptyWarning, false );
}


void FileLoader::fileHighResolutionChanged(){
	QString emptyWarning( "Please select a high resolution image file.");
	fileChanged( ui.highResolutionLineEdit, emptyWarning, false );
}

void FileLoader::fileLowResolutionChanged(){
	QString emptyWarning( "Please select a low resolution image file.");
	fileChanged( ui.lowResolutionLineEdit, emptyWarning, false );
}

void FileLoader::outputDirectoryChanged(){
	QString emptyWarning( "Please select a directory for the output image file.");
	fileChanged( ui.outputImageDirectoryLineEdit, emptyWarning, true );
}

void FileLoader::fileChanged( QLineEdit* destinationLineEdit,
		const QString& emptyWarning, bool directory ){
	QModelIndex currentIndex = ui.treeWidget->currentIndex();
	bool validPath = currentIndex.isValid();
	QString path;
	if ( validPath ){
		path = fileModel->filePath( currentIndex );
		if ( directory ){
			QDir directoryTest( path );
			if ( ! directoryTest.exists() ){
				validPath = false;
			}
		}
	}

	if ( validPath ){
		destinationLineEdit->setText( path );
	}
	else {
		if ( emptyWarning.length() > 0 ){
			QMessageBox::warning( this, "", emptyWarning );
		}
	}
}


bool FileLoader::validatePath( QLineEdit* lineEdit, const QString& errorPrefix, bool file, QString& destination ){
	QString filePath = lineEdit->text();
	QFile imageFile( filePath );
	bool valid = imageFile.exists();
	if ( !file ){
		QDir imageDir( filePath );
		valid = imageDir.exists();
	}
	if ( !valid ){
		QString warningMsg = errorPrefix + filePath + " is not valid.";
		QMessageBox::warning( this, "Invalid Path", warningMsg );
	}
	else {
		destination = filePath;
	}
	return valid;
}

void FileLoader::filesChanged(){
	saveOutput = ui.saveOutputCheckBox->isChecked();
	bool validLowRes = validatePath( ui.lowResolutionLineEdit, "Low resolution image file: ", true, lowResolutionImageFile );
	bool validHighRes = validatePath( ui.highResolutionLineEdit, "High resolution image file: ", true, highResolutionImageFile  );
	//The dirty image is optional, we only validate it if there is something
	//there.
	bool validDirty = true;
	QString dirtyPath = ui.dirtyImageLineEdit->text().trimmed();
	if ( dirtyPath.length() > 0 ){
		validDirty = validatePath( ui.dirtyImageLineEdit, "Dirty image file: ", true, dirtyImageFile );
	}
	else {
		dirtyImageFile="";
	}
	//Output image is also optional.  We only validate it if the user has
	//checked the box for saving output.
	bool validOutput = true;
	if ( saveOutput ){
		validOutput = validatePath( ui.outputImageDirectoryLineEdit, "Output image directory: ", false, outputDirectory );
		QString outputFileName = ui.outputImageFileLineEdit->text();
		if ( validOutput ){
			outputDirectory = ui.outputImageDirectoryLineEdit->text();
			if ( !outputDirectory.endsWith( QDir::separator())){
				outputDirectory.append( QDir::separator());
			}

			if (outputFileName.trimmed().length() == 0 ){
				validOutput = false;
				QString errorMsg( "Please specify an output file name.");
				QMessageBox::warning( this, "Invalid File Path", errorMsg );
			}
			else {
				outputFile = ui.outputImageFileLineEdit->text();
			}
		}
		else if ( outputFileName.trimmed().length() > 0 ){
			outputFile = ui.outputImageFileLineEdit->text();
		}
	}

	//Close the dialog if everything is valid and indicate the change.
	if ( validLowRes && validHighRes && validOutput && validDirty ){
		emit imageFilesChanged();
		this->close();
	}
}

void FileLoader::updateOutput(const QString& directory, const QString& file ){
	outputDirectory = directory;
	outputFile = file;
	ui.outputImageFileLineEdit->setText( outputFile );
	ui.outputImageDirectoryLineEdit->setText( outputDirectory );
}

void FileLoader::filesReset(){
	ui.lowResolutionLineEdit->setText( lowResolutionImageFile );
	ui.highResolutionLineEdit->setText( highResolutionImageFile );
	ui.outputImageFileLineEdit->setText( outputFile );
	ui.dirtyImageLineEdit->setText(dirtyImageFile );
	ui.outputImageDirectoryLineEdit->setText( outputDirectory );
	ui.saveOutputCheckBox->setChecked( saveOutput );
	this->close();
}

FileLoader::~FileLoader()
{

}
}
