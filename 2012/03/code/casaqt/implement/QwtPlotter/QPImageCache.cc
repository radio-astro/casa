//# QPImageCache.cc: Classes for caching axes images.
//# Copyright (C) 2009
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
#ifdef AIPS_HAS_QWT
#include <casaqt/QwtPlotter/QPImageCache.h>

#include <casaqt/QwtPlotter/QPCanvas.qo.h>

#include <QPainter>

uint qHash(const QPAxesCache::Key& key) { return key.hash(); }

namespace casa {

//////////////////////////////
// QPIMAGECACHE DEFINITIONS //
//////////////////////////////

QPImageCache::QPImageCache() { }
QPImageCache::QPImageCache(const QSize& size) :
    m_image(size, QImage::Format_ARGB32) { }
QPImageCache::QPImageCache(int width, int height) :
    m_image(width, height, QImage::Format_ARGB32) { }
QPImageCache::QPImageCache(const QPImageCache& copy): m_image(copy.m_image) { }
QPImageCache::~QPImageCache() { }

bool QPImageCache::isNull() const { return m_image.isNull(); }
QSize QPImageCache::size() const { return m_image.size(); }
void QPImageCache::fill(unsigned int fillValue) { m_image.fill(fillValue); }
int QPImageCache::depth() const { return m_image.depth(); }
QPainter* QPImageCache::painter() { return new QPainter(&m_image); }
void QPImageCache::paint(QPainter& painter, const QRect& drawRect) const {
    if(!m_image.isNull()) {
        QImage scaled = m_image;
        if(drawRect.size() != m_image.size())
            scaled = m_image.scaled(drawRect.size(), Qt::IgnoreAspectRatio,
                Qt::SmoothTransformation);
        painter.drawImage(drawRect, scaled);
    }
}

QImage& QPImageCache::asQImage() { return m_image; }
const QImage& QPImageCache::asQImage() const { return m_image; }

QPImageCache& QPImageCache::operator=(const QPImageCache& copy) {
    m_image = copy.m_image;
    return *this; }
bool QPImageCache::operator==(const QPImageCache& other) {
    return m_image == other.m_image; }


/////////////////////////////
// QPAXESCACHE DEFINITIONS //
/////////////////////////////

// 500 MB limit, where an approx. 500 x 400 image is ~1MB
const int QPAxesCache::DEFAULT_MEMORY_LIMIT_KB = 500000;

const String QPAxesCache::CLASS_NAME = "QPAxesCache";


QPAxesCache::QPAxesCache(QPCanvas& canvas, int memLimitKb) : m_canvas(canvas),
        m_cache(memLimitKb > 0 ? memLimitKb : DEFAULT_MEMORY_LIMIT_KB) { }

QPAxesCache::~QPAxesCache() { }


unsigned int QPAxesCache::size() const { return (unsigned int)m_cache.size(); }

int QPAxesCache::memorySize() const { return m_cache.totalCost(); }

int QPAxesCache::memoryLimit() const { return m_cache.maxCost(); }

void QPAxesCache::setMemoryLimit(int memoryLimitKb) {
    if(memoryLimitKb <= 0) memoryLimitKb = DEFAULT_MEMORY_LIMIT_KB;
    if(memoryLimitKb != m_cache.maxCost()) m_cache.setMaxCost(memoryLimitKb);
}

QSize QPAxesCache::currImageSize() const {
    Value* v = m_cache.object(Key(m_canvas));
    if(v == NULL || v->size() == 0) return QSize();
    else return v->constBegin()->size();
}

QSize QPAxesCache::fixedImageSize() const { return m_fixedSize; }
void QPAxesCache::setFixedSize(QSize size) {
    if(size != m_fixedSize) {
        m_fixedSize = size;
        clear();
    }
}

void QPAxesCache::clear() { m_cache.clear(); }

void QPAxesCache::clearLayer(PlotCanvasLayer layer) {
    Value* v;
    foreach(Key key, m_cache.keys()) {
        v = m_cache.object(key);
        if(v != NULL) {
            v->remove(layer);
            if(v->size() == 0) m_cache.remove(key);
        }
    }
}

void QPAxesCache::clearLayers(int layersFlag) {
    Value* v;
    foreach(Key key, m_cache.keys()) {
        v = m_cache.object(key);
        if(v != NULL) {
            foreach(PlotCanvasLayer layer, v->keys())
                if(layersFlag & layer) v->remove(layer);
            if(v->size() == 0) m_cache.remove(key);
        }
    }
}

bool QPAxesCache::currHasImage() const {
    Value* v = m_cache.object(Key(m_canvas));
    return v != NULL && v->size() > 0;
}

bool QPAxesCache::currHasImage(PlotCanvasLayer layer) const {
    Value* v = m_cache.object(Key(m_canvas));
    return v != NULL && v->contains(layer);
}

QPImageCache QPAxesCache::currImage(PlotCanvasLayer layer) {
    Value* v = m_cache.object(Key(m_canvas));
    if(v == NULL) return QPImageCache();
    else          return v->value(layer);
}

void QPAxesCache::addCurrImage(PlotCanvasLayer l, const QPImageCache& image) {
    Key key(m_canvas);
    Value* value = m_cache.take(key);
    if(value == NULL) value = new Value();
    value->insert(l, QPImageCache(image));
    m_cache.insert(key, value, value->memorySize());
}


//////////////////////////////////
// QPAXESCACHE::KEY DEFINITIONS //
//////////////////////////////////

QPAxesCache::Key::Key() { }

QPAxesCache::Key::Key(const QPCanvas& canvas) {
    setValue(canvas); }

QPAxesCache::Key::~Key() { }


void QPAxesCache::Key::setValue(const QPCanvas& canvas) {
    clear();
    
    vector<PlotItemPtr> items = canvas.allPlotItems();
    QSet<PlotAxis> axes;
    for(unsigned int i = 0; i < items.size(); i++) {
        if(items[i].null()) continue;
        axes.insert(items[i]->xAxis());
        axes.insert(items[i]->yAxis());
    }
    
    QSetIterator<PlotAxis> iter(axes);
    PlotAxis axis;
    prange_t range;
    while(iter.hasNext()) {
        axis = iter.next();
        range = canvas.axisRange(axis);
        insert(axis, QPair<double, double>(range.first, range.second));
    }
}

uint QPAxesCache::Key::hash() const {
    // Not terribly efficient/unique, but it'll do..
    
    vector<PlotAxis> axes = PlotCanvas::allAxes();
    QString axesStr;
    double from, to;
    QStringList values;
    for(unsigned int i = 0; i < axes.size(); i++) {
        if(contains(axes[i])) {
            axesStr += "1";
            from = value(axes[i]).first;
            to = value(axes[i]).second;
        } else {
            axesStr += "0";
            from = 0;
            to = 0;
        }
        values << QString::number(from, 'e') << QString::number(to, 'e');
    }
    values.prepend(axesStr);
    return qHash(values.join(""));
}


////////////////////////////////////
// QPAXESCACHE::VALUE DEFINITIONS //
////////////////////////////////////

QPAxesCache::Value::Value() { }
QPAxesCache::Value::Value(const QMap<PlotCanvasLayer, QPImageCache>& copy) :
        QMap<PlotCanvasLayer, QPImageCache>(copy) { }
QPAxesCache::Value::~Value() { }

int QPAxesCache::Value::memorySize() const {
    // From QPixmapCache documentation:
    // A pixmap takes roughly (width * height * depth)/8 bytes of memory.
    int size = 0;
    foreach(QPImageCache image, *this)
        size += (image.size().width() * image.size().height() * image.depth())/
                (8 * 1000);
    return size;
}

}

#endif
