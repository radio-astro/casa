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


#include <casa/aips.h>
#include <casa/Containers/Record.h>

#include <display/QtPlotter/QtDrawCanvas.qo.h>
#include <display/QtPlotter/QtDrawColors.qo.h>

#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QColor>
#include <QHash>
#include <QWidget>
#include <QMouseEvent>
#include <cmath>
#include <QtGui>
#include <iostream>
#include <graphics/X11/X_exit.h>

namespace casa {

	QtDrawCanvas::~QtDrawCanvas()
	{}

	QtDrawCanvas::QtDrawCanvas(QWidget *parent)
		: QWidget(parent),
		  title(), xLabel(), yLabel(), welcome()
		  //, lineType(LINE)
	{
		curveMap.clear();
		setAttribute(Qt::WA_NoBackground);
		setBackgroundRole(QPalette::Dark);
		setForegroundRole(QPalette::Light);
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		setFocusPolicy(Qt::StrongFocus);
		rubberBandIsShown = false;
		imageMode = false;

		setPlotSettings(QtDrawSettings());

	}



	void QtDrawCanvas::setPlotSettings(const QtDrawSettings &settings) {
		zoomStack.resize(1);
		zoomStack[0] = settings;
		curZoom = 0;

		curMarker = 0;
		refreshPixmap();
	}

	void QtDrawCanvas::zoomOut() {
		if (curZoom > 0) {
			--curZoom;
			refreshPixmap();
			emit zoomChanged();
		}
	}

	void QtDrawCanvas::zoomIn() {
		if (curZoom < (int)zoomStack.size() - 1) {
			++curZoom;
			refreshPixmap();
			emit zoomChanged();
		}
	}

	void QtDrawCanvas::markNext() {

		//cout << " curMarker=" << curMarker
		//<< " size=" << (int)markerStack.size() << endl;
		if (markMode && curMarker < (int)markerStack.size() - 1) {
			++curMarker;
		}
		refreshPixmap();
	}

	void QtDrawCanvas::markPrev() {
		//cout << " --------------curMarker=" << curMarker
		// << " size=" << (int)markerStack.size() << endl;
		if (markMode && curMarker > 0) {
			--curMarker;
			refreshPixmap();
		}
	}

	void QtDrawCanvas::setCurveData(int id, const CurveData data,
	                                int tp, int cl) {
		//std::cout << "id=" << id
		//          << " tp=" << tp
		//          << " cl=" << cl <<std::endl;
		std::map<int, CurveData>::iterator it;
		if (id == 0) {
			it = curveMap.begin();
			while (it != curveMap.end()) {
				int key = (*it).first;
				curveMap.erase(key);
				typeMap.erase(key);
				colorMap.erase(key);
				++it;
			}
			curveMap[1] = data;
			typeMap[1] = tp;
			colorMap[1] = cl;
		} else {
			int absid = abs(id);
			int k = 0;
			it = curveMap.begin();
			while (it != curveMap.end()) {
				if ((*it).first == absid) {
					k = absid;
					break;
				}
				++it;
			}

			if (k < 1) {
				//std::cout << "absid=" << absid
				//          << " tp=" << tp
				//          << " cl=" << cl <<std::endl;
				curveMap[absid] = data;
				typeMap[absid] = tp;
				colorMap[absid] = cl;
			} else {
				if (id < 0) {
					curveMap.erase(absid);
					typeMap.erase(absid);
					colorMap.erase(absid);
				} else {
					for (uInt a = 0; a < data.size(); a++) {
						curveMap[k].push_back(data[a]);
					}
					typeMap[k] = tp;
					colorMap[k] = cl;
				}
			}
		}
		/*
		std::map<int, int>::iterator iter = typeMap.begin();
		while (iter != typeMap.end()) {
		    std::cout << "line=" << iter->first
		              << " type=" << iter->second
		              << " " << typeMap[iter->first] << std::endl;
		    ++iter;
		}
		*/


		//refreshPixmap();
	}

	void QtDrawCanvas::setDataRange() {
		double xmin = 1000000000000000000000000.;
		double xmax = -xmin;
		double ymin = 1000000000000000000000000.;
		double ymax = -ymin;
		std::map<int, CurveData>::iterator it = curveMap.begin();
		while (it != curveMap.end()) {
			const CurveData &data = (*it).second;
			int maxPoints = data.size() / 2;

			for (int i = 0; i < maxPoints; ++i) {
				double dx = data[2 * i];
				double dy = data[2 * i + 1];
				xmin = (xmin > dx) ? dx : xmin;
				xmax = (xmax < dx) ? dx : xmax;
				ymin = (ymin > dy) ? dy : ymin;
				ymax = (ymax < dy) ? dy : ymax;
			}
			++it;
		}

		QtDrawSettings settings;
		if (xmax == xmin) {
			xmax = xmax + 0.5;
			xmin = xmin - 0.5;
		}
		if (ymax == ymin) {
			ymax = ymax + 0.5;
			ymin = ymin - 0.5;
		}
		settings.minX = xmin;
		settings.maxX = xmax;
		settings.minY = ymin;
		settings.maxY = ymax;
		settings.adjust();
		setPlotSettings(settings);

	}

	void QtDrawCanvas::clearCurve(int id) {
		curveMap.erase(id);
		refreshPixmap();
	}

