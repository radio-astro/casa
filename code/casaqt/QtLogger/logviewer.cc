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


#include <casaqt/QtLogger/logviewer.qo.h>
#include <casaqt/QtLogger/logmodel.qo.h>
#include <casaqt/QtLogger/logwatcher.qo.h>

#if ! defined(__APPLE__) || defined(__USE_WS_X11__)
#include <graphics/X11/X_enter.h>
#endif
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
#include <QStatusBar>
#include <QPushButton>
#include <QToolButton>
#include <QSortFilterProxyModel>
#include <QComboBox>
#include <QCheckBox>
#include <QTextDocument>
#include <QLabel>
#include <QTextEdit>
//#include <QDialogButtonBox> //Qt4.2
#include <QHBoxLayout> // Qt 4.1
#include <QVBoxLayout>
#include <QScrollArea>
#include <QSettings>
#include <QSize>
#include <QPoint>
//#include <QHostInfo>
#include <QDesktopWidget>
#include <stdlib.h>
#if ! defined(__APPLE__) || defined(__USE_WS_X11__)
#include <graphics/X11/X_exit.h>
#endif

const QString rsrcPath = ":/images";

namespace casa {


LogViewer::LogViewer(QString logFile, QWidget *parent)
    : QMainWindow(parent), currentColumn(0),
      currentLogRow(),
      currentFilter(""), currentSearch(""),nextRow(0),
      fileName(logFile),
      logView(0), logModel(0), logPos(0)
{ 
    setObjectName("logger");
    //setMinimumSize(800, 300);
    setSizePolicy(QSizePolicy::Expanding,
                  QSizePolicy::Expanding);

    proxyModel = new QSortFilterProxyModel(this);

    setupFileActions();
    setupEditActions();
    setupInsertActions();
    
    logView = new QTreeView(this);
    setCentralWidget(logView);
    logView->setFocus();

    canClose = true;
    if (logFile == "casapy.log") {
       fileName = logFile;
       canClose = false;
    }
    //qDebug() << "load....";
    load(fileName);
    //qDebug() << "....load";
    updateMenus();

    searchText->setToolTip("enter search text here");
    connect(searchText, SIGNAL(editingFinished()), 
            this, SLOT(search()));
    filterText->setToolTip("enter filter text here");
    filterText->setText(currentFilter);
    connect(filterText, SIGNAL(editingFinished()), 
            this, SLOT(changeFilterText()));
    //connect(filterText, SIGNAL(textChanged(const QString&)), 
    //   proxyModel, SLOT(setFilterRegExp(const QString&)));
    //connect(filterText, SIGNAL(textChanged(const QString&)), 
    //   proxyModel, SLOT(setFilterWildcard(const QString&)));
    //connect(filterText, SIGNAL(textChanged(const QString&)), 
    //   proxyModel, SLOT(setFilterFixedString(const QString&))); 
    insertText->setToolTip("enter user log message here");
    connect(insertText, SIGNAL(editingFinished()), 
            this, SLOT(insert()));
    doFilter();
    connect(logView, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(clicked(const QModelIndex &)));
    //an open qt 4.1.x bug, scroll does not work here
    //qDebug() << "--------";
    QModelIndex idx = proxyModel->index(
          proxyModel->rowCount() - 1, 0, QModelIndex());
    logView->setCurrentIndex(idx);
    logView->scrollTo(idx);
    clicked(idx);
    //qDebug() << "========";
    logView->scrollToBottom();
    logView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    logView->setTextElideMode(Qt::ElideLeft);
    QSettings settings("NRAO", "casa");
    resize(settings.value("logSize", QSize(800, 300)).toSize());

    QPoint pt = settings.value("logPos", QPoint(200, 200)).toPoint();
    //qDebug() << "logPos=" << pt;
    int width = QApplication::desktop()->width();
    int height = QApplication::desktop()->height();
    while (pt.x() < 0) pt.rx() += width;
    while (pt.x() > width) pt.rx() -= width;
    while (pt.y() < 0) pt.ry() += height;
    while (pt.y() > height) pt.ry() -= height;

    move(pt);

#if QT_VERSION >= 0x040300
	setUnifiedTitleAndToolBarOnMac(true);
#endif
}
QSize LogViewer::sizeHint () const {
    return QSize(800, 360);
}
void LogViewer::closeEvent(QCloseEvent *e)
{
    QSettings settings("NRAO", "casa");
    settings.setValue("logPos", pos());
    settings.setValue("logSize", size());

    //never save anything unless explicitly ask
    /*
    if (maybeSave())
        e->accept();
    else
        e->ignore();
    */
    canClose = 1;
    if (canClose)
        e->accept();
    else
        e->ignore();
}

void LogViewer::moveEvent(QMoveEvent */*e*/)
{
    QSettings settings("NRAO", "casa");
    settings.setValue("logPos", pos());
    settings.setValue("logSize", size());
}

void LogViewer::setupFileActions()
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("File Actions"));
    addToolBar(tb);

    QMenu *menu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(menu);

    QAction *a;
    actionOpen = a = new QAction(
       QIcon(rsrcPath + "/fileopen.png"), tr("&Open..."), this);
    a->setShortcut(QKeySequence(QKeySequence::Open));
    connect(a, SIGNAL(triggered()), this, SLOT(fileOpen()));
    tb->addAction(a);
    menu->addAction(a);

    menu->addSeparator();

    actionSave = a = new QAction(
       QIcon(rsrcPath + "/filesave.png"), tr("&Save"), this);
    a->setShortcut(QKeySequence(QKeySequence::Save));
    connect(a, SIGNAL(triggered()), this, SLOT(fileSave()));
    tb->addAction(a);
    menu->addAction(a);

    actionSaveAs = a = new QAction(tr("Save &As..."), this);
    a->setShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_S);
    connect(a, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
    menu->addAction(a);
    menu->addSeparator();

    actionPrint = a = new QAction(
       QIcon(rsrcPath + "/fileprint.png"), tr("&Print..."), this);
    a->setShortcut(QKeySequence(QKeySequence::Print));
    connect(a, SIGNAL(triggered()), this, SLOT(filePrint()));
    tb->addAction(a);
    menu->addAction(a);

#ifndef Q_WS_MAC
    actionPdf = a = new QAction(QIcon(rsrcPath + 
         "/exportpdf.png"), tr("&Export PDF..."), this);
    a->setShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_E);
    connect(a, SIGNAL(triggered()), this, SLOT(filePrintPdf()));
    tb->addAction(a);
    menu->addAction(a);
