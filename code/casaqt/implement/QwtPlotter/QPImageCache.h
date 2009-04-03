//# QPImageCache.h: Classes for caching axes images.
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
#ifndef QPIMAGECACHE_H_
#define QPIMAGECACHE_H_

#ifdef AIPS_HAS_QWT

#include <graphics/GenericPlotter/PlotOptions.h>

#include <QCache>
#include <QImage>
#include <QMap>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class QPCanvas;


// Abstraction of whatever Qt class the plotter uses to store images.
// Currently is a QImage to avoid threading issues associated with QPixmap.
// This class is assumed to use memory intelligently, so that only one
// underlying image is used, with copy-on-write semantics.  This is done
// automatically with most Qt classes (such as QImage or QPixmap).
class QPImageCache {
public:
    // Creates a null, blank image.
    QPImageCache();
    
    // Creates a new image with the given size.
    // <group>
    QPImageCache(const QSize& size);
    QPImageCache(int width, int height);
    // </group>
    
    // Copies the given image, with copy-on-write semantics. 
    QPImageCache(const QPImageCache& copy);
    
    // Destructor.
    ~QPImageCache();
    
    
    // Returns true if this image is null/invalid, false otherwise.
    bool isNull() const;
    
    // Returns the size of this image.
    QSize size() const;
    
    // Fills the image with the given value.
    void fill(unsigned int fillValue);
    
    // Returns the depth of the image.
    int depth() const;
    
    // Returns a painter that can draw into this image.  It is the caller's
    // responsibility to delete the painter upon completion.
    QPainter* painter();
    
    // Draws the image using the given painter, into the given draw rect.  If
    // the image is not the same size as the draw rect, it will be stretched
    // appropriately.
    // <group>
    void paint(QPainter& painter, const QRect& drawRect) const;
    void paint(QPainter* painter, const QRect& drawRect) const {
        if(painter != NULL) paint(*painter, drawRect); }
    // </group>
    
    
    // Used to access the underlying data structure.  Direct access should be
    // avoided, in case it changes in the future.
    // <group>
    QImage& asQImage();
    const QImage& asQImage() const;
    // </group>
    
    
    // Copy operator, with copy-on-write semantics.
    QPImageCache& operator=(const QPImageCache& copy);
    
    // Equality operators.
    // <group>
    bool operator==(const QPImageCache& other);
    bool operator!=(const QPImageCache& other) { return !(operator==(other)); }
    // </group>
    
private:
    // Underlying image.
    QImage m_image;
};


// Class to managed cached images associated with a canvas axes stack.
class QPAxesCache {    
public:
    // Static //
    
    // Default cache size limit, in kilobytes.
    static const int DEFAULT_MEMORY_LIMIT_KB;
    
    // Convenient access to class name for logging.
    static const String CLASS_NAME;
    
    // Convenience class to use as a key.  Every axis that has at least one
    // item attached to it will get an entry in the map, along with its range.
    class Key : public QMap<PlotAxis, QPair<double, double> > {
    public:
        // Default constructor.  Empty key.
        Key();
        
        // Constructor which uses the current axes state of the given canvas.
        Key(const QPCanvas& canvas);
        
        // Destructor.
        ~Key();
        
        
        // Sets the key value using the current axes state of the given canvas.
        void setValue(const QPCanvas& canvas);
        
        // Returns a hash for this key.
        uint hash() const;
    };
    
    
    // Non-Static //
    
    // Constructor which takes a size limit for the cache.
    QPAxesCache(QPCanvas& canvas, int sizeLimitKb = DEFAULT_MEMORY_LIMIT_KB);
    
    // Destructor.
    ~QPAxesCache();
    
    
    // Returns the current size of the cache.
    unsigned int size() const;
    
    // Returns the (approximate) current memory size of the cache, in
    // kilobytes.
    int memorySize() const;
    
    // Gets/Sets the size limit for the cache, in kilobytes.
    // <group>
    int memoryLimit() const;
    void setMemoryLimit(int memoryLimitKb);
    // </group>
    
    // Returns the current size of the images that are cached.  This can be
    // used to check for resizing, in which case the cache should probably be
    // cleared.
    QSize currImageSize() const;
    
    // Gets/Sets the fixed cache image size.  See
    // PlotCanvas::cachedAxesStackImageSize().
    // <group>
    QSize fixedImageSize() const;
    void setFixedSize(QSize size);
    // </group>
    
    // Clears all cached images.
    void clear();
    
    // Clears all cached images for the given layer(s).
    // <group>
    void clearLayer(PlotCanvasLayer layer);
    void clearLayers(int layersFlag);
    // </group>
    
    // Returns true if the cache has an entry for the current axes state of its
    // canvas, false otherwise.
    bool currHasImage() const;
    
    // Returns true if the cache has an image for the current axes state of its
    // parent for the given layer, false otherwise.
    bool currHasImage(PlotCanvasLayer layer) const;
    
    // Returns the image for the current axes state of its canvas for the given
    // layer, or a null image if there is none.
    QPImageCache currImage(PlotCanvasLayer layer);
    
    // Adds the given image for the current axes state of its canvas for the
    // given layer.  The current image size is set to the size of the given
    // image (unless it is a null image).
    void addCurrImage(PlotCanvasLayer layer, const QPImageCache& image);
    
private:
    // Convenience class to use as a value.
    class Value : public QMap<PlotCanvasLayer, QPImageCache> {
    public:
        // Default constructor.
        Value();
        
        // Copy constructor.
        Value(const QMap<PlotCanvasLayer, QPImageCache>& copy);
        
        // Destructor.
        ~Value();
        
        // Returns the approximate memory size, in kilobytes.
        int memorySize() const;
    };
    
    
    // Parent canvas.
    QPCanvas& m_canvas;
    
    // Image cache.
    QCache<Key, Value> m_cache;
    
    // Fixed image size.
    QSize m_fixedSize;
};

}

// Provides a hashing function for QPAxesCache::Key, for use with Qt.
uint qHash(const QPAxesCache::Key& key);

#endif
#endif /* QPIMAGECACHE_H_ */