	QSize QtDrawCanvas::minimumSizeHint() const {
		return QSize(4 * Margin, 4 * Margin);
	}

	QSize QtDrawCanvas::sizeHint() const {
		return QSize(8 * Margin, 6 * Margin);
	}

	void QtDrawCanvas::paintEvent(QPaintEvent *event) {
		QPainter painter(this);

		QVector<QRect> rects = event->region().rects();
		for (int i = 0; i < (int)rects.size(); ++i)
			painter.drawPixmap(rects[i], pixmap, rects[i]);

		//painter.drawPixmap(0, 0, pixmap);

		if (rubberBandIsShown) {
			painter.setPen(Qt::yellow);
			painter.drawRect(rubberBandRect.normalized());
		}
		if (hasFocus()) {
			QStyleOptionFocusRect option;
			option.init(this);
			option.backgroundColor =
			    palette().color(QPalette::Background);
			style()->drawPrimitive(QStyle::PE_FrameFocusRect,
			                       &option, &painter, this);
		}

	}

	void QtDrawCanvas::resizeEvent(QResizeEvent *) {
		refreshPixmap();
	}

	void QtDrawCanvas::mousePressEvent(QMouseEvent *event) {
		if (event->button() == Qt::LeftButton) {
			rubberBandIsShown = true;
			rubberBandRect.setTopLeft(event->pos());
			rubberBandRect.setBottomRight(event->pos());
			updateRubberBandRegion();
			setCursor(Qt::CrossCursor);
		}
		if (event->button() == Qt::RightButton) {
			int x = event->pos().x() - Margin;
			int y = event->pos().y() - Margin;
			//cout << "x=" << x << " y=" << y << endl;
			QtDrawSettings prevSettings = zoomStack[curZoom];
			QtDrawSettings settings;

			double dx = prevSettings.spanX()
			            / (width() - 2 * Margin);
			double dy = prevSettings.spanY()
			            / (height() - 2 * Margin);
			x = (int)(prevSettings.minX + dx * x);
			y = (int)(prevSettings.maxY - dy * y);
			//cout << "x=" << x << " y=" << y << endl;

			std::map<int, CurveData>::iterator
			it = markerStack.begin();

			while (it != markerStack.end()) {
				int id = (*it).first;
				const CurveData &data = (*it).second;
				//cout << " " << data[0] << " " << data[2]
				//        << " " << data[1] << " " << data[3] << endl;
				if (x >= data[0] && x < data[2] &&
				        y <= data[1] && y > data[3]) {
					markerStack[id] =
					    markerStack[markerStack.size() - 1];
					markerStack.erase(markerStack.size() - 1);
					refreshPixmap();
					break;
				}
				++it;
			}
		}
	}

	void QtDrawCanvas::mouseMoveEvent(QMouseEvent *event) {
		if (event->buttons() & Qt::LeftButton) {
			updateRubberBandRegion();
			rubberBandRect.setBottomRight(event->pos());
			updateRubberBandRegion();
		}
	}

	void QtDrawCanvas::mouseReleaseEvent(QMouseEvent *event) {
		//qDebug() << "mouse release" << event->button()
		//         << Qt::LeftButton;
		if (event->button() == Qt::LeftButton) {
			rubberBandIsShown = false;
			updateRubberBandRegion();
			unsetCursor();

			QRect rect = rubberBandRect.normalized();

			//qDebug() << "rect: left" << rect.left()
			//         << "right" << rect.right()
			//         << "top" << rect.top()
			//         << "bottom" << rect.bottom()
			//         << "width" << width()
			//         << "height" << height()
			//         << "margin" << Margin;
			//zoom only if zoom box is in the plot region
			if (rect.left() < Margin || rect.top() < Margin ||
			        rect.right() > width() - Margin ||
			        rect.bottom() > height() -  Margin)
				return;
			//qDebug() << "inside";

			if (rect.width() < 4 || rect.height() < 4)
				return;
			//qDebug() << "big enough";

			//cout << "numCurves " << curveMap.size() << endl;
			//if (curveMap.size() == 0)
			//    return;

			rect.translate(-Margin, -Margin);

			QtDrawSettings prevSettings = zoomStack[curZoom];
			QtDrawSettings settings;

			double dx = prevSettings.spanX()
			            / (width() - 2 * Margin);
			double dy = prevSettings.spanY()
			            / (height() - 2 * Margin);
			settings.minX = prevSettings.minX + dx * rect.left();
			settings.maxX = prevSettings.minX + dx * rect.right();
			settings.minY = prevSettings.maxY - dy * rect.bottom();
			settings.maxY = prevSettings.maxY - dy * rect.top();
			settings.adjust();
			if (!markMode) {
				if (curveMap.size() != 0) {
					//qDebug() << "zoomin ";
					zoomStack.resize(curZoom + 1);
					zoomStack.push_back(settings);
					zoomIn();
				}
			} else {
				//qDebug() << "mark ";
				CurveData data;
				data.push_back(settings.minX);
				data.push_back(settings.maxY);
				data.push_back(settings.maxX);
				data.push_back(settings.minY);

				//cout << " minX=" << (int)settings.minX
				//<< " maxX=" << (int)settings.maxX
				//<< " minY=" <<  (int)settings.minY
				//<< " maxY=" << (int)settings.maxY << endl;
				markerStack[markerStack.size()] = data;
				markNext();
			}
		}
	}

