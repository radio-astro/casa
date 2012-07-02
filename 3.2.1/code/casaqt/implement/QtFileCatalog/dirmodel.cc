//# Copyright (C) 2005
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/ormodify it
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
//# $Id:  $

#include <casaqt/QtFileCatalog/diritem.h>
#include <casaqt/QtFileCatalog/dirmodel.qo.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableInfo.h>
#include <casa/BasicSL/String.h>
#include <casa/OS/File.h>
#include <casa/iostream.h>
#include <casa/fstream.h>
#include <casa/Exceptions/Error.h>

#include <graphics/X11/X_enter.h>
#include <QtGui>
#include <QDebug>
#include <QDateTime>
#include <QSettings>
#include <QColor>
#include <graphics/X11/X_exit.h>



namespace casa { 

DirModel::DirModel(QDir *data, QObject *parent)
    : QAbstractItemModel(parent), insertItem(0), searchKey("")
{
    QList<QVariant> rootData;
    rootData << "Name" << "Type" 
             << "Size" << "Last Modified"; 
    rootItem = new DirItem(rootData);
    setupModelData(data, rootItem);
}

DirModel::~DirModel()
{
    delete rootItem;
    delete insertItem;
}
const QStringList DirModel::cols = QStringList()
         << "ALL" << "Image" << "Measurement Set"
         << "FITS" << "Miriad Image" << "Mariad Vis" << "IERS"
         << "Python" << "Postscript" << "Graphics" << "PDF"
         << "Directory" << "Any Table" << "Record"
         << "Calibrarion" << "Log Message" << "Other Table"
         << "Gipsy" << "Plot File"
         << "Ascii" << "Regular File";

const QColor DirModel::colors[] = {
     Qt::white, Qt::black, Qt::red, Qt::darkRed,
     Qt::green, Qt::darkGreen, Qt::cyan, Qt::darkCyan, 
     Qt::magenta, Qt::darkMagenta, Qt::darkYellow, Qt::gray, 
     QColor(255, 0, 0, 127), QColor(0, 255, 0, 127),
     QColor(0, 0, 255, 127), QColor(0, 127, 127, 127),
     Qt::darkGray, Qt::lightGray, Qt::blue, Qt::darkBlue,
     QColor(0, 127, 255, 255), Qt::color0, Qt::color1, Qt::transparent
};

int DirModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<DirItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QColor DirModel::itemColor(const DirItem *item) {
   for (int i = 0; i < cols.count(); i++) {
       if (item->data(1).toString() == cols.at(i))
         return QColor(colors[i]);
   }    
   return QColor(Qt::black);
}

QVariant DirModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    DirItem *item = static_cast<DirItem*>(index.internalPointer());
    QVariant vt = item->data(index.column());
    if (role == Qt::TextColorRole) {
        for (int i = 0; i < cols.count(); i++) {
          //qDebug() << item->data(1).toString() << " " << cols.at(i);
          if (item->data(1).toString() == cols.at(i))
              return QVariant(QColor(colors[i]));
        }    
        return  QVariant(QColor(Qt::red));
    }
    if (role == Qt::BackgroundColorRole) {
        QString msg = item->data(0).toString();
        //qDebug() << "msg=" << msg << " searchKey=" << searchKey;
        if (searchKey != "" && msg != "" && msg.contains(searchKey)) {
           return QVariant(QColor(Qt::yellow));
        }
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    return vt;
}

Qt::ItemFlags DirModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled 
           | Qt::ItemIsEditable
           | Qt::ItemIsSelectable; 
}

QVariant DirModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex DirModel::index(int row, int column, const QModelIndex &parent)
            const
{
    DirItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<DirItem*>(parent.internalPointer());

    DirItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex DirModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();
    //qDebug() << "index valid";

    DirItem *childItem = static_cast<DirItem*>(index.internalPointer());
    DirItem *parentItem = childItem->parent();

    if (parentItem == rootItem) {
        //qDebug() << "root index";
        return QModelIndex();
    }

    //qDebug() << "child index";
    return createIndex(parentItem->row(), 0, parentItem);
}

