//# PlotMSConstants.h: Constants and useful classes/methods for plotms.
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
#ifndef PLOTMSCONSTANTS_H_
#define PLOTMSCONSTANTS_H_

#include <graphics/GenericPlotter/PlotFactory.h>
#include <ms/MeasurementSets/MeasurementSet.h>

#include <map>
#include <vector>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// Useful macros for defining enums.  Although the two macros can be used
// separately, their intended use is for them to be used sequentially.
// Parameters:
// * NAME: name of the enum,
// * ALLMETHOD: name of the method that returns a vector of all defined members
//              of the enum,
// * ALLSTRMETHOD: name of the method that returns a vector of the string
//                 representation of all defined members of the enum,
// * CONVMETHOD: name of the method that converts between the enum and its
//               String representation,
// * ... (__VA_ARGS__): list of enum methods for PMS_ENUM1 and list of
//                      their string representations for PMS_ENUM2.  IMPORTANT:
//                      if both macros are used then the lists must be the same
//                      size and in the same order.
// <group>
#define PMS_ENUM1(NAME,ALLMETHOD,ALLSTRMETHOD,CONVMETHOD,...)                 \
    enum NAME {                                                               \
        __VA_ARGS__                                                           \
    };                                                                        \
                                                                              \
    static const vector< NAME >& ALLMETHOD () {                               \
        static const NAME arr[] = {                                           \
            __VA_ARGS__                                                       \
        };                                                                    \
        static const int count = sizeof(arr) / sizeof(arr[0]);                \
        static const vector< NAME > v(arr, &arr[count]);                      \
        return v;                                                             \
    }

#define PMS_ENUM2(NAME,ALLMETHOD,ALLSTRMETHOD,CONVMETHOD,...)                 \
    static const vector<String>& ALLSTRMETHOD () {                            \
        static const String arr[] = {                                         \
            __VA_ARGS__                                                       \
        };                                                                    \
        static const int count = sizeof(arr) / sizeof(arr[0]);                \
        static const vector<String> v(arr, &arr[count]);                      \
        return v;                                                             \
    }                                                                         \
                                                                              \
    static const String& CONVMETHOD ( NAME v) {                               \
        return ALLSTRMETHOD ()[v]; }                                          \
                                                                              \
    static const NAME & CONVMETHOD (const String& v, bool* ok = NULL) {       \
        const vector<String>& strs = ALLSTRMETHOD ();                         \
        const vector< NAME >& enms = ALLMETHOD ();                            \
        for(unsigned int i = 0; i < strs.size(); i++) {                       \
            if(PMS::strEq(v, strs[i], true)) {                                \
                if(ok != NULL) *ok = true;                                    \
                return enms[i];                                               \
            }                                                                 \
        }                                                                     \
        if(ok != NULL) *ok = false;                                           \
        return enms[0];                                                       \
    }
// </group>


// Container class for useful constants/methods.
class PMS {
public:
    // Enum for the axis choices that are available to be plotted.  Used both
    // by the user to select what to plot and by the cache loading system.
    // <group>
    PMS_ENUM1(Axis, axes, axesStrings, axis,
	      SCAN,FIELD,TIME,TIME_INTERVAL,
	      SPW,CHANNEL,FREQUENCY,CORR,
	      ANTENNA1,ANTENNA2,BASELINE,
	      UVDIST,UVDIST_L,U,V,W,
	      AMP,PHASE,REAL,IMAG,FLAG,
	      AZIMUTH,ELEVATION,PARANG,
	      ROW)

      // VEL_RADIO, VEL_OPTICAL, VEL_RELATIVISTIC
      // HOURANGLE, 
      // WEIGHT, FLAG_ROW)

    PMS_ENUM2(Axis, axes, axesStrings, axis,
	      "Scan","Field","Time","Time_interval",
	      "Spw","Channel","Frequency","Corr",
	      "Antenna1","Antenna2","Baseline",
	      "UVDist","UVDist_L","U","V","W",
	      "Amp","Phase","Real","Imag","Flag",
	      "Azimuth","Elevation","ParAng",
	      "Row")

