#include <display3d/3DViewer/tqlayout.h>

#include <graphics/X11/X_enter.h>
#include <QtGui>
#include <graphics/X11/X_exit.h>


namespace casa {


    FlowLayout::FlowLayout(QWidget *parent, int margin, int spacing)
        : QLayout(parent)
    {
        setMargin(margin);
        setSpacing(spacing);
    }

    FlowLayout::FlowLayout(int spacing)
    {
        setSpacing(spacing);
    }

    FlowLayout::~FlowLayout()
    {
        QLayoutItem *item;
        while ((item = takeAt(0)))
            delete item;
    }

    void FlowLayout::addItem(QLayoutItem *item)
    {
        itemList.append(item);
    }

    int FlowLayout::count() const
    {
        return itemList.size();
    }

    QLayoutItem *FlowLayout::itemAt(int index) const
    {
        return itemList.value(index);
    }

    QLayoutItem *FlowLayout::takeAt(int index)
    {
        if (index >= 0 && index < itemList.size())
            return itemList.takeAt(index);
        else
            return 0;
    }

    Qt::Orientations FlowLayout::expandingDirections() const
    {
        return 0;
    }

    bool FlowLayout::hasHeightForWidth() const
    {
        return true;
    }

    int FlowLayout::heightForWidth(int width) const
    {
        int height = doLayout(QRect(0, 0, width, 0), true);
        return height;
    }

    void FlowLayout::setGeometry(const QRect &rect)
    {
        QLayout::setGeometry(rect);
        doLayout(rect, false);
    }

    QSize FlowLayout::sizeHint() const
    {
        return minimumSize();
    }

    QSize FlowLayout::minimumSize() const
    {
        QSize size;
        QLayoutItem *item;
        foreach (item, itemList)
            size = size.expandedTo(item->minimumSize());

        size += QSize(2*margin(), 2*margin());
        return size;
    }

    int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
    {
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



BorderLayout::BorderLayout(QWidget *parent, int margin, int spacing)
    : QLayout(parent)
{
    setMargin(margin);
    setSpacing(spacing);
}

BorderLayout::BorderLayout(int spacing)
{
    setSpacing(spacing);
}


BorderLayout::~BorderLayout()
{
    QLayoutItem *l;
    while ((l = takeAt(0)))
        delete l;
}

void BorderLayout::addItem(QLayoutItem *item)
{
    add(item, West);
}

void BorderLayout::addWidget(QWidget *widget, Position position)
{
    add(new QWidgetItem(widget), position);
}

Qt::Orientations BorderLayout::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

bool BorderLayout::hasHeightForWidth() const
{
    return false;
}

int BorderLayout::count() const
{
    return list.size();
}

QLayoutItem *BorderLayout::itemAt(int index) const
{
    ItemWrapper *wrapper = list.value(index);
    if (wrapper)
        return wrapper->item;
    else
        return 0;
}

QSize BorderLayout::minimumSize() const
{
    return calculateSize(MinimumSize);
}

void BorderLayout::setGeometry(const QRect &rect)
{
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

QSize BorderLayout::sizeHint() const
{
    return calculateSize(SizeHint);
}

QLayoutItem *BorderLayout::takeAt(int index)
{
    if (index >= 0 && index < list.size()) {
        ItemWrapper *layoutStruct = list.takeAt(index);
        return layoutStruct->item;
    }
    return 0;
}

void BorderLayout::add(QLayoutItem *item, Position position)
{
    list.append(new ItemWrapper(item, position));
}

QSize BorderLayout::calculateSize(SizeType sizeType) const
{
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