	void QtDrawCanvas::keyPressEvent(QKeyEvent *event) {
		if (!imageMode)
			switch (event->key()) {
			case Qt::Key_Plus:
				zoomIn();
				break;
			case Qt::Key_Minus:
				zoomOut();
				break;
			case Qt::Key_Left:
				zoomStack[curZoom].scroll(-1, 0);
				refreshPixmap();
				break;
			case Qt::Key_Right:
				zoomStack[curZoom].scroll(+1, 0);
				refreshPixmap();
				break;
			case Qt::Key_Down:
				zoomStack[curZoom].scroll(0, -1);
				refreshPixmap();
				break;
			case Qt::Key_Up:
				zoomStack[curZoom].scroll(0, +1);
				refreshPixmap();
				break;
			default:
				QWidget::keyPressEvent(event);
			}
	}

	void QtDrawCanvas::focusInEvent(QFocusEvent * /*event*/) {
		emit gotFocus();
		//refreshPixmap();
	}

	void QtDrawCanvas::wheelEvent(QWheelEvent *event) {
		int numDegrees = event->delta() / 8;
		int numTicks = numDegrees / 15;

		if (event->orientation() == Qt::Horizontal)
			zoomStack[curZoom].scroll(numTicks, 0);
		else
			zoomStack[curZoom].scroll(0, numTicks);
		refreshPixmap();
	}

	void QtDrawCanvas::updateRubberBandRegion() {
		// QRect rect = rubberBandRect.normalized();
		// update(rect.left(), rect.top(), rect.width(), 1);
		// update(rect.left(), rect.top(), 1, rect.height());
		// update(rect.left(), rect.bottom(), rect.width(), 1);
		// update(rect.right(), rect.top(), 1, rect.height());
		update();
	}

	void QtDrawCanvas::refreshPixmap() {

		if (zoomStack[curZoom].numXTicks < 2)
			setPlotSettings(QtDrawSettings());

		pixmap = QPixmap(size());
		pixmap.fill(this, 0, 0);
		QPainter painter(&pixmap);

		drawLabels(&painter);
		if (!imageMode) {
			drawGrid(&painter);
			drawCurves(&painter);
		} else {
			drawBackBuffer(&painter);
			drawTicks(&painter);
			if (markMode)
				drawRects(&painter);
		}
		if (welcome.text != "")
			drawWelcome(&painter);
		update();
	}

	void QtDrawCanvas::drawBackBuffer(QPainter *painter) {
		QRect rect(Margin, Margin,
		           width() - 2 * Margin, height() - 2 * Margin);
		QtDrawSettings settings = zoomStack[curZoom];
		QRect src((int)settings.minX, (int)settings.minY,
		          (int)settings.maxX, (int)settings.maxY);
		painter->drawPixmap(rect, backBuffer, src);
	}

	void QtDrawCanvas::drawGrid(QPainter *painter) {
		QRect rect(Margin, Margin,
		           width() - 2 * Margin, height() - 2 * Margin);
		QtDrawSettings settings = zoomStack[curZoom];
		QPen quiteDark(QPalette::Dark);
		QPen light(QPalette::Highlight);


		for (int i = 0; i <= settings.numXTicks; ++i) {
			int x = rect.left() + (i * (rect.width() - 1)
			                       / settings.numXTicks);
			double label = settings.minX + (i * settings.spanX()
			                                / settings.numXTicks);
			if (abs(label) < 0.00000005) label = 0.f;
			painter->setPen(quiteDark);
			painter->drawLine(x, rect.top(), x, rect.bottom());
			painter->setPen(light);
			painter->drawLine(x, rect.bottom(), x, rect.bottom() + 5);
			painter->drawText(x - 50, rect.bottom() + 5, 100, 15,
			                  Qt::AlignHCenter | Qt::AlignTop,
			                  QString::number(label));
		}
		for (int j = 0; j <= settings.numYTicks; ++j) {
			int y = rect.bottom() - (j * (rect.height() - 1)
			                         / settings.numYTicks);
			double label = settings.minY + (j * settings.spanY()
			                                / settings.numYTicks);

			if (abs(label) < 0.00000005) label = 0.f;
			painter->setPen(quiteDark);
			painter->drawLine(rect.left(), y, rect.right(), y);
			painter->setPen(light);
			painter->drawLine(rect.left() - 5, y, rect.left(), y);
			painter->drawText(rect.left() - Margin, y - 10,
			                  Margin - 5, 20,
			                  Qt::AlignRight | Qt::AlignVCenter,
			                  QString::number(label));
		}
		painter->drawRect(rect);
		//painter->setPen(Qt::red);
		//painter->drawRect(rubberBandRect);
	}

