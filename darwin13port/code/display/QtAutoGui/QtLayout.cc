//# QtLayout.cc: Qt implementation of flow-layout and borderlayout.
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
//#

#include <display/QtAutoGui/QtLayout.h>

#include <graphics/X11/X_enter.h>
#  include <QtGui>
#include <graphics/X11/X_exit.h>


namespace casa { //# NAMESPACE CASA - BEGIN


	QtFlowLayout::QtFlowLayout(QWidget *parent, int margin, int spacing)
		: QLayout(parent) {
		setMargin(margin);
		setSpacing(spacing);
	}

	QtFlowLayout::QtFlowLayout(int spacing) {
		setSpacing(spacing);
	}

	QtFlowLayout::~QtFlowLayout() {
		QLayoutItem *item;
		while ((item = takeAt(0)))
			delete item;
	}

	void QtFlowLayout::addItem(QLayoutItem *item) {
		itemList.append(item);
	}

	int QtFlowLayout::count() const {
		return itemList.size();
	}

	QLayoutItem *QtFlowLayout::itemAt(int index) const {
		return itemList.value(index);
	}

	QLayoutItem *QtFlowLayout::takeAt(int index) {
		if (index >= 0 && index < itemList.size())
			return itemList.takeAt(index);
		else
			return 0;
	}

	Qt::Orientations QtFlowLayout::expandingDirections() const {
		return 0;
	}

	bool QtFlowLayout::hasHeightForWidth() const {
		return true;
	}

	int QtFlowLayout::heightForWidth(int width) const {
		int height = doLayout(QRect(0, 0, width, 0), true);
		return height;
	}

	void QtFlowLayout::setGeometry(const QRect &rect) {
		QLayout::setGeometry(rect);
		doLayout(rect, false);
	}

	QSize QtFlowLayout::sizeHint() const {
		return minimumSize();
	}

	QSize QtFlowLayout::minimumSize() const {
		QSize size;
		QLayoutItem *item;
		foreach (item, itemList)
		size = size.expandedTo(item->minimumSize());

		size += QSize(2*margin(), 2*margin());
		return size;
	}

	int QtFlowLayout::doLayout(const QRect &rect, bool testOnly) const {
		int x = rect.x();
		int y = rect.y();
		int lineHeight = 0;

		QLayoutItem *item;
		foreach (item, itemList) {
			int nextX = x + item->sizeHint().width() + spacing();
			if (nextX - spacing() > rect.right() && lineHeight > 0) {
				x = rect.x();
				y = y + lineHeight + spacing();
				nextX = x + item->sizeHint().width() + spacing();
				lineHeight = 0;
			}

			if (!testOnly)
				item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

			x = nextX;
			lineHeight = qMax(lineHeight, item->sizeHint().height());
		}
		return y + lineHeight - rect.y();
	}



	QtBorderLayout::QtBorderLayout(QWidget *parent, int margin, int spacing)
		: QLayout(parent) {
		setMargin(margin);
		setSpacing(spacing);
	}

	QtBorderLayout::QtBorderLayout(int spacing) {
		setSpacing(spacing);
	}


	QtBorderLayout::~QtBorderLayout() {
		QLayoutItem *l;
		while ((l = takeAt(0)))
			delete l;
	}

	void QtBorderLayout::addItem(QLayoutItem *item) {
		add(item, West);
	}

	void QtBorderLayout::addWidget(QWidget *widget, Position position) {
		add(new QWidgetItem(widget), position);
	}

	Qt::Orientations QtBorderLayout::expandingDirections() const {
		return Qt::Horizontal | Qt::Vertical;
	}

	bool QtBorderLayout::hasHeightForWidth() const {
		return false;
	}

	int QtBorderLayout::count() const {
		return list.size();
	}

	QLayoutItem *QtBorderLayout::itemAt(int index) const {
		ItemWrapper *wrapper = list.value(index);
		if (wrapper)
			return wrapper->item;
		else
			return 0;
	}

	QSize QtBorderLayout::minimumSize() const {
		return calculateSize(MinimumSize);
	}

	void QtBorderLayout::setGeometry(const QRect &rect) {
		ItemWrapper *center = 0;
		int eastWidth = 0;
		int westWidth = 0;
		int northHeight = 0;
		int southHeight = 0;
		int centerHeight = 0;
		int i;

		QLayout::setGeometry(rect);

		for (i = 0; i < list.size(); ++i) {
			ItemWrapper *wrapper = list.at(i);
			QLayoutItem *item = wrapper->item;
			Position position = wrapper->position;

			if (position == North) {
				item->setGeometry(QRect(rect.x(), northHeight, rect.width(),
				                        item->sizeHint().height()));
				northHeight += item->geometry().height() + spacing();
			} else if (position == South) {
				item->setGeometry(QRect(item->geometry().x(),
				                        item->geometry().y(), rect.width(),
				                        item->sizeHint().height()));

				southHeight += item->geometry().height() + spacing();

				item->setGeometry(QRect(rect.x(),
				                        rect.y() + rect.height() - southHeight + spacing(),
				                        item->geometry().width(),
				                        item->geometry().height()));
			} else if (position == Center) {
				center = wrapper;
			}
		}

		centerHeight = rect.height() - northHeight - southHeight;

		for (i = 0; i < list.size(); ++i) {
			ItemWrapper *wrapper = list.at(i);
			QLayoutItem *item = wrapper->item;
			Position position = wrapper->position;

			if (position == West) {
				item->setGeometry(QRect(rect.x() + westWidth, northHeight,
				                        item->sizeHint().width(), centerHeight));

				westWidth += item->geometry().width() + spacing();
			} else if (position == East) {
				item->setGeometry(QRect(item->geometry().x(), item->geometry().y(),
				                        item->sizeHint().width(), centerHeight));

				eastWidth += item->geometry().width() + spacing();

				item->setGeometry(QRect(
				                      rect.x() + rect.width() - eastWidth + spacing(),
				                      northHeight, item->geometry().width(),
				                      item->geometry().height()));
			}
		}

		if (center)
			center->item->setGeometry(QRect(westWidth, northHeight,
			                                rect.width() - eastWidth - westWidth,
			                                centerHeight));
	}

	QSize QtBorderLayout::sizeHint() const {
		return calculateSize(SizeHint);
	}

	QLayoutItem *QtBorderLayout::takeAt(int index) {
		if (index >= 0 && index < list.size()) {
			ItemWrapper *layoutStruct = list.takeAt(index);
			return layoutStruct->item;
		}
		return 0;
	}

	void QtBorderLayout::add(QLayoutItem *item, Position position) {
		list.append(new ItemWrapper(item, position));
	}

	QSize QtBorderLayout::calculateSize(SizeType sizeType) const {
		QSize totalSize;

		for (int i = 0; i < list.size(); ++i) {
			ItemWrapper *wrapper = list.at(i);
			Position position = wrapper->position;
			QSize itemSize;

			if (sizeType == MinimumSize)
				itemSize = wrapper->item->minimumSize();
			else // (sizeType == SizeHint)
				itemSize = wrapper->item->sizeHint();

			if (position == North || position == South || position == Center)
				totalSize.rheight() += itemSize.height();

			if (position == West || position == East || position == Center)
				totalSize.rwidth() += itemSize.width();
		}
		return totalSize;
	}

} //# NAMESPACE CASA - END