    //              "time", "uvdist", "channel", "corr", "frequency", "vel_radio",
    //              "vel_optical", "vel_relativistic", "u", "v", "w", "azimuth",
    //              "elevation", "baseline", "hourangle", "parallacticangle", "amp",
    //              "phase", "real", "imag", "weight", "antenna1", "antenna2",
    //              "time_interval", "field", "scan", "spw", "flag", "flag_row")
    // </group>
              
    // Returns the axes scale for the given axis.  Currently NORMAL unless the
    // axis is TIME, in which case the scale is DATE_MJ_SEC.
    static PlotAxisScale axisScale(Axis axis);
    
    
    // Enum for the different data columns for data axes.
    // <group>
    PMS_ENUM1(DataColumn, dataColumns, dataColumnStrings, dataColumn,
              DATA, CORRECTED, MODEL, RESIDUAL)
    PMS_ENUM2(DataColumn, dataColumns, dataColumnStrings, dataColumn,
              "data", "corrected", "model", "residual")
    // </group>
              
    // Returns whether or not the given axis needs the second data parameter to
    // indicate which data column to use or not.  Currently false except for
    // AMP, PHASE, REAL, and IMAG.
    static bool axisIsData(Axis axis);
              
              
    // Enum for different axes types.  Currently only used to display this
    // information to the user in the GUI's cache tab.
    // <group>
    PMS_ENUM1(AxisType, axesTypes, axesTypeStrings, axisType,
              TBOOL, TINT, TFLOAT, TDOUBLE, TTIME)
    PMS_ENUM2(AxisType, axesTypes, axesTypeStrings, axisType,
              "boolean", "integer", "float", "double", "time")
    //</group>
             
    // Returns the type for the given axis.
    static AxisType axisType(Axis axis);
    
    
    // Enum for different axes units.  Currently only used in labels.
    // <group>
    PMS_ENUM1(AxisUnit, axesUnits, axesUnitStrings, axisUnit,
              UNONE, UDATETIME)
    PMS_ENUM2(AxisUnit, axesUnits, axesUnitStrings, axisUnit,
              "", "date-time")
    // </group>
              
    // Returns the unit for the given axis.
    static AxisUnit axisUnit(Axis axis);
    
    
    // Convert to/from dates and doubles, using the given scale (must be either
    // DATE_MJ_SEC or DATE_MJ_DAY).
    // <group>
    static double dateDouble(unsigned int year, unsigned int mon,
            unsigned int day, unsigned int hour, unsigned int min,
            double sec, PlotAxisScale scale = DATE_MJ_SEC);
    static void dateDouble(double value, unsigned int& year, unsigned int& mon,
            unsigned int& day, unsigned int& hour, unsigned int& min,
            double& sec, PlotAxisScale scale = DATE_MJ_SEC);
    // </group>    
              
    // Returns true if the given Strings are equals, false otherwise.  If
    // ignoreCase is false then it is a direct String comparison using ==;
    // otherwise the String characters are compared while ignoring case for
    // letters.
    static bool strEq(const String& str1, const String& str2,
                      bool ignoreCase = false);
    
    
    // Default Parameter Values //
    
    // Default axes for single plots.
    // <group>
    static const Axis DEFAULT_XAXIS;
    static const Axis DEFAULT_YAXIS;
    // </group>
    
    // Default data column for axes.
    static const DataColumn DEFAULT_DATACOLUMN;
    
    // Default canvas axes.
    // <group>
    static const PlotAxis DEFAULT_CANVAS_XAXIS;
    static const PlotAxis DEFAULT_CANVAS_YAXIS;
    // </group>
    
    // Default canvas axis label format, in String form.
    static const String DEFAULT_CANVAS_AXIS_LABEL_FORMAT;
    
    // Default flag to show/hide canvas axes.
    static const bool DEFAULT_SHOWAXIS;
    
    // Default legend flag and position.
    // <group>
    static const bool DEFAULT_SHOWLEGEND;
    static const PlotCanvas::LegendPosition DEFAULT_LEGENDPOSITION;
    // </group>
    
    // Default symbols for normal and masked points.
    // <group>
    static PlotSymbolPtr DEFAULT_SYMBOL(PlotFactoryPtr factory);
    static PlotSymbolPtr DEFAULT_MASKED_SYMBOL(PlotFactoryPtr factory);
    // </group>
    
