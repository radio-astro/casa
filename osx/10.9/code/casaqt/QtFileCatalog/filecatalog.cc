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


#include <casaqt/QtFileCatalog/filecatalog.qo.h>
#include <casaqt/QtFileCatalog/dirmodel.qo.h>
#include <casaqt/QtFileCatalog/pictureviewer.qo.h>
#include <casaqt/QtFileCatalog/texteditor.qo.h>

#include <graphics/X11/X_enter.h>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMenuBar>
#include <QPrintDialog>
#include <QPrinter>
#include <QTreeView>
#include <QTreeWidget>
#include <QToolBar>
#include <QtDebug>
#include <QCloseEvent>
#include <QMessageBox>
#include <QLineEdit>
#include <QFileInfo>
#include <QFileInfoList>
#include <QStatusBar>
#include <QToolButton>
#include <QComboBox>
#include <QTextDocument>
#include <QLabel>
#include <QSettings>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <QDir>
#include <QInputDialog>
#include <QProcess>
#include <graphics/X11/X_exit.h>

#ifdef Q_WS_MAC
const QString rsrcPath = ":/images/mac";
#else
const QString rsrcPath = ":/images/win";
#endif

namespace casa {


FileCatalog::FileCatalog(QString /*logFile*/, QWidget *parent)
    : QMainWindow(parent), 
      canDel(0), canNew(0), canOpen(0), canGo(0), hasSelected(0),
      copyName(""), currentLogRow(),
      currentSearch(""),
      dirView(0), dirModel(0), proxyModel(0), dir(0)
{ 
    setMinimumSize(600, 500);
    setSizePolicy(QSizePolicy::Fixed,
                  QSizePolicy::MinimumExpanding);
    proxyModel = new QSortFilterProxyModel(this);

    dirView = new QTreeView(this);
    setCentralWidget(dirView);
    dirView->setFocus();
    //dirView->setSortingEnabled(true); // qt4.2
    dirView->setEditTriggers(0); //(QAbstractItemView::SelectedClicked);

    dir = new QDir();
    dir->setFilter(QDir::AllDirs | //QDir::NoSymLinks | 
                   QDir::Files);
    dir->setSorting(QDir::Name);

    QSettings settings("NRAO", "casa");
    QString prevDir = settings.value("prevDir", 
            dir->currentPath()).toString();
    dir->cd(prevDir);
    visitedList << prevDir;

    setupActions();
    load(dir);

    updateMenus();
    doFilter();
    connect(dirView, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(clicked(const QModelIndex &)));
    
}

void FileCatalog::closeEvent(QCloseEvent *e)
{
    e->accept();
}

void FileCatalog::setupActions()
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("File Actions"));
    addToolBar(Qt::LeftToolBarArea, tb);

    QMenu *menu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(menu);

    QAction *a;
    actionNew = a = new QAction(
       QIcon(rsrcPath + "/filenew.png"), tr("&New File"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_N);
    connect(a, SIGNAL(triggered()), this, SLOT(fileNew()));
    tb->addAction(a);
    menu->addAction(a);

    actionOpen = a = new QAction(
       QIcon(rsrcPath + "/editnewdir.png"), tr("New Fo&lder"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_L);
    connect(a, SIGNAL(triggered()), this, SLOT(dirNew()));
    tb->addAction(a);
    menu->addAction(a);

    actionOpen = a = new QAction(
       QIcon(rsrcPath + "/fileopen.png"), tr("&Open..."), this);
    a->setShortcut(Qt::CTRL + Qt::Key_O);
    connect(a, SIGNAL(triggered()), this, SLOT(fileOpen()));
    tb->addAction(a);
    menu->addAction(a);

    a = actionCD = new QAction(
        QIcon(rsrcPath + "/diropen.png"), tr("Go&to"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(changeDir()));
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    tb->addAction(a);
    menu->addAction(a);
    //menu->addSeparator();

    menu->addSeparator();
    
    a = new QAction(tr("&Quit"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_Q);
    connect(a, SIGNAL(triggered()), this, SLOT(close()));
    menu->addAction(a);

    tb = new QToolBar(this);
    tb->setWindowTitle(tr("Edit Actions"));
    addToolBar(Qt::LeftToolBarArea, tb);

    menu = new QMenu(tr("&Edit"), this);

    menuBar()->addMenu(menu);

    a = actionDelete = new QAction(
        QIcon(rsrcPath + "/editcut.png"), tr("Dele&te"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(remove()));
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    tb->addAction(a);
    menu->addAction(a);

    a = actionCopy = new QAction(
        QIcon(rsrcPath + "/editcopy.png"), tr("C&opy"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(copy()));
    a->setShortcut(Qt::CTRL + Qt::Key_O);
    a->setToolTip("copy selected");
    tb->addAction(a);
    menu->addAction(a);

    a = actionPaste = new QAction(
        QIcon(rsrcPath + "/editpaste.png"), tr("Past&e"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(paste()));
    a->setShortcut(Qt::CTRL + Qt::Key_E);
    a->setToolTip("paste selected");
    tb->addAction(a);
    menu->addAction(a);

    a = actionRename = new QAction(
        QIcon(rsrcPath + "/clear.png"), tr("Rena&me"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(rename()));
    a->setShortcut(Qt::CTRL + Qt::Key_R);
    a->setToolTip("Rename selected");
    tb->addAction(a);
    menu->addAction(a);

    tb2 = new QToolBar(this);
    addToolBar(Qt::TopToolBarArea, tb2);
    insertLabel = new QLabel("Current Dir: "),
    tb2->addWidget(insertLabel);

    visitedDir = new QComboBox();
    //qDebug() << "list=" << visitedList.join(",");
    visitedDir->addItems(visitedList);
    visitedDir->setSizePolicy(QSizePolicy::Expanding, 
                            QSizePolicy::Fixed);
    visitedDir->setCurrentIndex(0);
    connect(visitedDir, SIGNAL(activated(int)),
            this, SLOT(changeDir(int)));
    tb2->addWidget(visitedDir);
    visitedDir->setEditable(0);
    //visitedDir->setDuplicatesEnabled(0);
    /*
    insertText = new QLineEdit();
    insertText->setText(visitedDir->itemText(0));
    connect(insertText, SIGNAL(editingFinished()), this, SLOT(insert()));
    connect(insertText, SIGNAL(textEdited(const QString&)),
            insertText, SLOT(setText(const QString&)));
    visitedDir->setLineEdit(insertText);
    */
    menu = new QMenu(tr("&View"), this);
    menuBar()->addMenu(menu);

    searchLabel = new QLabel("Mask: ");
    tb2->addWidget(searchLabel);
    searchText = new QLineEdit();
    searchText->setMaximumWidth(100);
    searchText->setToolTip("enter search text here");
    connect(searchText, SIGNAL(editingFinished()), 
            this, SLOT(search()));
    tb2->addWidget(searchText); 

    a = actionSearch = new QAction(
       QIcon(rsrcPath + "/find.png"), tr("Sear&ch"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(search()));
    a->setShortcut(Qt::CTRL + Qt::Key_C);
    tb2->addAction(a);
    menu->addAction(a);
    
    QMenu* tpMenu = new QMenu("Type");
    
    QAction *tp;
    for (int i = 0; i < DirModel::cols.count(); i++) {
       tp = new QAction(DirModel::cols.at(i), this);
       tp->setCheckable(1);
       tpMenu->addAction(tp);
       tp->setChecked(i == 0);
    }
    filterList << DirModel::cols.at(0);
    //connect(filterColumn, SIGNAL(activated(int)), 
    //     this, SLOT(changeFilterColumn(int)));
    //connect(filterColumn, SIGNAL(activated(int)), 
    //        proxyModel, SLOT(setFilterKeyColumn(int)));
    a = actionFilter = new QAction(
                 //QIcon(rsrcPath + "/editundo.png"), 
                 tr("Filt&er"), this);
    connect(a, SIGNAL(triggered()), tpMenu, SLOT(show()));
    connect(tpMenu, SIGNAL(triggered(QAction*)), 
            this, SLOT(filterChanged(QAction*)));
    a->setShortcut(Qt::CTRL + Qt::Key_E);
    a->setMenu(tpMenu);
    tb2->addAction(a);
    menu->addAction(a);

}


void FileCatalog::updateMenus()
{
    actionNew->setEnabled(canNew);
    actionCD->setEnabled(canGo);
    bool b = hasSelected && !canGo;
    actionOpen->setEnabled(b);

    actionDelete->setEnabled(hasSelected && canDel);
    actionCopy->setEnabled(hasSelected && canDel);
    actionPaste->setEnabled(canNew && !copyName.isEmpty());
    actionRename->setEnabled(hasSelected && canDel);
    actionSearch->setEnabled(1);
    actionFilter->setEnabled(1);

    QString shownName = filterList.join(",");
    setWindowTitle(tr("File Catalog (%1)").arg(shownName));
    setWindowModified(false);
    dirView->resizeColumnToContents(0);
}

bool FileCatalog::load(QDir *f)
{

    dirModel = new DirModel(f);
    proxyModel->setSourceModel(dirModel);
    dirView->setModel(proxyModel);

    //dirView->setColumnWidth(0, 180);
    //dirView->setColumnWidth(1, 80);
    //dirView->setColumnWidth(2, 90);

    //dirView->setUniformRowHeights(1);
    //dirView->setSortingEnabled(true)
    //dirView->setAlternatingRowColors(1);
    QFileInfo inf(dir->cleanPath(dir->path()));
    canNew = inf.isWritable();
    repaint();
    return true;
}
void FileCatalog::fileNew()
{
   QString s = dir->cleanPath(dir->path());
   QString fileName =s.append("/File").append(
          QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
   QFile file(fileName);
   //bool ret =
     file.open(QFile::WriteOnly | QIODevice::Text);
   file.close();
   delete dirModel;
   load(dir);
    canGo = 0;
    canOpen = 1;
    canDel = 1;
    canNew = 1;
    hasSelected = 0;
    updateMenus();
}

void FileCatalog::dirNew()
{
    //bool ret =
    dir->mkdir(QString("Dir").append(
       QDateTime::currentDateTime().toString("yyyyMMddhhmmss")));
    QString s = dir->cleanPath(dir->path());
    delete dirModel;
    load(dir);
    canGo = 1;
    canOpen = 1;
    canDel = 1;
    canNew = 1;
    hasSelected = 0;
    updateMenus();
}
void FileCatalog::fileOpen()
{
   dir->makeAbsolute();
   QModelIndex idx = dirModel->index(
          currentLogRow.row(), 0, QModelIndex());
   QVariant vt = dirModel->data(idx, Qt::DisplayRole);
   QString s = dir->cleanPath(
            dir->path()).append("/").append(vt.toString());
   idx = dirModel->index(currentLogRow.row(), 1, QModelIndex());
   QString tp = dirModel->data(idx, Qt::DisplayRole).toString();

   QProcess *process = new QProcess(this);
   QString program;
   QStringList arguments;
   arguments << s;
   
   if (tp == "Image") {
     program = "casaviewer"; 
     process->start(program, arguments);
   }
   if (tp == "Measurement Set") {
     program = "casabrowser"; 
     process->start(program, arguments);
   }
   else if (tp == "Postscript") {
     program = "ghostview"; 
     process->start(program, arguments);
   }
   else if (tp == "PDF") {
     program = "xpdf"; 
     process->start(program, arguments);
   }
   else if (tp == "Graphics") {
    //qDebug() << "show graphics";
    PictureViewer *imageViewer = new PictureViewer("");
    imageViewer->open(s); 
    imageViewer->show();
    return;
   }
   else if (tp == "Regular File") {
    //qDebug() << "show ascii file";
    TextEditor *ed = new TextEditor();
    ed->open(s); 
    ed->show();
    //many things can happen here. e.g. modify file
    //thus the size changed. or, open new files from
    //the editor. all these can change directory
    //contents. but, dirView can not know. - a file 
    //system watcher is needed - do it like the logger?
    //or wait for qt solution.
    return;
   }
   else {
     return;
   }
   hasSelected = 0;
   updateMenus();
   return;
}

void FileCatalog::rename()
{
   if (currentLogRow.row() == -1)
      return;
    
   QModelIndex idx = dirModel->index(
          currentLogRow.row(), 0, QModelIndex());
   QVariant vt = dirModel->data(idx, Qt::DisplayRole);
   QString nm = vt.toString();

   bool ok;
   QString text = QInputDialog::getText(
           this, tr(QString("Rename ").append(nm).toAscii()),
           tr("New name: "), QLineEdit::Normal,
           "                      ", &ok);
   if (ok && !text.isEmpty()) {
      //bool b = 
         dir->rename(nm, text) ; 
      //qDebug() << "remane success: " << b
      //         << " old name=" << nm << " new name=" << text;
      delete dirModel;
      load(dir);
      //canGo = 0;
      //canOpen = 1;
      //canDel = 1;
      //canNew = 1;
      hasSelected = 0;
      updateMenus();
   }
}
void FileCatalog::search()
{
   QString newSearch = searchText->text().trimmed();
   if (newSearch == currentSearch)
      return;
   currentSearch = newSearch;
   dirModel->searchKeyChanged(currentSearch);
   //qDebug() << "new search=" << currentSearch;
}
void FileCatalog::insert()
{
   //QString newSearch = insertText->text().trimmed();
   qDebug() << "insert new dir=";// << newSearch;
}

bool FileCatalog::copyDir(QString &nm, QString &to)
{
   QDir f(nm);
   if (!f.exists())
     return false;
   QString src = nm.section("/", -1);
   QDir tgtDir(to);
   tgtDir.mkdir(src);
   QString tgt = to.append("/").append(src);
   //qDebug() << "src=" << nm << " tgt=" << tgt;
   QFileInfoList lst = f.entryInfoList();
   for (int i = 0; i < lst.count(); i++) {
      QFileInfo inf = lst.at(i);
      if (!inf.exists()) continue;
      QString d = inf.fileName();
      if (d != "." && d != "..") {
         if (inf.isDir()) { 
            QString next = nm.append("/").append(d);
            copyDir(next, tgt);
         }
         else { 
            QFile file(nm);
            //qDebug() << "from file name=" << file.fileName();
            int sd = tgt.lastIndexOf("/");
            QString nowName = tgt.mid(0, sd).append("/").
                      append(nm.section("/", -1));
            //qDebug() << "nowname=" << nowName;
            //bool ret = 
            file.copy(nowName); 
            //qDebug() << "copy ret=" << ret;
         }
      }
   }
   f.cdUp();
   return true;
}

bool FileCatalog::removeDir(const QString &nm)
{
   QDir f(nm);
   if (!f.exists())
     return false;
   QFileInfoList lst = f.entryInfoList();
   for (int i = 0; i < lst.count(); i++) {
      QFileInfo inf = lst.at(i);
      if (!inf.exists()) continue;
      QString d = inf.fileName();
      if (d != "." && d != "..") {
         if (inf.isDir()) { 
            QString next = nm;
            next.append("/").append(d);
            removeDir(next);
            //qDebug() << "entry " << i << " " << d << " is dir";
         }
         else { 
            f.remove(d);
            //qDebug() << "entry " << i << " " << d << " is file";
         }
      }
   }
   f.cdUp();
   f.rmdir(nm);
   return true;
}

void FileCatalog::remove()
{
   if (currentLogRow.row() == -1)
      return;

   QModelIndex idx = dirModel->index(
          currentLogRow.row(), 0, QModelIndex());
   QVariant vt = dirModel->data(idx, Qt::DisplayRole);
   QString nm = vt.toString();
   idx = dirModel->index(
          currentLogRow.row(), 1, QModelIndex());
   vt = dirModel->data(idx, Qt::DisplayRole);
  int ret = QMessageBox::warning(this, tr("Casa File Catalog"),
         tr("Are you sure you want to delete \n"
            "%1 %2 ?").arg(vt.toString()).arg(nm),
            QMessageBox::Yes, 
            QMessageBox::No | QMessageBox::Default,
            QMessageBox::Cancel | QMessageBox::Escape);
    if (ret == QMessageBox::No || ret == QMessageBox::Cancel)
        return;
    dir->makeAbsolute();
    QString ss = dir->cleanPath(dir->path()).append("/").append(nm);

   //qDebug() << "remove " << ss; 
   dir->remove(ss);
   removeDir(ss);
   //qDebug() << "done remove " << ss; 
   dir->makeAbsolute();
   QString s = dir->cleanPath(dir->path());
   delete dirModel;
   load(dir);
   hasSelected = 0;
   canOpen = 0;
   canDel = 0;
   canGo = 0;
   QFileInfo inf(s);
   canNew = inf.isWritable();
   updateMenus();
   //doFilter();
}
void FileCatalog::copy()
{
   if (currentLogRow.row() == -1)
      return;

   dir->makeAbsolute();
   QModelIndex idx = dirModel->index(
          currentLogRow.row(), 0, QModelIndex());
   QVariant vt = dirModel->data(idx, Qt::DisplayRole);
   copyName = dir->cleanPath(
            dir->path()).append("/").append(vt.toString());
   updateMenus();
}
void FileCatalog::paste()
{
   if (copyName.isEmpty())
      return;

   dir->makeAbsolute();
   
   QString selName = ""; 
   if (currentLogRow.row() == -1) 
      selName = dir->currentPath();
   else {
      QModelIndex idx = dirModel->index(
          currentLogRow.row(), 0, QModelIndex());
      QVariant vt = dirModel->data(idx, Qt::DisplayRole);
      selName = dir->cleanPath(
            dir->path()).append("/").append(vt.toString());
    }

    int ret = QMessageBox::warning(this, tr("Casa File Catalog"),
         tr("Are you sure you want to copy \n"
            "%1\nto\n%2 ?").arg(copyName).arg(selName),
            QMessageBox::Yes, 
            QMessageBox::No | QMessageBox::Default,
            QMessageBox::Cancel | QMessageBox::Escape);
    if (ret == QMessageBox::No || ret == QMessageBox::Cancel)
        return;

   //qDebug() << "copy " << copyName << " to " << selName; 

   QFileInfo to(selName);
   QFileInfo from(copyName);

   if (!(to.exists() && from.exists()))
      return;

   if (from.isDir()) {
     if (to.isDir()) {
         //qDebug() << "dir to dir";
         copyDir(copyName, selName);
     } 
   }
   else {
      if (to.isDir()) {
         //qDebug() << "file to dir";
         QFile file(copyName);
         QString sd = selName.append("/").
                      append(copyName.section("/", -1));
         //bool ret = 
            file.copy(sd); 
      }
      else {
         //qDebug() << "file to file";
         if (copyName != selName) {
            bool ret = dir->remove(selName);
            //qDebug() << "remove ret=" << ret;
            QFile file(copyName);
            //qDebug() << "from file name=" << file.fileName();
            int sd = selName.lastIndexOf("/");
            QString nowName = selName.mid(0, sd).append("/").
                      append(copyName.section("/", -1));
            //qDebug() << "nowname=" << nowName;
            ret = file.copy(nowName); 
            //qDebug() << "copy ret=" << ret;
            //qDebug() << "after copy file name=" << file.fileName();
         }
      }
   }
   copyName = "";
   dir->makeAbsolute();
   QString s = dir->cleanPath(dir->path());
   delete dirModel;
   load(dir);
   hasSelected = 0;
   canOpen = 0;
   canDel = 0;
   canGo = 0;
   QFileInfo inf(s);
   canNew = inf.isWritable();
   updateMenus();
}

void FileCatalog::filterChanged(QAction* act)
{
   QString sel = act->text();
   if (act->isChecked()){
      if (sel == DirModel::cols.at(0)) {
          filterList.clear();
          filterList << DirModel::cols.at(0);
      }
      else {
          filterList << sel;
          if (filterList.count() > 1) {
             for (int i = 0; i < filterList.count(); i++) {
                if (DirModel::cols.at(0) == filterList.at(i))
                   filterList.removeAt(i);
             }
          }
      }
   }
   else {
      for (int i = 0; i < filterList.count(); i++) {
        if (sel == filterList.at(i))
           filterList.removeAt(i);
      }
   }
   if (filterList.count() == 0) {
      filterList << DirModel::cols.at(0);
   }
   doFilter();
   updateMenus();
}
void FileCatalog::changeDir(int i)
{
   QString vt = visitedDir->itemText(i);
   changeDir(vt);
}
void FileCatalog::changeDir()
{
   QModelIndex idx = dirModel->index(
          currentLogRow.row(), 0, QModelIndex());
   QVariant vt = dirModel->data(idx, Qt::DisplayRole);
   //qDebug() << "goTo=" << vt.toString(); 
   changeDir(vt.toString());
}
void FileCatalog::changeDir(QString vt)
{
   QDir saved = *dir;
   //qDebug() << "change dir to " << vt;
     if (dir->cd(vt)) {
         QStringList entryList = dir->entryList();
         if (entryList.size() == 0) {
             QMessageBox::warning(this, tr("Casa File Catalog"),
             tr("Could not enter the directory:\n %1").arg(dir->path()));
             *dir = saved;
         }
         dir->makeAbsolute();
         QString s = dir->cleanPath(dir->path());
         //dirLineEdit_->setText(s);
         delete dirModel;
         load(dir);
         hasSelected = 0;
         canOpen = 0;
         canDel = 0;
         canGo = 0;
         QFileInfo inf(s);
         canNew = inf.isWritable();

         bool already = true;
         for (int i = 0; i < visitedList.count(); i++) {
           if (visitedList.at(i) == s) {
              already = false;
              break;
           }
         }
         if (!already) visitedList << s;
         visitedDir->insertItem(0, s);
         visitedDir->setCurrentIndex(0);
       }
   currentLogRow = QModelIndex();
   updateMenus();
   //doFilter();
}
void FileCatalog::doFilter()
{
   QString currentFilter = filterList.join("|");
   if (filterList.count() == 1 && 
       filterList.at(0) == DirModel::cols.at(0))
        currentFilter = ".*";
   //qDebug() << " currentFilter=" << currentFilter ;
   proxyModel->setFilterKeyColumn(1); 
   proxyModel->setFilterRegExp(
      QRegExp(currentFilter, Qt::CaseInsensitive)
            //, QRegExp::FixedString)
   ); 
   //dirView->repaint(); 
}

void FileCatalog::clicked(const QModelIndex &index)
{
   //qDebug() << " rowCount=" << proxyModel->rowCount();
   currentLogRow = proxyModel->mapToSource(index);
   //qDebug() << "selected=" << currentLogRow.row();
   QModelIndex idx = dirModel->index(
                currentLogRow.row(), 1, QModelIndex());
   QVariant vt = dirModel->data(idx, Qt::DisplayRole);
   //qDebug() << "clicked=" << vt.toString(); 
   
   QFileInfo inf(dir->cleanPath(dir->path()));
   canNew = inf.isWritable();
   canOpen = inf.isReadable();
   canDel = inf.isWritable();
   hasSelected = 1;
   if (vt.toString().compare("Directory") == 0) {
      canGo = 1;
      idx = dirModel->index(currentLogRow.row(), 0, QModelIndex());
      vt = dirModel->data(idx, Qt::DisplayRole);
      //qDebug() << "clicked name=" << vt.toString(); 
      if (vt.toString() == "..") {
         canDel = 0;
         //qDebug() << "parent dir =" << vt.toString(); 
      }
    }
    else {
      idx = dirModel->index(currentLogRow.row(), 0, QModelIndex());
      vt = dirModel->data(idx, Qt::DisplayRole);
      dir->makeAbsolute();
      QString s = dir->cleanPath(
            dir->path()).append("/").append(vt.toString());
      //qDebug() << "open=" << s; 
      canGo = 0;
    }
    updateMenus();
    //qDebug() << "canGo" << canGo;
  }
}

