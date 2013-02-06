/*
 * SliceZoomer.h
 *
 *  Created on: Jan 22, 2013
 *      Author: slovelan
 */

#ifndef SLICEZOOMER_H_
#define SLICEZOOMER_H_

#include <qwt_plot_zoomer.h>

class QwtPlotCanvas;
class QKeyEvent;

namespace casa {

class SliceZoomer : public QwtPlotZoomer {

public:
	SliceZoomer( QwtPlotCanvas* plot);
	virtual ~SliceZoomer();
	void zoomIn();
	void zoomOut();
	void zoomNeutral();
	virtual void zoom( const QwtDoubleRect & );


protected:
	virtual void widgetMouseReleaseEvent( QMouseEvent* event );

private:
	bool zoomMode;
	QwtPlotCanvas* canvas;
	QPixmap zoomMap;
	QCursor* zoomCursor;

};

} /* namespace casa */
#endif /* SLICEZOOMER_H_ */
