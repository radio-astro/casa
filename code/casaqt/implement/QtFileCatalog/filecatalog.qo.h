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


#ifndef FILECATALOG_H
#define FILECATALOG_H

#include <graphics/X11/X_enter.h>
#include <QMainWindow>
#include <QMap>
#include <QModelIndex>
#include <QPointer>


class QAction;
class QComboBox;
class QLineEdit;
class QTreeView;
class QDir;
class QMenu;
class QFile;
class QSortFilterProxyModel;
class QComboBox;
class QStringList;
class QLabel;

#include <graphics/X11/X_exit.h>

namespace casa {

class DirModel;

class FileCatalog : public QMainWindow
{
    Q_OBJECT

public:
    FileCatalog(QString logger = 0, QWidget *parent = 0);

protected:
    virtual void closeEvent(QCloseEvent *e);

private:
    void setupActions();
    void updateMenus();
    bool load(QDir *f);
    bool removeDir(const QString &);
    bool copyDir(QString &, QString &);
    bool maybeSave();
    bool canDel;
    bool canNew;
    bool canOpen;
    bool canGo;
    bool hasSelected;


private slots:
    void fileNew();
    void dirNew();
    void fileOpen();
    
    void search();
    void remove();
    void changeDir();
    void changeDir(int);
    void changeDir(QString);
    void copy();
    void paste();
    void rename();
    void insert();
    void doFilter();
    void filterChanged(QAction*);
    void clicked(const QModelIndex &index);
private:

    QAction *actionNew,
        *actionOpen,
        *actionCD,
        *actionSearch,
        *actionFilter,
        *actionInsert,
        *actionRename,
        *actionCopy,
        *actionPaste,
        *actionDelete;

    QLineEdit *searchText;
    QLineEdit *insertText;

    QComboBox *filterColumn;
    QStringList filterList;
    QComboBox *visitedDir;
    QStringList visitedList;

    QToolBar *tb;
    QToolBar *tb2;
    QLabel *searchLabel;
    QLabel *insertLabel;

    QString copyName;
    QModelIndex currentLogRow;
    QString currentSearch;

    QString fileName;

    QTreeView *dirView;
    DirModel *dirModel;
    QSortFilterProxyModel *proxyModel;
    QDir* dir;
};

}
#endif
