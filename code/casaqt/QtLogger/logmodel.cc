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

#include <casaqt/QtLogger/logitem.h>
#include <casaqt/QtLogger/logmodel.qo.h>

#include <graphics/X11/X_enter.h>
#include <QtGui>
#include <QDebug>
#include <QDateTime>
#include <QFont>
#include <QSettings>
#include <sys/time.h>
#include <iostream>
#include <graphics/X11/X_exit.h>


namespace casa { 

LogModel::LogModel(const QString &data, QObject *parent)
    : QAbstractItemModel(parent), insertItem(0), searchKey(""),
      fontSize(8)
{
    QSettings settings("NRAO", "casa");
    fontSize = settings.value("logFont", 12).toInt();

    QList<QVariant> rootData;
    rootData << "Time" << "Priority" 
             << "Origin" << "Message"; 
    rootItem = new LogItem(rootData);
    //qDebug() << "data=" <<  data;
    //qDebug() << "byte=" << data.count();

	appendData(data);
}

LogModel::~LogModel()
{
    delete rootItem;
    delete insertItem;
}
int LogModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<LogItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    LogItem *item = static_cast<LogItem*>(index.internalPointer());
    QVariant vt = item->data(index.column());
    if (role == Qt::TextColorRole) {
        if (item->data(1).toString() == "SEVERE")
           return QVariant(QColor(Qt::red));
        if (item->data(1).toString() == "WARN")
           return QVariant(QColor(255, 0, 0, 127));
        if (item->data(1).toString() == "NOTE")
           return QVariant(QColor(0, 0, 255, 127));
    }
    if (role == Qt::BackgroundColorRole) {
        QString msg = item->data(3).toString();
        if (searchKey != "" && msg != "" && msg.contains(searchKey)) {
           return QVariant(QColor(Qt::green));
        }
    }
    if (role == Qt::FontRole) {
		QFont monospace("Courier",fontSize, QFont::Bold);
		monospace.setFixedPitch(true);
		monospace.setStyleHint(QFont::TypeWriter);
        return QVariant(monospace);
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    return vt;
}

Qt::ItemFlags LogModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled 
           //| Qt::ItemIsEditable
           | Qt::ItemIsSelectable; 
}

QVariant LogModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex LogModel::index(int row, int column, const QModelIndex &parent)
            const
{
    LogItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<LogItem*>(parent.internalPointer());

    LogItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex LogModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();
    //qDebug() << "index valid";

    LogItem *childItem = static_cast<LogItem*>(index.internalPointer());
    LogItem *parentItem = childItem->parent();

    if (parentItem == rootItem) {
        //qDebug() << "root index";
        return QModelIndex();
    }

    //qDebug() << "child index";
    return createIndex(parentItem->row(), 0, parentItem);
}

