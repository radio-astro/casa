#ifndef TQFLOWLAYOUT_H
#define TQFLOWLAYOUT_H

#include <casa/aips.h>

#include <graphics/X11/X_enter.h>
#include <QLayout>
#include <QRect>
#include <QWidgetItem>
#include <graphics/X11/X_exit.h>


namespace casa { //# NAMESPACE CASA - BEGIN



class FlowLayout : public QLayout
    {
    public:
        FlowLayout(QWidget *parent, int margin = 0, int spacing = -1);
        FlowLayout(int spacing = -1);
        ~FlowLayout();

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


class BorderLayout : public QLayout
{
public:
    enum Position { West, North, South, East, Center };

    BorderLayout(QWidget *parent, int margin = 0, int spacing = -1);
    BorderLayout(int spacing = -1);
    ~BorderLayout();

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
    struct ItemWrapper
    {
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
