//# PlotMSRegions.h: Properties of selected regions for a PlotMSPlot.
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
#ifndef PLOTMSREGIONS_H_
#define PLOTMSREGIONS_H_

#include <plotms/PlotMS/PlotMSConstants.h>

#include <map>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// A single region; basically just a container class for four doubles.
class PlotMSRegion {
public:
    // Default constructor, which creates an invalid region in which all four
    // values are zero.
    PlotMSRegion();
    
    // Constructor that takes the four values of the region.
    PlotMSRegion(double xMin, double xMax, double yMin, double yMax);
    
    // Copy constructor (see operator=()).
    PlotMSRegion(const PlotMSRegion& copy);
    
    // Destructor.
    ~PlotMSRegion();
    
    
    // Returns whether or not this region is valid.  A region is invalid if
    // either of its maxes are <= its mins.
    bool isValid() const;
    
    // Returns whether or not this region contains the given (x, y) pair.  If
    // exclusive is true, then the borders are not included.
    bool contains(double x, double y, bool exclusive = true) const;
    
    // Returns the region values.
    // <group>
    double left() const { return xMin(); }
    double right() const { return xMax(); }
    double bottom() const { return yMin(); }
    double top() const { return yMax(); }
    
    double xMin() const;
    double xMax() const;
    double yMin() const;
    double yMax() const;
    // </group>
    
    // Sets the region values.
    // <group>
    void setValues(double xMin, double xMax, double yMin, double yMax);
    void setX(double min, double max);
    void setY(double min, double max);
    void setXMin(double val);
    void setXMax(double val);
    void setYMin(double val);
    void setYMax(double val);
    void setLeft(double val) { setXMin(val); }
    void setRight(double val) { setXMax(val); }
    void setBottom(double val) { setYMin(val); }
    void setTop(double val) { setYMax(val); }
    // </group>
    
    
    // Equality operators.
    // <group>
    bool operator==(const PlotMSRegion& region) const;
    bool operator!=(const PlotMSRegion& region) const {
        return !(operator==(region)); }
    // </group>
    
    // Copy operator.
    PlotMSRegion& operator=(const PlotMSRegion& copy);
    
private:
    // Values.
    double itsXMin_, itsXMax_, itsYMin_, itsYMax_;
};

// PlotMSRegions is a class that holds information about selected regions for a
// single PlotMSPlot.  Because PlotMSPlot can have potentially many different
// plots across potentially many different canvases, it is important that
// PlotMSRegions be general enough to handle many different cases.
class PlotMSRegions {
public:
    // Constructor, which creates an empty selection.
    PlotMSRegions();
    
    // Destructor.
    ~PlotMSRegions();
    
    
    // Returns all axis pairs that have regions.
    Vector<pair<PMS::Axis, PMS::Axis> > allAxisPairs() const;
    
    // Returns whether or not there are regions for the given (x, y) axis pair.
    bool hasRegionsFor(PMS::Axis x, PMS::Axis y) const;
    
    // Returns a copy of the regions for the given (x, y) axis pair.
    Vector<PlotMSRegion> regionsFor(PMS::Axis x, PMS::Axis y) const;
    
    // Adds the given regions for the given (x, y) axis pair.  Will not remove
    // any existing regions for that pair.  Only adds unique regions.
    // <group>
    void addRegions(PMS::Axis x, PMS::Axis y, const vector<PlotMSRegion>& r) {
        addRegions(x, y, Vector<PlotMSRegion>(r)); }
    void addRegions(PMS::Axis x, PMS::Axis y, const Vector<PlotMSRegion>& r);
    // </group>
    
    // Convenience method for adding all selected regions using the standard
    // select tool on the given canvas for the given (x, y) axis pair.
    void addRegions(PMS::Axis x, PMS::Axis y, PlotCanvasPtr canvas);
    
    // Sets the regions for the given (x, y) axis pair to the given.  Removes
    // any existing regions for that pair.  Only adds unique regions.
    // <group>
    void setRegions(PMS::Axis x, PMS::Axis y, const vector<PlotMSRegion>& r) {
        setRegions(x, y, Vector<PlotMSRegion>(r)); }
    void setRegions(PMS::Axis x, PMS::Axis y, const Vector<PlotMSRegion>& r);
    // </group>
    
    // Convenience method for setting the regions for the given (x, y) axis
    // pair using the standard select tool on the given canvas.
    void setRegions(PMS::Axis x, PMS::Axis y, PlotCanvasPtr canvas);
    
    // Clears the regions for the given (x, y) axis pair.
    void clearRegions(PMS::Axis x, PMS::Axis y);
    
private:
    // Convenience macro for map type.
    typedef map<pair<PMS::Axis, PMS::Axis>, Vector<PlotMSRegion> > PMSRMap;
    
    // Map from (x, y) to selected regions.
    PMSRMap itsRegions_;
};

}

#endif /* PLOTMSREGIONS_H_ */