int LogModel::rowCount(const QModelIndex &parent) const
{
    LogItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<LogItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void LogModel::searchKeyChanged(const QString &key) 
{
    searchKey = key;
    beginInsertRows(QModelIndex(), 0, 0);
    //qDebug() << "searchKey" << searchKey;
    endInsertRows();
}

bool LogModel::removeRows (int row, int count, const QModelIndex &parent){
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

bool LogModel::insertRows (int row, int count, const QModelIndex &parent){
    //qDebug() << "child=" << rootItem->childCount() 
    //         << " insert rows=" << row << " count=" << count;
    if (row < 0) row = 0;
    int last = row + count;
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

void LogModel::appendData(const QString &data)
{
	// bwaters: change the thing to tab-delimited single lines
	QStringList dataLines = data.split("\n");
	
	const int count = dataLines.size();
	
	if(count < 1)
		return;
	// else

	const int row   = rowCount();
	const int last  = row + count;
	
	QStringListIterator lines(dataLines);

	beginInsertRows(QModelIndex(), row, last - 2);

	
	while (lines.hasNext())
	{
		QString line = lines.next();
		
		//qDebug() << line << "\n";
		
		QStringList items = line.split("\t", QString::SkipEmptyParts);
		
		// qDebug() << items.size() << "\n";
		
		if( items.size() > 1) {
			QString date 	= items.at(0);
			QString priority= items.at(1);
			QString origin;
			if (items.size() > 3) {
				origin = items.at(2);
			}
                        if (items.size() == 3) {
                             if (items.at(2).contains("::")){
				origin = items.at(2);
                             }
                             else {
				origin = " ";
                             }
                        }

            //use textelide property instead
            //QString dt = date.right(8).append(' ').append(date.left(10));
            //date = dt;

			// qDebug() << "date=" << date << "\t";
			// qDebug() << "priority=" << priority << "\t";
			// qDebug() << "origin=" << origin << "\t";
			
			QList<QVariant> columnData;
			columnData << date << priority << origin;

			if(items.size() == 3) {
				QString message = items.at(2);
                                if (items.at(2).contains("::")){
			        	message = "   ";
                                }
                                else {
				        message.replace('\r','\n').truncate(2500);
                                }
				columnData << message;
				// qDebug() << message << "\t";
			}
			if(items.size() > 3) {
				QString message = items.at(3);

				// one quirk: multi-line messages are 'encoded' with \r
				// in place of newline (\n) characters, we should put these back
				message.replace('\r','\n').truncate(2500);
				columnData << message;
				// qDebug() << message << "\t";
			}
			
			// qDebug() << "=========\n";
			
			insertItem = new LogItem(columnData, rootItem);
			if(insertItem)
				rootItem->appendChild(insertItem);
		}
	}
	endInsertRows();
}

void LogModel::setupModelData(const QString &lines, LogItem */*parent*/)
{
	appendData(lines);
}

bool LogModel::insert(int row, const QString &msg)
{
   if (msg.isEmpty())
      return false;
   QList<QVariant> columnData;
   QDateTime dt = QDateTime::currentDateTime().toUTC();
   columnData << dt.toString(Qt::ISODate).replace("T", " ");
   columnData << "NOTE";
   columnData << QString(getenv("USER"));
   columnData << msg.trimmed();
   insertItem = new LogItem(columnData, rootItem);
   return insertRow(row, QModelIndex()); 
}

void LogModel::writeData(QTextStream &os){
   QModelIndex idx = QModelIndex(); //=createIndex(1, 0, rootItem);
   for (int i = 0; i < rowCount(); i++) {
      os << data(index(i, 0, idx), Qt::DisplayRole).toString(); 
      os << " ";
      //os << "\t";
      os << data(index(i, 1, idx), Qt::DisplayRole).toString(); 
      os << " ";
      //os << "\t";
      os << data(index(i, 2, idx), Qt::DisplayRole).toString().split(':')[0]; 
      os << "\t";
      // Split data at space and pad with spaces if the message
      // string is too long
      os << data(index(i, 3, idx), Qt::DisplayRole).toString(); 
      os << "\n";
   }
}

QString LogModel::stringData(){
   QString str = "";
   QModelIndex idx = QModelIndex(); //createIndex(1, 0, rootItem);
   for (int i = 0; i < rowCount(); i++) {
      str += data(index(i, 0, idx), Qt::DisplayRole).toString(); 
      str += " ";
      //str += "\t";
      str += data(index(i, 1, idx), Qt::DisplayRole).toString(); 
      str += " ";
      //str += "\t";
      str += data(index(i, 2, idx), Qt::DisplayRole).toString().split(':')[0]; 
      str += "\t";
      // Split data at space and pad with spaces if the message
      // string is too long
      str += data(index(i, 3, idx), Qt::DisplayRole).toString(); 
      str += "\n";
   }
   return str;
}

QString LogModel::stringData(int i){
   QString str = "";
   QModelIndex idx = QModelIndex();
   str += data(index(i, 0, idx), Qt::DisplayRole).toString(); 
      str += " ";
   //str += "\t";
   str += data(index(i, 1, idx), Qt::DisplayRole).toString(); 
      str += " ";
   //str += "\t";
   str += data(index(i, 2, idx), Qt::DisplayRole).toString().split(':')[0]; 
   str += "\t";
   str += data(index(i, 3, idx), Qt::DisplayRole).toString(); 
   str += "\n";
   return str;
}

void LogModel::changeFont(int i){
   if (i < 0 && fontSize > 6) fontSize--;
   if (i > 0 && fontSize < 24) fontSize++;
   if (i == 0) fontSize = 12;
   beginInsertRows(QModelIndex(), 0, 0);
   endInsertRows();

   QSettings settings("NRAO", "casa");
   settings.setValue("logFont", fontSize);
}

int LogModel::getFontSize()
{
   return fontSize;
}

}