	void QtDrawCanvas::drawTicks(QPainter *painter) {
		QRect rect(Margin, Margin,
		           width() - 2 * Margin, height() - 2 * Margin);
		QtDrawSettings settings = zoomStack[curZoom];
		QPen quiteDark(QPalette::Dark);
		QPen light(QPalette::Highlight);

		for (int i = 0; i <= settings.numXTicks; ++i) {
			int x = rect.left() + (i * (rect.width() - 1)
			                       / settings.numXTicks);
			double label = settings.minX + (i * settings.spanX()
			                                / settings.numXTicks);
			if (abs(label) < 0.00000005) label = 0.f;
			painter->setPen(quiteDark);
			painter->drawLine(x, rect.top(), x, rect.top() + 5);
			painter->setPen(light);
			painter->drawLine(x, rect.bottom() - 5, x, rect.bottom());
			painter->drawText(x - 50, rect.bottom() + 5, 100, 15,
			                  Qt::AlignHCenter | Qt::AlignTop,
			                  QString::number(label));
		}
		for (int j = 0; j <= settings.numYTicks; ++j) {
			int y = rect.bottom() - (j * (rect.height() - 1)
			                         / settings.numYTicks);
			double label = settings.minY + (j * settings.spanY()
			                                / settings.numYTicks);
			if (abs(label) < 0.00000005) label = 0.f;
			painter->setPen(quiteDark);
			painter->drawLine(rect.right() - 5, y, rect.right(), y);
			painter->setPen(light);
			painter->drawLine(rect.left(), y, rect.left() + 5, y);
			painter->drawText(rect.left() - Margin / 2, y - 10,
			                  Margin - 5, 20,
			                  Qt::AlignRight | Qt::AlignVCenter,
			                  QString::number(label));
		}
		painter->drawRect(rect);
		//painter->setPen(Qt::red);
		//painter->drawRect(rubberBandRect);
	}

	void QtDrawCanvas::drawLabels(QPainter *painter) {
		QFont ft(painter->font());
		QPen pen(painter->pen());

		painter->setPen(title.color);
		painter->setFont(QFont(title.fontName, title.fontSize));
		painter->drawText(Margin, 8, width() - 2 * Margin, Margin / 2,
		                  Qt::AlignHCenter | Qt::AlignTop, title.text);

		painter->setPen(xLabel.color);
		painter->setFont(QFont(xLabel.fontName, xLabel.fontSize));
		painter->drawText(Margin, height() - Margin / 2,
		                  width() - 2 * Margin, Margin / 2,
		                  Qt::AlignHCenter | Qt::AlignTop, xLabel.text);

		QPainterPath text;
		QFont font(yLabel.fontName, yLabel.fontSize);
		QRect fontBoundingRect =
		    QFontMetrics(font).boundingRect(yLabel.text);
		text.addText(-QPointF(fontBoundingRect.center()),
		             font, yLabel.text);
		font.setPixelSize(50);
		painter->rotate(-90);
		painter->translate(- height() / 2, Margin / 4);
		painter->fillPath(text, yLabel.color);
		painter->translate(height() / 2, - Margin / 4);
		painter->rotate(90);

		painter->setPen(pen);
		painter->setFont(ft);
	}

	void QtDrawCanvas::drawWelcome(QPainter *painter) {
		QFont ft(painter->font());
		QPen pen(painter->pen());

		painter->setPen(welcome.color);
		painter->setFont(QFont(title.fontName, welcome.fontSize));
		painter->drawText(Margin, Margin,
		                  width() - 2 * Margin, height() - Margin * 2,
		                  Qt::AlignHCenter | Qt::AlignVCenter,
		                  welcome.text);
		painter->setPen(pen);
		painter->setFont(ft);


	}

	void QtDrawCanvas::drawRects(QPainter *painter) {
		QtDrawSettings settings = zoomStack[curZoom];
		QRect rect(Margin, Margin,
		           width() - 2 * Margin, height() - 2 * Margin);

		painter->setClipRect(rect.x() + 1, rect.y() + 1,
		                     rect.width() - 2, rect.height() - 2);

		std::map<int, CurveData>::iterator it = markerStack.begin();

		int h = (*pMask).ncolumn();
		int w = (*pMask).nrow();
		for (int i = 0; i < w; i++)
			for (int j = 0; j < h; j++) {
				(*pMask)(i, j) = 1;
			}
		while (it != markerStack.end()) {
			int id = (*it).first;
			const CurveData &data = (*it).second;

			QPainterPath points;
			//cout << " id=" << id << endl;
			//cout << " " << data[0] << " " << data[2]
			//        << " " << data[1] << " " << data[3] << endl;
			int h = (*pMask).ncolumn();
			for (int i = (int)data[0]; i < (int)data[2]; i++)
				for (int j = (int)data[3]; j < (int)data[1]; j++) {
					(*pMask)(i, h - j) = 0;
				}

			double dx1 = data[0] - settings.minX;
			double dy1 = data[1] - settings.minY;
			double dx2 = data[2] - settings.minX;
			double dy2 = data[3] - settings.minY;

			double x1 = rect.left() + (dx1 * (rect.width() - 1)
			                           / settings.spanX());
			double y1 = rect.bottom() - (dy1 * (rect.height() - 1)
			                             / settings.spanY());
			double x2 = rect.left() + (dx2 * (rect.width() - 1)
			                           / settings.spanX());
			double y2 = rect.bottom() - (dy2 * (rect.height() - 1)
			                             / settings.spanY());

			points.addRect((int)x1, (int)y1,
			               (int)(x2 - x1), (int)(y2 - y1));


			painter->setPen(Qt::green);
			//cout << "curMarker=" << curMarker
			//       << " id=" << id << endl;
			if (id == curMarker)
				painter->setPen(Qt::white);
			painter->drawPath(points);
			++it;
		}
	}