#endif
    menu->addSeparator();

    a = new QAction(tr("&Quit"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_Q);
    connect(a, SIGNAL(triggered()), this, SLOT(close()));
    menu->addAction(a);
}

void LogViewer::setupEditActions()
{
    QToolBar *tb = new QToolBar(this);
    tb->setWindowTitle(tr("Edit Actions"));
    addToolBar(tb);

    QMenu *menu = new QMenu(tr("&Edit"), this);
    menuBar()->addMenu(menu);

    QAction *a;

    a = actionDelete = new QAction(
        QIcon(rsrcPath + "/editcut.png"), tr("Dele&te"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(remove()));
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    tb->addAction(a);
    menu->addAction(a);
    menu->addSeparator();
    
    a = actionDeleteAll = new QAction(
        QIcon(rsrcPath + "/editdelete.png"), tr("Pur&ge"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(purge()));
    //a->setShortcut(Qt::CTRL + Qt::Key_G);
    //a user thinks purge button dangerous thus disabled
    //tb->addAction(a);
    menu->addAction(a);

    a = actionCopy = new QAction(
        QIcon(rsrcPath + "/editcopy.png"), tr("C&opy"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(copy()));
    a->setShortcut(QKeySequence(QKeySequence::Copy));
    a->setToolTip("copy selected, then you can paste\n"
                  "it to other windows (e.g. email)");
    tb->addAction(a);
    menu->addAction(a);

    searchLabel = new QLabel("Search Message: ");
    tb->addWidget(searchLabel);
    searchText = new QLineEdit();
    tb->addWidget(searchText); 
    a = actionSearch = new QAction(
       QIcon(rsrcPath + "/find.png"), tr("Sear&ch"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(search()));
    a->setShortcut(QKeySequence(QKeySequence::Find));
    tb->addAction(a);
    menu->addAction(a);
    a = actionNext = new QAction(
       QIcon(rsrcPath + "/downarrowstop.png"), tr("Find &Next"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(findNext()));
    a->setShortcut(QKeySequence(QKeySequence::FindNext));
    tb->addAction(a);
    menu->addAction(a);

    filterLabel = new QLabel("Filter: ");
    tb->addWidget(filterLabel);
    filterColumn = new QComboBox();
    QStringList cols;
    cols << "Time" << "Priority" << "Origin" << "Message";
    filterColumn->addItems(cols);
    connect(filterColumn, SIGNAL(activated(int)), 
            this, SLOT(changeFilterColumn(int)));
    //connect(filterColumn, SIGNAL(activated(int)), 
    //        proxyModel, SLOT(setFilterKeyColumn(int)));
    tb->addWidget(filterColumn);

    filterText = new QLineEdit();
    tb->addWidget(filterText); 
    a = actionFilter = new QAction(
       QIcon(rsrcPath + "/view_filter.png"), tr("F&ilter"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(changeFilterText()));
    // a->setShortcut(Qt::CTRL + Qt::Key_I);
    tb->addAction(a);
    menu->addAction(a);

    a = actionReload = new QAction(
       QIcon(rsrcPath + "/view_refresh.png"), tr("Reload"), this);
    // a->setShortcut(Qt::CTRL + Qt::Key_M);
    a->setToolTip("Reload, display full log, or undo search/filter");
    //QToolButton* rl = new QToolButton();
    //rl->setDefaultAction(actionReload);
    connect(a, SIGNAL(triggered()), this, SLOT(reload()));
    tb->addAction(a);
    menu->addAction(a);
    //sb->addWidget(rl, 4);

    QStatusBar *sb = new QStatusBar(this);
    sb->setWindowTitle(tr("Insert Actions"));
    setStatusBar(sb);

    insertLabel = new QLabel("Insert Message: "),
    sb->addWidget(insertLabel);
    insertText = new QLineEdit();
    //insertText->setSizePolicy(QSizePolicy::Expanding, 
    //                          QSizePolicy::Expanding);
    //insertText->setMinimumWidth(400);
    sb->addWidget(insertText); 
    a = actionInsert = new QAction(
        QIcon(rsrcPath + "/edit_add.png"), tr("Inse&rt"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(insert()));
    a->setShortcut(Qt::CTRL + Qt::Key_R);
    QToolButton* it = new QToolButton();
    it->setDefaultAction(actionInsert);
    menu->addAction(a);
    sb->addWidget(it, 1);
  
    a = actionWrite = new QAction(
       QIcon(rsrcPath + "/object_edit.png"), tr("&Write a long log"), this);
    //a->setShortcut(Qt::CTRL + Qt::Key_W);
    QToolButton* wt = new QToolButton();
    wt->setDefaultAction(actionWrite);
    connect(a, SIGNAL(triggered()), this, SLOT(logOpen()));
    menu->addAction(a);
    sb->addWidget(wt, 2);

    a = actionRefresh = new QAction(
       QIcon(rsrcPath + "/view_refresh.png"), tr("Refresh"), this);
    a->setShortcut(Qt::CTRL + Qt::Key_H);
    a->setToolTip("Refresh, if you suspect the logger stop running");
    QToolButton* fr = new QToolButton();
    fr->setDefaultAction(actionRefresh);
    connect(a, SIGNAL(triggered()), this, SLOT(refresh()));
    menu->addAction(a);
    sb->addWidget(fr, 3);


    lockScroll = new QCheckBox("Lock scroll");
    lockScroll->setFixedWidth(90);
    //lockScroll->setSizePolicy(QSizePolicy::Minimum, 
    //                          QSizePolicy::Minimum);
    lockScroll->setToolTip("stop auto vertical scrolling");
    lockScroll->setChecked(false);
    sb->addWidget(lockScroll, 4);
}


void LogViewer::updateMenus()
{
    // bool b = (fileName != ""); 
    // actionOpen->setEnabled(!b);
    // actionSave->setEnabled(b);
    // actionSaveAs->setEnabled(b);
    // actionSearch->setEnabled(b);
    // actionFilter->setEnabled(b);
    // actionInsert->setEnabled(b);
    // actionDelete->setEnabled(b);
    // actionDeleteAll->setEnabled(b);
    // actionPrint->setEnabled(b);
    // actionPdf->setEnabled(b);
    // actionCopy->setEnabled(b);
    // actionHideDate->setEnabled(b);
    // actionHidePrio->setEnabled(b);
    // actionHideFrom->setEnabled(b);
    // actionWrite->setEnabled(b);
    // actionRefresh->setEnabled(b);
    // actionLarge->setEnabled(b && logModel &&
    //                         (logModel->getFontSize() < 24));
    // actionSmall->setEnabled(b && logModel &&
    //                         (logModel->getFontSize() > 6));

    //QString shownName = QFileInfo(fileName).fileName();
    //QString localHost = QHostInfo().localHostName(); 
    //qDebug() << "localhost=" << localHost;

    char *host = getenv("HOSTNAME");
    //qDebug() << "host=" << host;

    QString shownName = QFileInfo(fileName).absoluteFilePath();
    setWindowTitle(tr("Log Messages (%1:%2)")
       .arg(host).arg(shownName));
    setWindowModified(false);
}

void LogViewer::setupInsertActions()
{

    QMenu *menu = new QMenu(tr("&View"), this);
    menuBar()->addMenu(menu);

    QAction *a;

    a = actionHideDate = new QAction(tr("Hide dat&e"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(hide()));
    a->setShortcut(Qt::CTRL + Qt::Key_E);
    a->setCheckable(true);
    menu->addAction(a);

    a = actionHidePrio = new QAction(tr("Hide priorit&y"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(hide()));
    a->setShortcut(Qt::CTRL + Qt::Key_Y);
    a->setCheckable(true);
    menu->addAction(a);

    a = actionHideFrom = new QAction(tr("Hide origi&n"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(hide()));
    a->setShortcut(Qt::CTRL + Qt::Key_N);
    a->setCheckable(true);
    menu->addAction(a);

    menu->addSeparator();

    a = actionLarge = new QAction(tr("&Larger font"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(changeFont()));
    a->setShortcut(Qt::CTRL + Qt::Key_L);
    menu->addAction(a);

    a = actionSmall = new QAction(tr("Sm&aller font"), this);
    connect(a, SIGNAL(triggered()), this, SLOT(changeFont()));
    a->setShortcut(Qt::CTRL + Qt::Key_A);
    menu->addAction(a);

}


bool LogViewer::load(const QString &f)
{
    //qDebug() << "fileName--------" << fileName;
    //qDebug() << "f----" << f;
    if (!QFile::exists(f)) {
        QFile file(f);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;
        QTextStream out(&file);
        out << "\n";
        file.close();
    }

    std::string name = fileName.toStdString();
    try {
       logStream = new std::ifstream(name.c_str());
    }
    catch (std::bad_alloc &x) {
       qDebug() << "Can't load the file. " << x.what();
       return false;
    }

    logStream->seekg(0,std::ios::end);
    unsigned int len = logStream->tellg();
    //qDebug() << "#c=" << len;
    int length = 100 * 500000; //500000 rows each 100 chars
    if (len < (unsigned int)length) {
       length = len;
       logStream->seekg(0,std::ios::beg);
    }
    else {
       logStream->seekg(len - length);
    }
    char *buf;
    buf = new (std::nothrow) char[length+1];
    if (!buf) {
       qDebug() << "Can't allocate memory region of size=" << length ;
       return false;
    }

    //same functionality as above
    //try {
    //   buf = new char[length+1];
    //}
    //catch (std::bad_alloc &x) {
    //   qDebug() << "Can't allocate memory region: " << x.what();
    //   return false;
    //}

    buf[0] = '\0';
    logStream->read(buf,length);
    buf[length] = '\0';
	
    mutex.lock();
    logModel = new LogModel(buf);
    mutex.unlock();

    delete buf;

    // logWatcher = new QFileSystemWatcher();
    // logWatcher->addPath(fileName);
    // connect(logWatcher, SIGNAL(fileChanged(const QString&)),
    //         this, SLOT(fileChanged(const QString&)));


	FileWatcher *watcher = new FileWatcher(fileName);
	connect(watcher, SIGNAL(fileChanged(const QString&)),
	    this, SLOT(fileChanged(const QString&)));


    proxyModel->setSourceModel(logModel);
    logView->setModel(proxyModel);

    logView->resizeColumnToContents(0);
    logView->setUniformRowHeights(0);
    logView->resizeColumnToContents(3);

    logView->setAlternatingRowColors(1);
    repaint();
    return true;
}

bool LogViewer::maybeSave()
{
    if (fileName.startsWith(QLatin1String(":/")))
        return true;
    int ret = QMessageBox::warning(this, tr("Casa Log Viewer"),
         tr("The document has been modified.\n"
            "Do you want to save your changes?"),
            QMessageBox::Yes | QMessageBox::Default,
            QMessageBox::No,
            QMessageBox::Cancel | QMessageBox::Escape);
    if (ret == QMessageBox::Yes)
        return fileSave();
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

void LogViewer::fileOpen()
{
    QString fn = QFileDialog::getOpenFileName(this, 
         tr("Open File..."), QString(), 
         tr("Casa Log Files (*.log)"));
    if (!fn.isEmpty()) {
        fileName = fn;
        load(fileName);
    }
    updateMenus();
}

bool LogViewer::fileSave()
{
    if (fileName.isEmpty())
        return fileSaveAs();
    fileName = fileName.section('/', -1);
    if (fileName == "casapy.log")
        fileName.prepend("~");
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QIODevice::Text))
        return false;
    QTextStream ts(&file);
    logModel->writeData(ts);
    return true;
}

bool LogViewer::fileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName(this, 
       tr("Save as..."),
       QString(), tr("Casa Log Files (*.log)"));
    if (fn.isEmpty())
        return false;

    int pos = fn.lastIndexOf('/');

    if (pos < 0)
       return false;

    QString path = fn.left(pos);
    QString fnm = fn.section('/', -1);
    if (fnm == "casapy.log")
       fnm.prepend("~");
    if (fnm.startsWith("-"))
       fnm.replace("-", "_");
    //if (fnm == "")
    //    fnm = "~casapy.log";
    if (!fnm.endsWith(".log"))
        fnm.append(".log");

    fn = path + '/' + fnm;
    
    //qDebug() << "fn=" << fn << "fnm=" << fnm;
    
    QFile file(fn);
    if (!file.open(QFile::WriteOnly))
        return false;
    QTextStream ts(&file);
    logModel->writeData(ts);
    return true;
}

void LogViewer::filePrint()
{
#ifndef QT_NO_PRINTER
    QPrinter printer(QPrinter::ScreenResolution);
    //printer.setFullPage(true);
    QSettings settings("CASA", "Logger");
    //qDebug() << settings.value("Print/printer").toString();
#ifndef Q_WS_MAC
    printer.setPrinterName(settings.value("Print/printer").toString());
#endif
    printer.setPageSize((QPrinter::PageSize)(settings.value("Print/pagesize").toInt()));
    printer.setOrientation(QPrinter::Landscape);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (dlg->exec() == QDialog::Accepted) {
      QTextDocument td(logModel->stringData());
      QFont theFont("Monospace", 8, QFont::Normal);
      theFont.setStretch(50);
      td.setDefaultFont(theFont);
      //the standard us paper is w=663, h=975. if a margin
      //is desirable, make pageSize > 663
      QRect rect = printer.paperRect();
      //w = resolution (pixel / inch) * paperWidth (inch) 
      //h = resolution (pixel / inch) * paperHeight (inch) 
      int w = rect.width();
      int h = rect.height();
      int o = printer.orientation();
      //int r = printer.resolution();
      //qDebug() << "w=" << w << "h=" << h;
      //qDebug() << "r=" << r << "o=" << o;
      if (o == QPrinter::Portrait)  
          td.setPageSize(QSize(w - 100, h - 80));
      else 
          td.setPageSize(QSize(h - 100, w - 80));

      if (!printer.isValid()) {
         qDebug() << "not a valid printer" ;
      }
      else {
         td.print(&printer);
      }

      settings.setValue("Print/printer", printer.printerName());
      settings.setValue("Print/pagesize", printer.pageSize());
    }
    delete dlg;
#endif
}


void LogViewer::filePrintPdf()
{
#ifndef QT_NO_PRINTER
    QString fileName = QFileDialog::getSaveFileName(this, 
        "Export PDF", QString(), "*.pdf");
    if (fileName.isEmpty())
        return;
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    QTextDocument td(logModel->stringData());
    //qDebug() << "size" << td.pageSize().width() 
    //           << td.pageSize().height();
    td.setPageSize(QSize(713, 1025));
    td.setDefaultFont(QFont("Bitstream Vera Sans Mono", 6, QFont::Bold));
    td.print(&printer);
#endif
}


#define TIME_LIMIT_IN_MILLISECONDS	(1000)
void LogViewer::fileChanged(const QString& /*fileName*/)
{
	if (!logStream->eof()) {
		unsigned int current_offset = logStream->tellg( );
		logStream->seekg(0,std::ios::end);
                unsigned int now = logStream->tellg();
		int length = now - current_offset;
                //qDebug() << "current_offset=" << current_offset 
                //         << " length=" << length;
		logStream->seekg(current_offset,std::ios::beg);
		char *buf;
                buf = new (std::nothrow) char[length+1];
                if (!buf) {
                   qDebug() << "Can't allocate more memory region" ;
                   return;
                }

                //try {
                //   buf = new char[length+1];
                //}
                //catch(...) {
                //   qDebug() << "Can't allocate more memory region";
                //   return;
                //}

		buf[0] = '\0';

		logStream->read(buf,length);

		//
		// Here we backtrack on the read buffer, until we encounter a newline...
		// most of the time we do no backtracking because the log file is typically
		// updated line-by-line... however, sometimes we catch the file mid-write...
		//
		char *ptr;
		int line_offset = 0;
		for (ptr = &buf[length-1]; *ptr != '\n' && ptr >= buf; --ptr) line_offset += 1;

		if ( line_offset > 0 ) {
			logStream->seekg( -line_offset, std::ios::cur );
		}
		*(ptr+1) = '\0';

                int wid = logView->columnWidth(3);
		if (mutex.tryLock()) {
			logModel->appendData(buf);
			if (!lockScroll->isChecked())
				logView->scrollToBottom();

			logView->resizeColumnToContents(3);
                        wid = qMax(wid, logView->columnWidth(3));
                        logView->setColumnWidth(3, wid);
			logView->update();
			mutex.unlock();
		}
		delete buf;
	}
}


void LogViewer::search()
{
   QString newSearch = searchText->text().trimmed();
   if (newSearch == currentSearch)
      return;
   currentSearch = newSearch;
   logModel->searchKeyChanged(currentSearch);
   //qDebug() << "new search=" << currentSearch;
}

void LogViewer::findNext()
{
   QString newSearch = searchText->text().trimmed();
   //if (newSearch == currentSearch)
   //   return;
   //currentSearch = newSearch;
   //logModel->searchKeyChanged(currentSearch);
   
   if (newSearch == "")
      return;

   if (nextRow >= logModel->rowCount() - 1)
      nextRow = -1;

   QModelIndex idx = QModelIndex();
   int k = nextRow;
   int i = nextRow;
   //qDebug() << "i=" << i << " k=" << k 
   //         << " rowCount=" << logModel->rowCount() ;;
   for (i = nextRow + 1; i < logModel->rowCount(); i++) {
     QString str = proxyModel->data(
         proxyModel->index(i, 3, idx), Qt::DisplayRole).toString();
     //qDebug() << str << i;
     if (str != "" && str.contains(newSearch)) {
        QModelIndex idx = proxyModel->index(i, 0, QModelIndex());
        logView->setCurrentIndex(idx);
        logView->scrollTo(idx);
        nextRow = i;
        return;
     }
   }
   //qDebug() << "i=" << i;
   if (i == logModel->rowCount())
      nextRow = -1;

   for (int i = 0; i < k; i++) {
     QString str = proxyModel->data(
         proxyModel->index(i, 3, idx), Qt::DisplayRole).toString();
     //qDebug() << str << i;
     if (str != "" && str.contains(newSearch)) {
        QModelIndex idx = proxyModel->index(i, 0, QModelIndex());
        logView->setCurrentIndex(idx);
        logView->scrollTo(idx);
        nextRow = i;
        return;
     }
   }

   //qDebug() << "new search=" << currentSearch;
}

void LogViewer::reload()
{
   if (searchText->text() != "") {
       searchText->setText("");
       search();
   }
   if (filterText->text() != "") {
       filterText->setText("");
       changeFilterText();
   }
}

void LogViewer::remove()
{
   if (mutex.tryLock()) {
   //if (currentLogRow.row() == -1)
   //   return;
   //4.1.3>>>>>>>>>>
   QModelIndexList lst = logView->selectionModel()->selectedIndexes();

   QSet<int> idSet;
   for (int i = 0; i < lst.count(); i++) {
     QModelIndex s = proxyModel->mapToSource(lst.at(i));
     idSet.insert(s.row());
   }
 
   int rowNum = idSet.size();
   if (rowNum < 1)
       return;

   int rowId[rowNum];
   int k = 0;
   foreach (int value, idSet)            
     rowId[k++] = value;

   //for (int i = 0; i < rowNum; i++) {
   //   qDebug() << "rowId=" << rowId[i];
   //}

   int sortedId[rowNum];
   int last = 100000000;
   for (int j = 0; j < rowNum; j++) {
     int max = -1;
     int t = -1;
     for (int i = 0; i < rowNum; i++) {
       if (rowId[i] > max && rowId[i] < last) {
         max = rowId[i];
         t = i;
       }
     }
     sortedId[j] = rowId[t]; 
     last = max;
     //qDebug() << "max=" << last << "sorted" << j << sortedId[j];
   }
   
   //for (int i = 0; i < rowNum; i++) {
   //  qDebug() << "sorted row" << sortedId[i];
   //}

   for (int i = 0; i < rowNum; i++) {
     logModel->removeRow(sortedId[i], QModelIndex()); 
   }
   //<<<<<<<<<<<<<<<

   //4.2>>>>>>>> 
   //QModelIndexList lst = logView->selectionModel()->selectedRows();
   //for (int i = lst.count() - 1; i > -1; i--) {
   //  QModelIndex s = proxyModel->mapToSource(lst.at(i));
     //qDebug() << "selected/mapped row" << s.row();
   //  logModel->removeRow(s.row(), QModelIndex()); 
   //}
   //<<<<<<<<<<

   /*single item selection mode
   //bool ret = 
   //logModel->removeRow(currentLogRow.row(), QModelIndex()); 
   */
   mutex.unlock();
   }


}
void LogViewer::purge()
{
   if (mutex.tryLock()) {
      //bool ret = 
      logModel->removeRows(0, 
         proxyModel->rowCount(QModelIndex()), QModelIndex()); 
      mutex.unlock();
   }
}
void LogViewer::copy()
{
   QClipboard *board = QApplication::clipboard();
   //erase from memory the selected
   //board->clear(QClipboard::Selection);

   //qDebug() << "selection=" << board->supportsSelection();
   //qDebug() << "FindBuffer=" << board->supportsFindBuffer();
   QModelIndexList lst = logView->selectionModel()->selectedIndexes();

   //4.2>>>>>>>>>>
   //QModelIndexList lst = logView->selectionModel()->selectedIndexes();
   //QString clip = "";
   //for (int i = 0; i < lst.count(); i += 4) {
   // QModelIndex s = proxyModel->mapToSource(lst.at(i));
   //  qDebug() << "selected/mapped row" << s.row();
   // QString str = logModel->stringData(s.row()); 
   // clip = clip + str;
   //}
   //board->setText(clip);
   // qDebug() << "====cliplboard: " 
   //            << board->text(QClipboard::Selection);
   //4.2<<<<<<<<<<
   
   QSet<int> idSet;
   for (int i = 0; i < lst.count(); i++) {
     QModelIndex s = proxyModel->mapToSource(lst.at(i));
     //qDebug() << "selected/mapped row" << s.row();
     idSet.insert(s.row());
   }
 
   int rowNum = idSet.size();
   if (rowNum < 1)
       return;

   int rowId[rowNum];
   int k = 0;
   foreach (int value, idSet)            
     rowId[k++] = value;

   //for (int i = 0; i < rowNum; i++) {
   //   qDebug() << "rowId=" << rowId[i];
   //}

   int sortedId[rowNum];
   int last = 100000000;
   for (int j = 0; j < rowNum; j++) {
     int max = -1;
     int t = -1;
     for (int i = 0; i < rowNum; i++) {
       if (rowId[i] > max && rowId[i] < last) {
         max = rowId[i];
         t = i;
       }
     }
     sortedId[j] = rowId[t]; 
     last = max;
     //qDebug() << "max=" << last << "sorted" << j << sortedId[j];
   }
   
   //for (int i = 0; i < rowNum; i++) {
   //  qDebug() << "sorted row" << sortedId[i];
   //}

   QString clip = "";
   for (int i = rowNum - 1; i > -1; i--) {
     //qDebug() << "sorted row" << i;
     QString str = logModel->stringData(sortedId[i]); 
     //logModel->removeRow(sortedId[i], QModelIndex()); 
     clip = clip + str;
   }
   //board->setText(clip, QClipboard::Selection);
   board->setText(clip);

   
   

   /* single selection mode
   QString str = logModel->stringData(currentLogRow.row()); 
   QApplication::clipboard()->setText(
             str, QClipboard::Selection);
   //qDebug() << "====clilboard: " 
   //  << QApplication::clipboard()->text(QClipboard::Selection);
   */
}
void LogViewer::insert()
{
   QString newInsert = insertText->text();
   if (newInsert == "")
      return;
   //qDebug() << "new insert=" << newInsert;
   if (!logModel) 
      return;
   if (mutex.tryLock()) {
   //bool ret = 
   logModel->insert(currentLogRow.row(), newInsert);
   insertText->setText("");
   logView->resizeColumnToContents(3);
   //logView->repaint();
   mutex.unlock();
   }
}

void LogViewer::changeFilterColumn(int i)
{
   if (i == currentColumn)
      return;
   currentColumn = i;
   doFilter();
}

void LogViewer::changeFilterText()
{
   QString newFilter = filterText->text();
   if (newFilter == currentFilter)
      return;
   currentFilter = newFilter;
   doFilter();
}

void LogViewer::doFilter()
{
   //qDebug() << " column=" << currentColumn 
   //         << " currentFilter=" << currentFilter ;
   proxyModel->setFilterKeyColumn(currentColumn); 
   proxyModel->setFilterRegExp(
      QRegExp(currentFilter.append("|NOTE"), Qt::CaseInsensitive)
            //, QRegExp::FixedString)
   ); 
   //logView->repaint(); 
}

void LogViewer::clicked(const QModelIndex &index)
{
   //qDebug() << " rowCount=" << proxyModel->rowCount();
   currentLogRow = proxyModel->mapToSource(index);
   //as multiple selection allowed, copy/delete should 
   //check each item see if it is selected

}
void LogViewer::hide()
{
   QAction *a = dynamic_cast<QAction *>(sender());
   if (!a)
      return;
   QString txt = a->text();
   if (txt.contains("dat")) 
      logView->setColumnHidden(0, a->isChecked()); 
   if (txt.contains("priorit")) 
      logView->setColumnHidden(1, a->isChecked()); 
   if (txt.contains("origi")) 
      logView->setColumnHidden(2, a->isChecked()); 
}
void LogViewer::refresh()
{
//   logWatcher->update();
}
void LogViewer::logOpen()
{
   QDialog *dlg = new QDialog(0);
   QTextEdit *edit = new QTextEdit(this);
   dlg->setWindowTitle("Write a long log");
   dlg->setFixedWidth(400);
   /* Qt 4.2
   QDialogButtonBox *buttonBox = new QDialogButtonBox(
           QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

   buttonBox->button(QDialogButtonBox::Ok)->setText("Write");
   connect(buttonBox, SIGNAL(accepted()), dlg, SLOT(accept()));
   connect(buttonBox, SIGNAL(rejected()), dlg, SLOT(reject()));

   QVBoxLayout *mainLayout = new QVBoxLayout;
   mainLayout->addWidget(edit);
   mainLayout->addWidget(buttonBox);
   dlg->setLayout(mainLayout);
   */

   QVBoxLayout *mainLayout = new QVBoxLayout;
   mainLayout->addWidget(edit);
   QHBoxLayout *buttonLayout = new QHBoxLayout;
   QPushButton *okButton = new QPushButton("ok");
   QPushButton *cancelButton = new QPushButton("cancel");
   buttonLayout->addWidget(okButton);
   buttonLayout->addWidget(cancelButton);
   mainLayout->addLayout(buttonLayout);
   dlg->setLayout(mainLayout);
   connect(okButton, SIGNAL(clicked()), dlg, SLOT(accept()));
   connect(cancelButton, SIGNAL(clicked()), dlg, SLOT(reject()));

   if (QDialog::Accepted == dlg->exec()) {
      //qDebug() << "write log book";
      QString longlog = edit->toPlainText();
      if (longlog == "")
           return;
      logModel->insert(currentLogRow.row(), longlog);
      //logModel->searchKeyChanged(currentSearch);
   }
}
void LogViewer::changeFont()
{
   QAction *a = dynamic_cast<QAction *>(sender());
   if (!a)
      return;
   QString txt = a->text();
   if (txt.contains("ller")) 
      logModel->changeFont(-1);
   if (txt.contains("Large"))
      logModel->changeFont(1);
   updateMenus();
}
}
