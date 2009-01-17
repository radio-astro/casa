//# DS9FileWriter.qo.h: DS9 implementation of RSFileWriter classes.
//# Copyright (C) 2008
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
#ifndef DS9FILEWRITER_QO_H_
#define DS9FILEWRITER_QO_H_

#include <QTextStream>

#include <display/RegionShapes/QtDS9WriteOptions.ui.h>
#include <display/RegionShapes/RSFileReaderWriter.h>
#include <display/RegionShapes/DS9FileReader.h>

#include <casa/namespace.h>

namespace casa {

class RSMarker;

// Implementation of RegionFileWriter for writing to DS9 files.
class DS9FileWriter : public RSFileWriter {
public:
    // Options constants.
    // </group>
    static const String CSYS_INDIVIDUAL;
    static const String CSYS_IMAGE;
    static const String CSYS_B1950;
    static const String CSYS_J2000;
    static const String CSYS_GALACTIC;
    static const String CSYS_ECLIPTIC;
    static const String UNIT_IPIXELS;
    static const String UNIT_DEGREES;
    static const String UNIT_RADIANS;
    static const String PUNIT_SEXAGESIMAL;
    static const String PUNIT_HMS;
    static const String PUNIT_DMS;
    static const String SUNIT_ARCSEC;
    static const String SUNIT_ARCMIN;
    static const String DEFAULT_COMMENTS;
    // </group>
    
    // Converts a coordinate system constant (as used in DS9RFWOptions) into
    // its DS9::CoordinateSystem equivalent.
    static DS9::CoordinateSystem coordinateSystem(const String& system) {
        if(system == CSYS_IMAGE) return DS9::Image;
        else if(system == CSYS_B1950) return DS9::FK4;
        else if(system == CSYS_J2000) return DS9::FK5;
        else if(system == CSYS_GALACTIC) return DS9::Galactic;
        else if(system == CSYS_ECLIPTIC) return DS9::Ecliptic;
        
        else return DS9::defaultCoordinateSystem();
    }
    
    // Returns true if the given CASA world system is supported by DS9.
    static bool isValidDS9System(MDirection::Types worldSys) {
        return worldSys == MDirection::B1950 ||
               worldSys == MDirection::J2000 ||
               worldSys == MDirection::GALACTIC ||
               worldSys == MDirection::ECLIPTIC;
    }
    
    
    // Constructor.
    DS9FileWriter();
    
    // Destructor.
    ~DS9FileWriter();
    
    
    // Implements RegionFileWriter::optionsWidget.
    QWidget* optionsWidget() const;
    
    // Implements RegionFileWriter::setOptions.
    void setOptions(const QWidget* widget);

    // Implements RegionFileWriter::write.
    bool write(const vector<RegionShape*>& shapes) const;
    
private:
    // Custom options.
    // <group>
    bool m_custCoordSys;
    bool m_pixelCoordSys;
    String m_coordSys;
    String m_posUnits, m_sizeUnits;
    int m_precision;
    String m_comments;
    // </group>
    
    
    // Rests custom options to defaults.
    void resetOptions() const;
    
    // Writes header comments to the file.
    bool writeHeader(QTextStream& file, stringstream& errors) const;
    
    // Writes globals (default DS9Region properties) and, if applicable, the
    // overriding coordinate system.
    bool writeGlobals(QTextStream& file, stringstream& errors) const;
    