	QColor QtDrawCanvas::getLinearColor(double d) {
		double middle = 0.4;
		if (d > 1)
			return QColor(0, 0, 255);
		if (d < 0)
			return QColor(255, 0, 0);
		double r, g, b;
		if (d < 0.4) {
			r = 255 * (middle - d) / middle;
			g = 255 * d / middle;
			b = 0;
		} else {
			r = 0;
			g = 255 * ( 1 - d) / (1 - middle);
			b = (d - middle) * 255 / (1 - middle);
		}
		//std::cout << "r=" << r << " g=" << g
		//            << " b=" << b << std::endl;
		return QColor((int)r, (int)g, (int)b);
	}

	void QtDrawCanvas::drawCurves(QPainter *painter) {

		QtDrawSettings settings = zoomStack[curZoom];
		QRect rect(Margin, Margin,
		           width() - 2 * Margin, height() - 2 * Margin);

		painter->setClipRect(rect.x() + 1, rect.y() + 1,
		                     rect.width() - 2, rect.height() - 2);

		std::map<int, CurveData>::iterator it = curveMap.begin();

		int sz = curveMap.size();
		sz = sz > 1 ? sz - 1 : 1;

		while (it != curveMap.end()) {
			int id = (*it).first;
			const CurveData &data = (*it).second;
			int maxPoints = data.size() / 2;
			QPainterPath points;

			//std::cout << "id=" << id
			//          << " type=" << typeMap[id]
			//          << " color=" << colorMap[id] << std::endl;
			int tp = typeMap[id];
			if (typeMap[id] > FHIST)
				tp = (typeMap[id] - FHIST) % (HIST - 1);
			if (typeMap[id] < 0)
				tp = abs(typeMap[id]) % (HIST - 1);

			painter->setBrush(Qt::NoBrush);

			for (int i = 0; i < maxPoints; ++i) {
				double dx = data[2 * i] - settings.minX;
				double dy = data[2 * i + 1] - settings.minY;
				double x = rect.left() + (dx * (rect.width() - 1)
				                          / settings.spanX());
				double y = rect.bottom() - (dy * (rect.height() - 1)
				                            / settings.spanY());
				//std::cout << "i=" << i << " x=" << x
				//          << " y=" << y << std::endl;
				if (fabs(x) < 32768 && fabs(y) < 32768) {

					if (typeMap[id] < 1) {
						//draw continuous line
						//std::cout << "line" << std::endl;
						if (i == 0) {
							points.moveTo((int)x, (int)y);
						}
						points.lineTo((int)x, (int)y);
					}

					switch (tp) {
					case HIST:
					case FHIST: {
						//std::cout << "hist" << std::endl;
						double yz = rect.bottom() -
						            (- settings.minY * (rect.height() - 1)
						             / settings.spanY());
						if (i != 0) {
							QPointF prev = points.currentPosition();
							points.lineTo((int)x, (int)prev.y());
							points.lineTo((int)x, (int)yz);
							points.moveTo((int)prev.x(), (int)yz);
							points.lineTo((int)prev.x(), (int)prev.y());
							points.moveTo((int)x, (int)y);
						} else
							points.moveTo((int)x, (int)yz);
					}
					break;

					case CIRCLE:
					case FCIRCLE:
						//std::cout << "circle" << std::endl;
						//points.setFillRule(Qt::WindingFill);
						points.addEllipse(x - 2, y - 2, 4, 4);
						points.moveTo((int)x, (int)y);
						break;

					case RECT:
					case FRECT:
						//std::cout << "rect" << std::endl;
						points.addRect(x - 2, y - 2, 4, 4);
						points.moveTo((int)x, (int)y);
						break;

					case PLUS:
						//std::cout << "plus" << std::endl;
						points.moveTo(x - 2, y - 2);
						points.lineTo(x + 2, y - 2);
						points.moveTo(x - 2, y + 2);
						points.lineTo(x + 2, y + 2);
						points.moveTo(x, y);
					case FPLUS:
						//std::cout << "plusf" << std::endl;
						points.moveTo(x - 2, y);
						points.lineTo(x + 2, y);
						points.moveTo(x, y - 2);
						points.lineTo(x, y + 2);
						points.moveTo(x, y);
						break;

					case BEAM:
						//std::cout << "beam" << std::endl;
						points.moveTo(x - 2, y - 2);
						points.lineTo(x - 2, y + 2);
						points.moveTo(x + 2, y - 2);
						points.lineTo(x + 2, y + 2);
						points.moveTo(x, y);
					case FBEAM:
						//std::cout << "fbeam" << std::endl;
						points.moveTo(x - 2, y - 2);
						points.lineTo(x + 2, y - 2);
						points.moveTo(x - 2, y + 2);
						points.lineTo(x + 2, y + 2);
						points.moveTo(x, y - 2);
						points.lineTo(x, y + 2);
						points.moveTo(x, y);
						break;

					case BED:
						//std::cout << "bed" << std::endl;
						points.moveTo(x - 2, y - 2);
						points.lineTo(x + 2, y - 2);
						points.moveTo(x - 2, y + 2);
						points.lineTo(x + 2, y + 2);
						points.moveTo(x, y);
					case FBED:
						//std::cout << "fbed" << std::endl;
						points.moveTo(x - 2, y - 2);
						points.lineTo(x - 2, y + 2);
						points.moveTo(x + 2, y - 2);
						points.lineTo(x + 2, y + 2);
						points.moveTo(x - 2, y);
						points.lineTo(x + 2, y);
						points.moveTo(x, y);
						break;

					case CROSS:
						points.addEllipse(x - 2, y - 2, 4, 4);
					case FCROSS:
						//std::cout << "cross" << std::endl;
						points.moveTo((int)x - 2, (int)y - 2);
						points.lineTo((int)x + 2, (int)y + 2);
						points.moveTo((int)x + 2, (int)y - 2);
						points.lineTo((int)x - 2, (int)y + 2);
						points.moveTo((int)x, (int)y);
						break;

					case DIM:
					case FDIM:
						//std::cout << "dim" << std::endl;
						points.moveTo((int)x - 2, (int)y);
						points.lineTo((int)x, (int)y + 2);
						points.lineTo((int)x + 2, (int)y);
						points.lineTo((int)x, (int)y - 2);
						points.lineTo((int)x - 2, (int)y);
						points.moveTo((int)x, (int)y);
						break;

					case TRIGU:
					case FTRIGU:
						//std::cout << "trig u" << std::endl;
						points.moveTo((int)x - 2, (int)y - 1);
						points.lineTo((int)x, (int)y + 2);
						points.lineTo((int)x + 2, (int)y - 1);
						points.lineTo((int)x - 2, (int)y - 1);
						points.moveTo((int)x, (int)y);
						break;

					case TRIGD:
					case FTRIGD:
						//std::cout << "trig d" << std::endl;
						points.moveTo((int)x - 2, (int)y + 1);
						points.lineTo((int)x + 2, (int)y + 1);
						points.lineTo((int)x, (int)y - 2);
						points.lineTo((int)x - 2, (int)y + 1);
						points.moveTo((int)x, (int)y);
						break;

					case TRIGL:
					case FTRIGL:
						//std::cout << "trig l" << std::endl;
						points.moveTo((int)x - 2, (int)y);
						points.lineTo((int)x + 1, (int)y + 2);
						points.lineTo((int)x + 1, (int)y - 2);
						points.lineTo((int)x - 2, (int)y);
						points.moveTo((int)x, (int)y);
						break;

					case TRIGR:
					case FTRIGR:
						//std::cout << "trig r" << std::endl;
						points.moveTo((int)x + 2, (int)y);
						points.lineTo((int)x - 1, (int)y + 2);
						points.lineTo((int)x - 1, (int)y - 2);
						points.lineTo((int)x + 2, (int)y);
						points.moveTo((int)x, (int)y);
						break;

					case SAND:
					case FSAND:
						//std::cout << "sand" << std::endl;
						points.moveTo((int)x + 2, (int)y + 2);
						points.lineTo((int)x - 2, (int)y - 2);
						points.lineTo((int)x + 2, (int)y - 2);
						points.lineTo((int)x - 2, (int)y + 2);
						points.lineTo((int)x + 2, (int)y + 2);
						points.moveTo((int)x, (int)y);
						break;

					case  WING:
					case  FWING:
						//std::cout << "wing" << std::endl;
						points.moveTo((int)x + 2, (int)y + 2);
						points.lineTo((int)x + 2, (int)y - 2);
						points.lineTo((int)x - 2, (int)y + 2);
						points.lineTo((int)x - 2, (int)y - 2);
						points.lineTo((int)x + 2, (int)y + 2);
						points.moveTo((int)x, (int)y);
						break;

					case POINT:
						//std::cout << "points" << std::endl;
						points.addEllipse(x - 1, y - 1, 2, 2);
						points.moveTo((int)x, (int)y);
						break;

					case FPOINT:
						//std::cout << "points" << std::endl;
						points.addEllipse(x - 0.5, y - 0.5, 1, 1);
						points.moveTo((int)x, (int)y);
						break;

					default:
						break;
					}
				}
			}
			QColor clr = QtDrawColors::getDrawColor(colorMap[id]);
			painter->setPen(clr);
			if (tp % 2 == 1) {
				painter->setBrush(clr);
			}
			painter->drawPath(points);
			++it;
		}
	}