int DirModel::rowCount(const QModelIndex &parent) const
{
    DirItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<DirItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void DirModel::searchKeyChanged(const QString &key) 
{
    searchKey = key;
    beginInsertRows(QModelIndex(), 0, 0);
    //qDebug() << "searchKey" << searchKey;
    endInsertRows();
}
bool DirModel::removeRows (int row, int count, const QModelIndex &parent){
    //qDebug() << "child=" << rootItem->childCount() 
    //         << " remove rows=" << row << " count=" << count;
    int last = row + count;
    if (last > rootItem->childCount() || row < 0 || count < 1)
       return false;
    beginRemoveRows(parent, row, last - 1);
    for (int i = last - 1; i >= row; i--) {
       rootItem->removeChildAt(i);
    } 
    endRemoveRows();
    return true;
}

bool DirModel::insertRows (int row, int count, const QModelIndex &parent){
    //qDebug() << "child=" << rootItem->childCount() 
    //         << " remove rows=" << row << " count=" << count;
    int last = row + count;
    if (row < 0) row = 0;
    if (row > rowCount()) row = rowCount();
    if (count < 1)
       return false;
    beginInsertRows(parent, row, last - 1);
    for (int i = 0; i < count; i++) {
       rootItem->insertChildAt(row, insertItem);
    } 
    endInsertRows();
    return true;
}
void DirModel::appendData(const QString &lines)
{
    int count = rowCount();
    //setupModelData(lines, rootItem);
    beginInsertRows(QModelIndex(), count, count + 1);
    //for (int i = 0; i < count; i++) {
    //   rootItem->insertChildAt(row, insertItem);
    //} 
    endInsertRows();
}
void DirModel::goTo(QDir *dir)
{
    //removeRows(0, count(), QModelIndex());
    setupModelData(dir, rootItem);
    qDebug() << "change dir";
}
void DirModel::setupModelData(QDir *dir, DirItem *parent)
{
    dir->makeAbsolute();
    QStringList entryList = dir->entryList();
    for (int i = 0; i < entryList.size(); i++) {
       QString it = entryList.at(i);
       if (it.compare(".") > 0) {
          if (!(dir->path().compare("//") == 0 && 
                it.compare("..") == 0)) {
              QList<QVariant> columnData;
              QString path = dir->path() + "/" +  entryList.at(i);
              QStringList info = fileType(path);
              QString type = info.at(0);
              //cout << "path=" << path.toStdString()
              //        << "type=" << type.toStdString() << endl ;
              if (type.compare("Unknown") == 0) {
                   //do not show it
              }
              else {
                 columnData << it;
                 if (type.compare("Table") == 0) {
                 try {
                    Table tbl(path.toStdString());
                    TableInfo tblinfo = tbl.tableInfo();
                    String info = tblinfo.type();
                    //cout << "info=" << info << endl;
                    //cout << "info=" << info << endl;
                    for (unsigned int i = 0; i < info.length(); i++) {
                       type[i] = info[i];
                     }
                  }
                  catch (...) {
                     type = "Bad Table";
                  }
                }
                //else if (type.compare("Miriad Image") == 0)
                //{
                //    cout << "miriad" << endl;
                //}
                if (type.compare("IERSe") == 0) {
                    type ="IERS";
                }
                columnData << type;
                columnData << info.at(1);
                columnData << info.at(2);
                parent->appendChild(new DirItem(columnData, parent));
             }
           }
        }
    }
    QSettings settings("NRAO", "casa");
    //cout << "dir_.path()=" << dir_.path().toStdString() << endl;
    settings.setValue("prevDir", dir->path());
        
}
QStringList DirModel::fileType(const QString pathName) {
    QFileInfo fileInfo(pathName);
    QStringList info;
    QString result = "Unknown";
    if (fileInfo.isSymLink())
    {
        result = "SymLink";
    }
    else if (!fileInfo.exists())
    {
        result = "Invalid";
    }
    else if (fileInfo.isFile())
    {
        //qDebug() << "pathName=" << pathName << " is File";
        QFile file(pathName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
           char buf[1024];
           qint64 lineLength = file.read(buf, sizeof(buf));
           if (lineLength > 1000) {
             QString line(buf);
             //cout << "line=" << line.toStdString() << endl;
              if (line.remove(' ').contains("SIMPLE=T"))
              {
                  result = "FITS";
              }
           }
           else if (pathName.endsWith(".py", Qt::CaseInsensitive)) {
              result = "Python";       
           }
           else if (pathName.endsWith(".jpg", Qt::CaseInsensitive) ||
                    pathName.endsWith(".xpm", Qt::CaseInsensitive) ||
                    pathName.endsWith(".xbm", Qt::CaseInsensitive) ||
                    pathName.endsWith(".ppm", Qt::CaseInsensitive) ||
                    pathName.endsWith(".jpeg", Qt::CaseInsensitive) ||
                    pathName.endsWith(".png", Qt::CaseInsensitive)) {
              result = "Graphics";
           }
           else if (pathName.endsWith(".pdf", Qt::CaseInsensitive)) {
              result = "PDF";
           }
           else if (pathName.endsWith(".ps", Qt::CaseInsensitive) ||
                    pathName.endsWith(".eps", Qt::CaseInsensitive)) {
              result = "Postscript";
           }
           else if (pathName.endsWith(".rpm", Qt::CaseInsensitive)) { 
              result = "rpm";
           }
           else if (pathName.endsWith(".gz", Qt::CaseInsensitive)) { 
              result = "gnu zip";
           }
           else if (pathName.endsWith(".rar", Qt::CaseInsensitive)) { 
              result = "rar";
           }
           else if (pathName.endsWith(".tar", Qt::CaseInsensitive)) { 
              result = "tar";
           }
           else if (pathName.endsWith(".jar", Qt::CaseInsensitive)) { 
              result = "jar";
           }
           else if (pathName.endsWith(".b2", Qt::CaseInsensitive)) { 
              result = "b2";
           }
           else {
              result = "Regular File";
           }
        }
    }
    else if (fileInfo.isDir())
    {
        while (result == "Unknown") {
           QFileInfo tab(pathName + "/table.dat");
           if (tab.isFile()) {
               result = "Table";
               break;
           }

           QFileInfo hd(pathName,  "header");
           QFileInfo imt(pathName +  "/image" );
           if (hd.isFile() && imt.exists()) {
               result = "Miriad Image";
               break;
           }
           QFileInfo vis(pathName + "/visdata" );
           if (hd.isFile() && vis.exists()) {
               result = "Miriad Vis";
               break;
           }
           else {
               result = "Directory";
               break;
           }
        }
    }
    else {}
    //qDebug() << "result=" << result;
    info << result;
    info << QString("%1").arg(fileInfo.size());
    //qDebug() << "size=" << QString("%1").arg(fileInfo.size());
    info << fileInfo.lastModified().toString();
    //qDebug() << "last=" << fileInfo.lastModified().toString();
    return info;
}
                                                       
bool DirModel::insert(int row, const QString &msg)
{
   if (msg.isEmpty())
      return false;
   QList<QVariant> columnData;
   QDateTime dt = QDateTime::currentDateTime();
   columnData << dt.toString();
   columnData << "NOTE";
   columnData << QString(getenv("USER"));
   columnData << msg.trimmed();
   insertItem = new DirItem(columnData, rootItem);
   return insertRow(row, QModelIndex()); 
}

void DirModel::writeData(QTextStream &os){
   QModelIndex idx = QModelIndex(); //=createIndex(1, 0, rootItem);
   for (int i = 0; i < rowCount(); i++) {
      os << data(index(i, 0, idx), Qt::DisplayRole).toString(); 
      os << "    ";
      os << data(index(i, 1, idx), Qt::DisplayRole).toString(); 
      os << " ";
      os << data(index(i, 2, idx), Qt::DisplayRole).toString(); 
      os << ":\n";
      os << data(index(i, 3, idx), Qt::DisplayRole).toString(); 
      os << "\n";
      os << "\n";
   }
}

QString DirModel::stringData(){
   QString str = "";
   QModelIndex idx = QModelIndex(); //createIndex(1, 0, rootItem);
   for (int i = 0; i < rowCount(); i++) {
      str += data(index(i, 0, idx), Qt::DisplayRole).toString(); 
      str += "    ";
      str += data(index(i, 1, idx), Qt::DisplayRole).toString(); 
      str += " ";
      str += data(index(i, 2, idx), Qt::DisplayRole).toString(); 
      str += ":\n";
      str += data(index(i, 3, idx), Qt::DisplayRole).toString(); 
      str += "\n";
      str += "\n";
   }
   return str;
}

QString DirModel::stringData(int i){
   QString str = "";
   QModelIndex idx = QModelIndex();
   str += data(index(i, 0, idx), Qt::DisplayRole).toString(); 
   str += "    ";
   str += data(index(i, 1, idx), Qt::DisplayRole).toString(); 
   str += " ";
   str += data(index(i, 2, idx), Qt::DisplayRole).toString(); 
   str += ":\n";
   str += data(index(i, 3, idx), Qt::DisplayRole).toString(); 
   str += "\n";
   str += "\n";
   return str;
}

}