    // Writes a single shape to the file.
    bool writeShape(QTextStream& file, stringstream& errors,
                    const RegionShape* shape, bool isComposite = false) const;

    
    // Returns list of all properties to be used in the globals.
    static vector<String> globalProperties() {
        static vector<String> v(14);
        
        v[0]  = DS9Region::PROP_COLOR;    v[1]  = DS9Region::PROP_DASH;
        v[2]  = DS9Region::PROP_DASHLIST; v[3]  = DS9Region::PROP_DELETE;
        v[4]  = DS9Region::PROP_EDIT;     v[5]  = DS9Region::PROP_FIXED;
        v[6]  = DS9Region::PROP_FONT;     v[7]  = DS9Region::PROP_HIGHLITE;
        v[8]  = DS9Region::PROP_INCLUDE;  v[9]  = DS9Region::PROP_MOVE;
        v[10] = DS9Region::PROP_ROTATE;   v[11] = DS9Region::PROP_SELECT;
        v[12] = DS9Region::PROP_SOURCE;   v[13] = DS9Region::PROP_WIDTH;
        
        return v;
    }
    
    // Finds the DS9 region type for the given shape.  Returns true if no
    // errors were reported, false otherwise.  If errors occurred they are
    // logged on the given stringstream.  Note that RSMarkers are expected
    // to already be converted to a DS9-friendly format.  If this method
    // returns false, the given shape should NOT be written.
    static bool regionType(const RegionShape* shape, DS9::RegionType& type,
                           stringstream& errors);
    
    // Converts the given RSMarker into one or more DS9-friendly RSMarkers.
    // If marker is NULL or the markers is empty then an empty vector is
    // returned.  Note that the caller is responsible for deleting the new
    // markers.
    static vector<RSMarker*> convertMarker(const RSMarker* marker,
                                           bool& conversionWasNeeded);
    
    // Converts the given value to a QString version using the given units.
    // The value is assumed to be in units RegionShape::UNIT.  toUnits should
    // be one of: UNIT_IPIXELS, UNIT_DEGREES, UNIT_RADIANS, PUNIT_HMS,
    // PUNIT_DMS, SUNIT_ARCSEC, SUNIT_ARCMIN, or "".  If toUnits is blank, no
    // converting or adding a unit is done.
    static QString convertValue(double value, const String& toUnits,
                                int precision);
    
    // Converts the given position unit, if applicable, from sexagesimal to
    // either HMS or DMS.  Does nothing if the position unit is not
    // sexagesimal.
    static String sgUnit(const String& posUnit, int index,
                         const String& coordSys) {
        if(posUnit == PUNIT_SEXAGESIMAL) {
            if(index % 2 == 0 && coordSys != CSYS_GALACTIC) return PUNIT_HMS;
            else return PUNIT_DMS;
        } else return posUnit;
    }
};


// Widget used for entering options specific to the DS9 format.
class QtDS9RFWOptions : public QWidget, Ui::DS9WriteOptions {
    Q_OBJECT
    
public:
    // Constructor.
    QtDS9RFWOptions();
    
    // Destructor.
    ~QtDS9RFWOptions();
    
    
    // Returns true if the user picked a custom coordinate system, false if
    // DS9RegionFileWriter::CSYS_INDIVIDUAL was chosen.
    bool isCustomCoordinateSystem() const;
    
    // Returns true if the user picked a custom coordinate system and that
    // system was DS9RegionFileWriter::CSYS_IMAGE, false otherwise.
    bool isPixelCoordinateSystem() const;
    
    // Returns the selected coordinate system.  This should only be used if
    // customCoordinateSystem() and pixelCoordinateSystem() were both false.
    // One of: DS9RegionFileWriter::CSYS_*
    String getCoordinateSystem() const;
    
    // Returns the selected position units.
    // One of: DS9RegionFileWriter::UNIT_* or DS9RegionFileWriter::PUNIT_*
    String getPositionUnits() const;
    
    // Returns the selected size units.
    // One of: DS9RegionFileWriter::UNIT_* or DS9RegionFileWriter::SUNIT_*
    String getSizeUnits() const;
    
    // Returns the entered decimal precision.
    int getPrecision() const;
    
    // Returned the entered comments.
    String getComments() const;
    
private slots:
    // Enable/disable position and size unit chooser accordingly.
    void coordinateSystemChanged(int index);
};

}

#endif /* DS9FILEWRITER_H_ */
