//# RSFileReaderWriter.h: Interfaces for classes that read/write shape files.
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
#ifndef RSFILEREADERWRITER_H_
#define RSFILEREADERWRITER_H_

#include <display/RegionShapes/RegionShape.h>
#include <casa/Containers/Record.h>
#include <display/Display/PanelDisplay.h>
#include <coordinates/Coordinates/CoordinateSystem.h>

#include <utility>

#include <casa/namespace.h>
using namespace std;

class QWidget;

namespace casa {

class RSFileReader;
class RSFileWriter;

// Convenience class for a String/bool pair.
class RFError {
public:
    // Constructor, blank error.
    RFError();
    
    // Constructor, error with the given text and isFatal flag.
    RFError(const String& error, bool isFatal = false);
    
    // Destructor.
    ~RFError();
    
    
    // Returns whether this error was fatal or not.
    bool isFatal() const;
    
    // Returns this error's text.
    const String& error() const;
    
    // Sets the error.
    void set(const String& error, bool isFatal = false);
    
private:
    String m_error;
    bool m_fatal;
};


// Superclass for readers and writers containing common definitions and
// operations.
class RSFileReaderWriter {
public:
    // Public Static Methods //
    
    // An enum of all known subclasses/formats supported.
    enum SupportedType {
        DS9, CASA_XML
    };
    
    // Converts between enum and String for SupportedType.
    // <group>
    static SupportedType supportedType(String type);
    static String supportedType(SupportedType type);
    // </group>

    // Returns the file extension for the given SupportedType.
    static String extensionForType(SupportedType type);

    // Returns all known SupportedTypes.
    // <group>
    static vector<SupportedType> supportedTypes();
    static vector<String> supportedTypesStrings();
    // </group>

    // Returns an appropriate child RegionFileReader class for the given
    // SupportedType, or NULL for an error (shouldn't happen).
    static RSFileReader* readerForType(SupportedType type);
    
    // Returns an new appropriate child RegionFileWriter class for the given
    // SupportedType, or NULL for an error (shouldn't happen).
    static RSFileWriter* writerForType(SupportedType type);
    
    // Returns an new appropriate options widget for the given SupportedType,
    // or NULL for an error (shouldn't happen).
    static QWidget* optionsWidgetForType(SupportedType type);
    
    
    // Non-Static Members //
    
    // Constructor.
    RSFileReaderWriter() { }
    
    // Destructor.
    virtual ~RSFileReaderWriter() { }
    
    // Sets the file to be read/written to the given.
    virtual void setFile(const String& filename);
    
    // Returns the last error set during read/write.
    virtual const RFError& lastError() const;
    
protected:
    // Filename to be read/written.
    String m_filename;
    
    // Last error seen during read/write.
    RFError m_lastError;
    
    // Convenience method for setting last error during read/write.
    virtual void setError(const String& error, bool isFatal = false) const;
};


// Abstract superclass for any class that reads a format that produces
// RegionShapes from a file.
class RSFileReader : public virtual RSFileReaderWriter {
public:
    // Constructor.
    RSFileReader() { }

    // Destructor.
    virtual ~RSFileReader() { }

    // Read the filename set with setFile and returns true if no errors were
    // reported, false otherwise.  If false is returned, the details can be
    // found using lastError().  Any valid RegionShapes that were read from the
    // file are placed in the given vector (which is cleared first).
    virtual bool read(vector<RegionShape*>& readShapes) = 0;
    
    // Calls setFile() then read().
    virtual bool readFile(const String& file, vector<RegionShape*>& shapes) {
        setFile(file);
        return read(shapes);
    }
};


// Abstract superclass for any class that writes RegionShapes to a region
// file format.
class RSFileWriter : public virtual RSFileReaderWriter {
public:
    // Constructor.
    RSFileWriter() { }
    
    // Destructor.
    virtual ~RSFileWriter() { }    
    
    // Provides a custom widget that can be used to get/set options specific
    // to each format type.
    virtual QWidget* optionsWidget() const = 0;
    
    // Sets the options to the values given in the widget.  May ignore invalid
    // widgets (i.e., widgets different from the kind provided by
    // optionsWidget()).
    virtual void setOptions(const QWidget* widget) = 0;

    // Write the given regions to the filename set with setFile and returns
    // true if no errors were reported, false otherwise.  If false is returned,
    // the details can be found using lastError().
    virtual bool write(const vector<RegionShape*>& shapes) const = 0;    
    
    // Calls setFile then write.
    virtual bool writeFile(const String& filename,
                           const vector<RegionShape*>& shapes) {
        setFile(filename);
        return write(shapes);
    }
};

}

#endif /*RSFILEREADERWRITER_H_*/
