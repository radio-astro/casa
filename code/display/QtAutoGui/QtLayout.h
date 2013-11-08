//# QtLayout.h: Qt definition of the flow-layout and borderlayout.
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


#ifndef QTFLOWLAYOUT_H
#define QTFLOWLAYOUT_H

#include <casa/aips.h>

#include <graphics/X11/X_enter.h>
#  include <QLayout>
#  include <QRect>
#  include <QWidgetItem>
#include <graphics/X11/X_exit.h>


namespace casa { //# NAMESPACE CASA - BEGIN



	class QtFlowLayout : public QLayout {
	public:
		QtFlowLayout(QWidget *parent, int margin = 0, int spacing = -1);
		QtFlowLayout(int spacing = -1);
		~QtFlowLayout();

		void addItem(QLayoutItem *item);
		Qt::Orientations expandingDirections() const;
		bool hasHeightForWidth() const;
		int heightForWidth(int) const;
		int count() const;
		QLayoutItem *itemAt(int index) const;
		QSize minimumSize() const;
		void setGeometry(const QRect &rect);
		QSize sizeHint() const;
		QLayoutItem *takeAt(int index);

	private:
		int doLayout(const QRect &rect, bool testOnly) const;

		QList<QLayoutItem *> itemList;
	};


	class QtBorderLayout : public QLayout {
	public:
		enum Position { West, North, South, East, Center };

		QtBorderLayout(QWidget *parent, int margin = 0, int spacing = -1);
		QtBorderLayout(int spacing = -1);
		~QtBorderLayout();

		void addItem(QLayoutItem *item);
		void addWidget(QWidget *widget, Position position);
		Qt::Orientations expandingDirections() const;
		bool hasHeightForWidth() const;
		int count() const;
		QLayoutItem *itemAt(int index) const;
		QSize minimumSize() const;
		void setGeometry(const QRect &rect);
		QSize sizeHint() const;
		QLayoutItem *takeAt(int index);

		void add(QLayoutItem *item, Position position);

	private:
		struct ItemWrapper {
			ItemWrapper(QLayoutItem *i, Position p) {
				item = i;
				position = p;
			}

			QLayoutItem *item;
			Position position;
		};

		enum SizeType { MinimumSize, SizeHint };
		QSize calculateSize(SizeType sizeType) const;

		QList<ItemWrapper *> list;
	};

} //# NAMESPACE CASA - END

#endif