	/*void QtDrawCanvas::setMarkMode(bool b)
	{
	    markMode = b;
	}*/

	void QtDrawCanvas::plotPolyLines(QString path) {
		QFile file(path);

		if (file.open(QIODevice::ReadOnly)) {
			QTextStream in(&file);
			int lineCount = 0;
			int pointCount = 0;
			CurveData data;
			while (!in.atEnd()) {
				QString line = in.readLine();
				if (!line.isNull() && !line.isEmpty()) {
					if (line.startsWith('#')) {
						if (line.contains("title"))
							setTitle(line.mid(7));
						if (line.contains("xLabel"))
							setXLabel(line.mid(8));
						if (line.contains("yLabel"))
							setYLabel(line.mid(8));
					} else {
						QStringList coords =
						    line.trimmed().split(QRegExp("\\s+"));
						if (coords.size() >= 1) {
							double x = coords[0].toDouble();
							double y = coords[1].toDouble();
							data.push_back(x);
							data.push_back(y);
							pointCount++;
						}
						if (in.atEnd() && pointCount > 0) {
							setCurveData(lineCount, data);
						}
					}
				} else {
					if (pointCount > 0) {
						setCurveData(lineCount, data);
						pointCount = 0;
						lineCount++;
						data.clear();
					}
				}

			}
		}

		setDataRange();
		return;
	}
	void QtDrawCanvas::plotPolyLine(const Vector<Float> &x,
	                                const Vector<Float> &y, int ln, int tp, int cl) {

		//qDebug() << "plot poly line float";
		Int xl, yl;
		x.shape(xl);
		y.shape(yl);
		CurveData data;
		if (ln >= 0) {
			for (int i = 0; i < min(xl, yl); i++) {
				data.push_back(x[i]);
				data.push_back(y[i]);
			}
		}
		setCurveData(ln, data, tp, cl);

		setDataRange();
		return;
	}