    // Default title (canvas, plot, etc.) format, in String form.
    static const String DEFAULT_TITLE_FORMAT;
};


// Specifies an MS selection.  See the mssSetData method in
// ms/MeasurementSets/MSSelectionTools.h for details.
class PlotMSSelection {
public:
    // Static //
    
    // Enum and methods to define the different fields for an MS selection.
    // <group>
    PMS_ENUM1(Field, fields, fieldStrings, field,
              FIELD, SPW, TIMERANGE, UVRANGE, ANTENNA, SCAN,
              CORR, ARRAY, MSSELECT)
    PMS_ENUM2(Field, fields, fieldStrings, field,
              "field", "spw", "timerange", "uvrange", "antenna", "scan",
              "corr", "array", "msselect")
    // </group>
              
    // Returns the default value for the given selection field.  Returns an
    // empty String except for FIELD which returns "2".
    static String defaultValue(Field f);
    
    
    // Non-Static //
    
    // Default constructor.
    PlotMSSelection();
    
    // Destructor.
    ~PlotMSSelection();
    
    
    // Applies this selection using the first MS into the second MS.  (See the
    // mssSetData method in ms/MeasurementSets/MSSelectionTools.h for details.)
    void apply(MeasurementSet& ms, MeasurementSet& selectedMS,
               Matrix<Int>& chansel) const;
    
    // Gets/Sets the value for the given selection field.
    // <group>
    const String& getValue(Field f) const;
    void getValue(Field f, String& value) const { value = getValue(f); }
    void setValue(Field f, const String& value);
    // </group>
    
    // Convenience methods for returning the standard selection fields.
    // <group>
    const String& field() const     { return getValue(FIELD);     }
    const String& spw() const       { return getValue(SPW);       }
    const String& timerange() const { return getValue(TIMERANGE); }
    const String& uvrange() const   { return getValue(UVRANGE);   }
    const String& antenna() const   { return getValue(ANTENNA);   }
    const String& scan() const      { return getValue(SCAN);      }
    const String& corr() const      { return getValue(CORR);      }
    const String& array() const     { return getValue(ARRAY);     }
    const String& msselect() const  { return getValue(MSSELECT);  }
    // </group>
    
    // Convenience methods for setting the standard selection fields.
    // <group>
    void setField(const String& v)     { setValue(FIELD, v);     }
    void setSpw(const String& v)       { setValue(SPW, v);       }
    void setTimerange(const String& v) { setValue(TIMERANGE, v); }
    void setUvrange(const String& v)   { setValue(UVRANGE, v);   }
    void setAntenna(const String& v)   { setValue(ANTENNA, v);   }
    void setScan(const String& v)      { setValue(SCAN, v);      }
    void setCorr(const String& v)      { setValue(CORR, v);      }
    void setArray(const String& v)     { setValue(ARRAY, v);     }
    void setMsselect(const String& v)  { setValue(MSSELECT, v);  }
    // </group>
    
    // Equality operators.
    // <group>
    bool operator==(const PlotMSSelection& other) const;
    bool operator!=(const PlotMSSelection& other) const {
        return !(operator==(other)); }
    // </group>
    
private:    
    // Selection field values.
    map<Field, String> itsValues_;
    
    // Initializes the values to their defaults.
    void initDefaults();
};



// Specifies averaging parameters for an MS.
class PlotMSAveraging {
public:
    // Constructor, which uses default values.
    PlotMSAveraging();
    
    // Destructor.
    ~PlotMSAveraging();
    
    
    // Gets/Sets whether channel averaging is turned on or not.
    // <group>
    bool channel() const;
    void setChannel(bool channel);
    // </group>
    
    // Gets/Sets the channel averaging value, which will be a value between
    // 0 and 1, inclusive.  This value indicates the "level" of averaging
    // desired: 0 means no averaging, and 1 means full averaging.
    // <group>
    double channelValue() const;
    void setChannelValue(double value);
    // </group>
    
    
    // Equality operators.
    // <group>
    bool operator==(const PlotMSAveraging& other) const;
    bool operator!=(const PlotMSAveraging& other) const {
        return !(operator==(other)); }
    // </group>
    
private:
    // Channel averaging flag and value, respectively.
    // <group>
    bool itsChannel_;
    double itsChannelValue_;
    // </group>
    
    
    // Sets the default values.
    void setDefaults();
};


