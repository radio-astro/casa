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
//# $Id: $


#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#if ! defined(__APPLE__) || defined(__USE_WS_X11__)
#include <graphics/X11/X_enter.h>
#endif

#include <fstream>
#include <QMainWindow>
#include <QMap>
#include <QModelIndex>
#include <QPointer>
#include <QMutex>
#include <QTime>
#include <QFileSystemWatcher>
#include <QTextStream>


class QAction;
class QComboBox;
class QLineEdit;
class QTreeView;
class QTreeWidget;
class QMenu;
class QFile;
class QSortFilterProxyModel;
class QCheckBox;
class QLabel;

#if ! defined(__APPLE__) || defined(__USE_WS_X11__)
#include <graphics/X11/X_exit.h>
#endif

namespace casa {

class LogModel;
class FileWatcher;

class LogViewer : public QMainWindow
{
    Q_OBJECT

public:
    LogViewer(QString logger = 0, QWidget *parent = 0);

protected:
    virtual void closeEvent(QCloseEvent *e);
    virtual void moveEvent(QMoveEvent *e);

private:
    void setupFileActions();
    void setupEditActions();
    void setupInsertActions();
    void updateMenus();
    bool load(const QString &f);
    bool maybeSave();

private slots:
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void filePrint();
    void filePrintPdf();
    void fileChanged(const QString&);
    void logOpen();
    
    void search();
    void findNext();
    void remove();
    void purge();
    void copy();
    void insert();
    void changeFilterText();
    void changeFilterColumn(int);
    void doFilter();
    void refresh();
    void reload();
    void hide();
    void changeFont();

    void clicked(const QModelIndex &index);

    virtual QSize sizeHint () const;

private:

    QAction *actionOpen,
        *actionSave,
        *actionSaveAs,
        *actionSearch,
        *actionNext,
        *actionFilter,
        *actionInsert,
        *actionCopy,
        *actionDelete,
        *actionDeleteAll,
        *actionPrint,
        *actionPdf,
        *actionHideDate,
        *actionHidePrio,
        *actionHideFrom,
        *actionWrite,
        *actionRefresh,
        *actionReload,
        *actionLarge,
        *actionSmall;

    QLineEdit *searchText,
              *insertText,
              *filterText;

    QComboBox *filterColumn;

    QToolBar *tb;
    QLabel *searchLabel;
    QLabel *filterLabel;
    QLabel *insertLabel;

    int currentColumn;
    QModelIndex currentLogRow;
    QString currentFilter;
    QString currentSearch;
    int nextRow;

    QString fileName;

    //#
    //# Switched from QTextStream to ifstream because QTextStream seemed broken
    //# on RHEL5 (qt4-4.2.1-1). If this is changed, test on RHEL5 *and* qt 4.2.1.
    //#
    std::ifstream *logStream;

    QTreeView *logView;
    LogModel *logModel;
    QSortFilterProxyModel *proxyModel;
    QFileSystemWatcher *logWatcher;
    qint64 logPos;    
    bool canClose;
    QCheckBox* lockScroll;
    
    QMutex mutex;
};

}
#endif