	void QtDrawCanvas::plotPolyLine(const Vector<Int> &x,
	                                const Vector<Int> &y, int ln, int tp, int cl) {
		//qDebug() << "plot poly line int";
		Int xl, yl;
		x.shape(xl);
		y.shape(yl);
		CurveData data;
		if (ln >= 0) {
			for (int i = 0; i < min(xl, yl); i++) {
				data.push_back(x[i]);
				data.push_back(y[i]);
			}
		}
		setCurveData(ln, data, tp, cl);

		setDataRange();
		return;
	}

	void QtDrawCanvas::plotPolyLine(const Vector<Double> &x,
	                                const Vector<Double>&y, int ln, int tp, int cl) {
		//qDebug() << "plot poly line double";
		Int xl, yl;
		x.shape(xl);
		y.shape(yl);
		CurveData data;
		if (ln >= 0) {
			for (int i = 0; i < min(xl, yl); i++) {
				data.push_back(x[i]);
				data.push_back(y[i]);
			}
		}
		setCurveData(ln, data, tp, cl);

		setDataRange();
		return;
	}

	void QtDrawCanvas::plotPolyLine(const Matrix<Double> &x,
	                                int ln, int tp, int cl) {
		Int nr, nc;
		x.shape(nr, nc);

		//every row is a trace
		int n = nc / 2;
		if (n < 1 || nr < 1)
			return;
		CurveData *data = new CurveData[nr];
		for (int i = 0; i < nr; i++) {
			if (ln < 0)
				setCurveData(ln - i, data[i]);
			else {
				for (int j = 0; j < n; j++) {
					data[i].push_back(x(i, 2 * j));
					data[i].push_back(x(i, 2 * j + 1));
				}
				int clr = cl + i;
				if (cl < 0)
					clr = abs(cl);

				int typ = abs(tp);
				if (tp > 0)
					typ = tp + i;

				int lin = ln + i;
				if (ln == 0 && lin > 1)
					lin++;
				setCurveData(lin, data[i], typ, clr);
			}
		}
		delete [] data;

		setDataRange();
		return;
	}

	void QtDrawCanvas::plotPolyLine(const Matrix<Int> &x,
	                                int ln, int tp, int cl) {

		Int nr, nc;
		x.shape(nr, nc);

		//every row is a trace
		int n = nc / 2;
		if (n < 1 || nr < 1)
			return;
		CurveData *data = new CurveData[nr];
		for (int i = 0; i < nr; i++) {
			if (ln < 0)
				setCurveData(ln - i, data[i]);
			else {
				for (int j = 0; j < n; j++) {
					data[i].push_back(x(i, 2 * j));
					data[i].push_back(x(i, 2 * j + 1));
				}
				int clr = cl + i;
				if (cl < 0)
					clr = abs(cl);

				int typ = abs(tp);
				if (tp > 0)
					typ = tp + i;

				int lin = ln + i;
				if (ln == 0 && lin > 0)
					lin++;
				setCurveData(lin, data[i], typ, clr);
			}
		}

		setDataRange();
		delete [] data;
		return;
	}

