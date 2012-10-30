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
	qDebug() << "Setting directory to "<<initialDir;
	ui.treeWidget->setModel( fileModel );
	QString rootDir = QDir::rootPath();
	fileModel->setRootPath(rootDir );
	QModelIndex initialIndex = fileModel->index( initialDir );
	qDebug() <<"File index="<<initialIndex.row()<<" col="<<initialIndex.column();
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

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(filesChanged()));
	connect( ui.cancelButton, SIGNAL( clicked()), this, SLOT(filesReset()));
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

void FileLoader::directoryChanged(const QModelIndex& modelIndex ){
	QString path = fileModel->filePath( modelIndex );
	//ui.treeWidget->setCurrentIndex( modelIndex );
	ui.directoryLineEdit->setText( path );
}


void FileLoader::fileHighResolutionChanged(){
	qDebug() << "High resolution file changed";
	QString emptyWarning( "Please select a high resolution image file.");
	fileChanged( ui.highResolutionLineEdit, emptyWarning, false );
}

void FileLoader::fileLowResolutionChanged(){
	qDebug() << "Low resolution file changed.";
	QString emptyWarning( "Please select a low resolution image file.");
	fileChanged( ui.lowResolutionLineEdit, emptyWarning, false );
}

void FileLoader::outputDirectoryChanged(){
	qDebug() << "Ouput directory changed";
	QString emptyWarning( "Please select a directory for the output image file.");
	fileChanged( ui.outputImageDirectoryLineEdit, emptyWarning, true );
}

void FileLoader::fileChanged( QLineEdit* destinationLineEdit,
		const QString& emptyWarning, bool directory ){
	QModelIndex currentIndex = ui.treeWidget->currentIndex();
	qDebug() << "Current index valid="<<currentIndex.isValid();
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
		qDebug() << "Path "<<path;
		destinationLineEdit->setText( path );
	}
	else {
		QMessageBox::warning( this, "", emptyWarning );
	}
}


bool FileLoader::validatePath( QLineEdit* lineEdit, const QString& errorPrefix, bool file, QString& destination ){
	QString filePath = lineEdit->text();
	QFile imageFile( filePath );
	bool valid = imageFile.exists();
	if ( !file ){
		QDir imageDir( filePath );
		valid = imageDir.exists();
		qDebug() << "Image directory valid="<<valid<<" for path "<<filePath;
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
	bool validOutput = true;
	if ( saveOutput ){
		validOutput = validatePath( ui.outputImageDirectoryLineEdit, "Output image directory: ", false, outputDirectory );
		QString outputFileName = ui.outputImageFileLineEdit->text();
		if (outputFileName.trimmed().length() == 0 ){
			validOutput = false;
			QString errorMsg( "Please specify an output file name.");
			QMessageBox::warning( this, "Invalid File Path", errorMsg );
		}
		else {
			outputFile = ui.outputImageFileLineEdit->text();
		}
	}
	else {
		outputFile = ui.outputImageFileLineEdit->text();
		outputDirectory = ui.outputImageDirectoryLineEdit->text();
	}

	//Close the dialog if everything is valid and indicate the change.
	if ( validLowRes && validHighRes && validOutput ){
		emit imageFilesChanged();
		this->close();
	}
}

void FileLoader::filesReset(){
	ui.lowResolutionLineEdit->setText( lowResolutionImageFile );
	ui.highResolutionLineEdit->setText( highResolutionImageFile );
	ui.outputImageFileLineEdit->setText( outputFile );
	ui.outputImageDirectoryLineEdit->setText( outputDirectory );
	ui.saveOutputCheckBox->setChecked( saveOutput );
	this->close();
}

FileLoader::~FileLoader()
{

}
}
