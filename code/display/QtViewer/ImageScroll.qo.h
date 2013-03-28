#ifndef IMAGESCROLL_QO_H
#define IMAGESCROLL_QO_H

#include <QtGui/QScrollArea>
#include <display/QtViewer/ImageScroll.ui.h>

class QDragMoveEvent;
class QDragEnterEvent;
class QDropEvent;

namespace casa {

class ImageView;
class QtDisplayData;

class ImageScroll : public QScrollArea
{
    Q_OBJECT

public:
    ImageScroll(QWidget *parent = 0);
    QList<ImageView*> getSelectedViews();
    void addImage( QtDisplayData* data );
    void closeImages();
    void addImageViews( QList<ImageView*>& views );
    void removeImageViews( QList<ImageView*>& view );
    ~ImageScroll();



protected:
	void dragEnterEvent( QDragEnterEvent* enterEvent );
    void dropEvent( QDropEvent* dropEvent );
    void dragMoveEvent( QDragMoveEvent* dragMoveEvent );

private:
    void resetScroll();
    QList<ImageView*> images;
    Ui::ImageScrollClass ui;
};

}

#endif // IMAGESCROLL_QO_H