	void QtDrawCanvas::plotPolyLine(const Matrix<Float> &x,
	                                int ln, int tp, int cl) {

		Int nr, nc;
		x.shape(nr, nc);

		//every row is a trace
		int n = nc / 2;
		if (n < 1 || nr < 1)
			return;
		CurveData *data = new CurveData[nr];
		for (int i = 0; i < nr; i++) {
			if (ln < 0)
				setCurveData(ln - i, data[i]);
			else {
				for (int j = 0; j < n; j++) {
					data[i].push_back(x(i, 2 * j));
					data[i].push_back(x(i, 2 * j + 1));
				}
				int clr = cl + i;
				if (cl < 0)
					clr = abs(cl);

				int typ = abs(tp);
				if (tp > 0)
					typ = tp + i;

				int lin = ln + i;
				if (ln == 0 && lin > 0)
					lin++;
				setCurveData(lin, data[i], typ, clr);
			}
		}

		//assume every row is a line
		//int m = nr / 2;
		//if (m < 1 || nc < 1)
		//    return;
		//CurveData data[n];
		//for (int i = 0; i < nc; i++)
		//{
		//     for (int j = 0; j < m; j++)
		//     {
		//         data[i].push_back(x(2 * j, i));
		//         data[i].push_back(x(2 * j + 1, i));
		//     }
		//     setCurveData(i, data[i], tp, cl);
		// }

		setDataRange();
		delete [] data;
		return;
	}

//template<class T>
	void QtDrawCanvas::drawImage(const Matrix<uInt> &data,
	                             Matrix<uInt> *mask) {
		pMask = mask;
		Int w = data.nrow();
		Int h = data.ncolumn();

		QImage im = QImage(w, h, QImage::Format_RGB32);
		uInt max = 0;
		uInt min = 100000000;
		for (int y=0; y<h; y++) {
			for (int x=0; x<w; x++) {
				if (min > data(x, y))
					min = data(x, y);
				if (max < data(x, y))
					max = data(x, y);
			}
		}
		uInt swap = 0;
		if (min > max) {
			swap = max;
			max = min;
			min = swap;
		} else if (min == max) {
			if (min > 0) {
				min--;
			} else {
				max++;
			}
		}
		swap = max - min;

		for (int y=0; y<h; y++) {
			for (int x=0; x<w; x++) {
				QRgb qrgb = getLinearColor((double)data(x, y)
				                           / swap).rgb();
				im.setPixel(x, h - y - 1, qrgb);
				//std::cout << "r=" << qRed(qrgb)
				//          << " g=" << qGreen(qrgb) << " b="
				//          << qBlue(qrgb) << std::endl;
			}
		}

		setPixmap(im);
		//setDataRange();

		setImageMode(true);

		return;
	}

	void QtDrawCanvas::drawImage(const Matrix<uInt> &data) {
		pMask = 0;

		Int w = data.nrow();
		Int h = data.ncolumn();

		QImage im = QImage(w, h, QImage::Format_RGB32);
		uInt max = 0;
		uInt min = 100000000;
		for (int y=0; y<h; y++) {
			for (int x=0; x<w; x++) {
				if (min > data(x, y))
					min = data(x, y);
				if (max < data(x, y))
					max = data(x, y);
			}
		}

		uInt swap = 0;
		if (min > max) {
			swap = max;
			max = min;
			min = swap;
		} else if (min == max) {
			if (min > 0) {
				min--;
			} else {
				max++;
			}
		}
		swap = max - min;

		for (int y=0; y<h; y++) {
			for (int x=0; x<w; x++) {
				QRgb qrgb = getLinearColor((double)data(x, y) /
				                           swap).rgb();
				im.setPixel(x,  h - y - 1, qrgb);
				//std::cout << "r=" << qRed(qrgb)
				//          << " g=" << qGreen(qrgb)
				//          << " b=" << qBlue(qrgb) << std::endl;
			}
		}

		setPixmap(im);
		//setDataRange();

		setImageMode(true);
		return;
	}
	void QtDrawCanvas::setPixmap(const QImage &data) {
		//std::cout << "pixmap w=" << pixmap.width()
		//            << " h=" << pixmap.height()
		//            << " pixmap isNull()=" << pixmap.isNull()
		//            << std::endl;
		backBuffer = QPixmap::fromImage(data);
		QtDrawSettings settings;
		settings.minX = 0;
		settings.maxX = backBuffer.width();
		settings.minY = 0;
		settings.maxY = backBuffer.height();
		settings.adjust();
		setPlotSettings(settings);
		//QImage img = backBuffer.toImage();
		//for (int i = 290; i < 300; i++)
		//    for (int j = 290; j < 300; j++)
		//std::cout << "x=" << i << " y=" << j
		//            << " pixmap data=" << hex <<
		//img.pixel(i, j) << dec << std::endl;
		refreshPixmap();
	}

	void QtDrawCanvas::setImageMode(bool b) {
		imageMode = b;
	}

	void QtDrawCanvas::increaseCurZoom() {
		curZoom++;

	}

	int QtDrawCanvas::getCurZoom() {
		return curZoom;
	}

	int QtDrawCanvas::getZoomStackSize() {
		return (int)zoomStack.size();
	}

	void QtDrawCanvas::setTitle(const QString &text,
	                            int fontSize, int clr, const QString &font) {
		title.text = text;
		title.fontName = font;
		title.fontSize = fontSize;
		title.color = QtDrawColors::getDrawColor(clr);
	}

	void QtDrawCanvas::setXLabel(const QString &text,
	                             int fontSize, int clr, const QString &font) {
		xLabel.text = text;
		xLabel.fontName = font;
		xLabel.fontSize = fontSize;
		xLabel.color = QtDrawColors::getDrawColor(clr);
	}

	void QtDrawCanvas::setYLabel(const QString &text,
	                             int fontSize, int clr, const QString &font) {
		yLabel.text = text;
		yLabel.fontName = font;
		yLabel.fontSize = fontSize;
		yLabel.color = QtDrawColors::getDrawColor(clr);
	}

	void QtDrawCanvas::setWelcome(const QString &text,
	                              int fontSize, int clr, const QString &font) {
		welcome.text = text;
		welcome.fontName = font;
		welcome.fontSize = fontSize;
		welcome.color = QtDrawColors::getDrawColor(clr);
	}

	QPixmap* QtDrawCanvas::graph() {
		return &pixmap;
	}

}