// Class for generating labels based upon axes, units, etc.  A format is a
// String that consists of tags and non-tags.  Tags are special substrings
// differentiated by being surrounded by a separator; tags are replaced with
// given values when the label is built.  For example: the %%axis%% tag is
// replaced with the name of the Axis that is passed in when generating the
// String.  Non-tags are copied exactly into the label.  An example of a format
// is: "%%xaxis%% vs. %%yaxis%%", when passed in the axes TIME and AMP, would
// return the label "time vs. amp".  Currently, labels can either be single
// axis (like for an axis label) or double axes (like for a plot title).
class PlotMSLabelFormat {
public:
    // Static //
    
    // Separator that goes before and after tags.
    static const String TAGSEPARATOR;
    
    // Tag for axes, both the single case and the double case.  This tag is
    // replaced with the axis name (see PMS::axis()) during generation.
    // <group>
    static const String TAG_AXIS;
    static const String TAG_XAXIS;
    static const String TAG_YAXIS;
    // </group>
    
    // Tag for axes units, both the single case and the double case.  This tag
    // is replaced with the axis unit name (see PMS::axisUnit()) during
    // generation.
    // <group>
    static const String TAG_UNIT;
    static const String TAG_XUNIT;
    static const String TAG_YUNIT;
    // </group>
    
    // Tags for if/endif for axes units.  Parts of the format that are
    // surrounded with an IF at the beginning and ENDIF at the end are only
    // copied to the label if the given unit is NOT PMS::UNONE.  For example,
    // "TAG_IF_UNIT(sample text)TAG_ENDIF_UNIT" would copy over "(sample text)"
    // only if the given unit was NOT PMS::UNONE.
    // <group>
    static const String TAG_IF_UNIT;
    static const String TAG_IF_XUNIT;
    static const String TAG_IF_YUNIT;
    static const String TAG_ENDIF_UNIT;
    static const String TAG_ENDIF_XUNIT;
    static const String TAG_ENDIF_YUNIT;
    // </group>
    
    // Convenience method to surround the given tag with the separator.
    static String TAG(const String& tag) {
        return TAGSEPARATOR + tag + TAGSEPARATOR; }
    
    
    // Non-Static //
    
    // Constructor which takes an optional starting format.
    PlotMSLabelFormat(const String& format = "");
    
    // Copy constructor.  See operator=().
    PlotMSLabelFormat(const PlotMSLabelFormat& copy);
    
    // Destructor.
    ~PlotMSLabelFormat();
    
    
    // Format.
    String format;
    
    
    // Generates a label, using the given single axis.  If any double axes tags
    // are in the format, the given axis will be used for them.
    String getLabel(PMS::Axis axis) const;
    
    // Generates a label, using the given double axes.  If any single axes tags
    // are in the format, the x axis will be used for it.
    String getLabel(PMS::Axis xAxis, PMS::Axis yAxis) const;
    
    // Equality operators.
    // <group>
    bool operator==(const PlotMSLabelFormat& other) const;
    bool operator!=(const PlotMSLabelFormat& other) const {
        return !(operator==(other)); }
    // </group>
    
    // Copy operator.
    PlotMSLabelFormat& operator=(const PlotMSLabelFormat& copy);
    
private:
    // Generates a label using the given format, single axis, and double axes.
    static String getLabel(const String& format, PMS::Axis axis,
            PMS::Axis xAxis, PMS::Axis yAxis);
    
    // Helper method for getLabel() which gets the next token in the format.
    // Returns true if a token was returned; false if the end of the format was
    // reached.  The given format will be automatically shortened as tokens are
    // taken out.  The next token is put in the token parameter, and the
    // tokenWasTag parameter is set to true if the token was a tag, false
    // otherwise.
    static bool nextToken(String& format, String& token, bool& tokenWasTag);
};

}

#endif /* PLOTMSCONSTANTS_H_ */
