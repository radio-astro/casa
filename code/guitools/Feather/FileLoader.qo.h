#ifndef FILELOADER_QO_H
#define FILELOADER_QO_H

#include <QtGui/QDialog>
#include <guitools/Feather/FileLoader.ui.h>

class QFileSystemModel;
class QKeyEvent;

namespace casa {

class FileLoader : public QDialog
{
    Q_OBJECT

public:
    FileLoader(QWidget *parent = 0);
    ~FileLoader();
    QString getFilePathLowResolution() const;
    QString getFilePathHighResolution() const;
    QString getFilePathOutput() const;
    bool isOutputSaved() const;

signals:
	void imageFilesChanged();

public slots:
	void saveStateChanged( int checked );

protected:
    void keyPressEvent( QKeyEvent* event );

private slots:
	void filesChanged();
	void filesReset();
	void directoryChanged(const QModelIndex& modelIndex );
	void fileLowResolutionChanged();
	void fileHighResolutionChanged();
	void outputDirectoryChanged();
	void validateDirectory( const QString& str );

private:
	bool validatePath( QLineEdit* lineEdit, const QString& errorPrefix, bool file, QString& destination );
	void fileChanged( QLineEdit* destinationLineEdit, const QString& emptyWarning, bool directory );
	QString lowResolutionImageFile;
	QString highResolutionImageFile;
	QString outputFile;
	QString outputDirectory;
	QFileSystemModel* fileModel;
	bool saveOutput;
    Ui::FileLoaderClass ui;
};

}

#endif // FILELOADER_QO_H
