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

#include <casa/Containers/Record.h>
#include <graphics/GenericPlotter/PlotFactory.h>

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
//              of the enum (also nALLMETHOD which returns the number of
//              defined members in the enum),
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
    }                                                                         \
                                                                              \
    static unsigned int n##ALLMETHOD () {                                     \
        static unsigned int n = ALLMETHOD ().size();                          \
        return n;                                                             \
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
    // **If these are changed, also update: xmlcasa/tasks/plotms.xml,
    // xmlcasa/scripts/task_plotms.py.**
    // <group>
    PMS_ENUM1(Axis, axes, axesStrings, axis,
	      SCAN,FIELD,TIME,TIME_INTERVAL,
	      SPW,CHANNEL,FREQUENCY,VELOCITY,CORR,
	      ANTENNA1,ANTENNA2,BASELINE,ROW,
	      AMP,PHASE,REAL,IMAG,WT,WTxAMP,
	      FLAG,FLAG_ROW,
	      UVDIST,UVDIST_L,U,V,W,UWAVE,VWAVE,WWAVE,
	      AZ0,EL0,HA0,PA0,
	      ANTENNA,AZIMUTH,ELEVATION,
	      PARANG,
	      GAMP,GPHASE,GREAL,GIMAG,
	      DELAY,SWP,TSYS,OPAC,
	      RADIAL_VELOCITY, RHO,
	      NONE)

    PMS_ENUM2(Axis, axes, axesStrings, axis,
	      "Scan","Field","Time","Interval",
	      "Spw","Channel","Frequency","Velocity","Corr",
	      "Antenna1","Antenna2","Baseline","Row",
	      "Amp","Phase","Real","Imag","Wt","Wt*Amp",
	      "Flag","FlagRow",
	      "UVdist","UVwave","U","V","W","Uwave","Vwave","Wwave",
	      "Azimuth","Elevation","HourAngle","ParAngle",
	      "Antenna","Ant-Azimuth","Ant-Elevation","Ant-ParAngle",
	      "GainAmp","GainPhase","GainReal","GainImag",
	      "Delay","SwPower","Tsys","Opac","Radial Velocity [km/s]", "Distance (rho) [km]",
	      "None")

    // </group>
              
    // Returns the axes scale for the given axis.  Currently NORMAL unless the
    // axis is TIME, in which case the scale is DATE_MJ_SEC.
    static PlotAxisScale axisScale(Axis axis);
    
    
    // Enum for the different data columns for data axes.
    // **If these are changed, also update: xmlcasa/tasks/plotms.xml.**
    // <group>
    PMS_ENUM1(DataColumn, dataColumns, dataColumnStrings, dataColumn,
              DATA, CORRECTED, MODEL, CORRMODEL, DATAMODEL)
    PMS_ENUM2(DataColumn, dataColumns, dataColumnStrings, dataColumn,
              "data", "corrected", "model", "corrected-model", "data-model")
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
              UNONE, UDATETIME/*, HERTZ, METERS_PER_SECOND, KILOMETERS_PER_SECOND,
              KILOMETERS, METERS*/);
    PMS_ENUM2(AxisUnit, axesUnits, axesUnitStrings, axisUnit,
              "", "date-time"/*"s", "Hz", "m/s", "km/s", "km", "m"*/);

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
    
    // Returns true if the given Records are equals, false otherwise.
    static bool recEq(const Record& rec1, const Record& rec2);
    
    // Converts the given templated vector to/from an int Vector.
    // <group>
    template <class T>
    static Vector<int> toIntVector(const vector<T>& v) {
        Vector<int> v2(v.size());
        for(unsigned int i = 0; i < v.size(); i++) v2[i] = (int)v[i];
        return v2;
    }
    
    template <class T>
    static vector<T> fromIntVector(const Vector<int>& v) {
        vector<T> v2(v.size());
        for(unsigned int i = 0; i < v.size(); i++) v2[i] = (T)v[i];
        return v2;
    }
    // </group>
    
    
    // Enum for the different MS summary types.
    // <group>
    PMS_ENUM1(SummaryType, summaryTypes, summaryTypeStrings, summaryType,
              S_ALL, S_WHERE, S_WHAT, S_HOW, S_MAIN, S_TABLES, S_ANTENNA,
              S_FEED, S_FIELD, S_OBSERVATION, S_HISTORY, S_POLARIZATION,
              S_SOURCE, S_SPW, S_SPW_POL,
              S_SYSCAL, S_WEATHER)

    PMS_ENUM2(SummaryType, summaryTypes, summaryTypeStrings, summaryType,
              "All", "Where", "What", "How", "Main", "Tables", "Antenna",
              "Feed", "Field", "Observation", "History", "Polarization",
              "Source", "Spectral Window", "Spectral Window and Polarization",
              "SysCal", "Weather")
    // </group>
              

   // Enum for export range.
   // <group>
   PMS_ENUM1(ExportRange, exportRanges, exportRangeStrings, exportRange, PAGE_CURRENT, PAGE_ALL)
   PMS_ENUM2(ExportRange, exportRanges, exportRangeStrings, exportRange, "Current Page", "All Pages")
   // </group>
              

    // Colorizing Values //
              
    // Returns the list of unique colors used to colorize plots.
    static const vector<String>& COLORS_LIST();
    
    
    // Default Parameter Values //
    
    // Default values for PlotMSParameters.
    // <group>
    static const String DEFAULT_LOG_FILENAME;
    static const int DEFAULT_LOG_EVENTS;
    static const LogMessage::Priority DEFAULT_LOG_PRIORITY;
    static const bool DEFAULT_CLEAR_SELECTIONS;
    static const int DEFAULT_CACHED_IMAGE_WIDTH;
    static const int DEFAULT_CACHED_IMAGE_HEIGHT;
    static const int DEFAULT_GRID_ROWS;
    static const int DEFAULT_GRID_COLS;
    // </group>
    
    // Default values for PMS_PP_Cache.
    // <group>
    static const Axis DEFAULT_XAXIS;
    static const Axis DEFAULT_YAXIS;
    static const DataColumn DEFAULT_DATACOLUMN;
    static const Axis DEFAULT_COLOR_AXIS;
    // </group>
    
    // Default values for PMS_PP_Canvas.
    // <group>
    static const PlotAxis DEFAULT_CANVAS_XAXIS;
    static const PlotAxis DEFAULT_CANVAS_YAXIS;
    static const String DEFAULT_CANVAS_AXIS_LABEL_FORMAT;
    static const bool DEFAULT_SHOWAXIS;
    static const bool DEFAULT_SHOWLEGEND;
    static const PlotCanvas::LegendPosition DEFAULT_LEGENDPOSITION;
    static const bool DEFAULT_SHOW_GRID;
    static PlotLinePtr DEFAULT_GRID_LINE(PlotFactoryPtr factory);
    static const String DEFAULT_TITLE_FORMAT;
    // </group>
    
    // Default values for export range;
    static const ExportRange DEFAULT_EXPORT_RANGE;

    // Default values for PMS_PP_Display.
    // <group>
    static PlotSymbolPtr DEFAULT_UNFLAGGED_SYMBOL(PlotFactoryPtr factory);
    static PlotSymbolPtr DEFAULT_FLAGGED_SYMBOL(PlotFactoryPtr factory);
    // </group>
    
    // Returns the minimum visible sizes for plot symbol types.
    static map<PlotSymbol::Symbol, int> SYMBOL_MINIMUM_SIZES();
    
    // Default text annotation properties.
    // <group>
    static PlotFontPtr DEFAULT_ANNOTATION_TEXT_FONT(PlotFactoryPtr factory);
    static PlotLinePtr DEFAULT_ANNOTATION_TEXT_OUTLINE(PlotFactoryPtr factory);
    static PlotAreaFillPtr DEFAULT_ANNOTATION_TEXT_BACKGROUND(
            PlotFactoryPtr factory);
    // </group>
    
    // Default rectangle annotation properties.
    // <group>
    static PlotLinePtr DEFAULT_ANNOTATION_RECT_LINE(PlotFactoryPtr factory);
    static PlotAreaFillPtr DEFAULT_ANNOTATION_RECT_FILL(PlotFactoryPtr f);
    // </group>
    
    
    // Logging Constants //
    
    // Log class origin.
    static const String LOG_ORIGIN;
    
    // Log event origin names.
    // <group>
    static const String LOG_ORIGIN_DBUS;
    static const String LOG_ORIGIN_FLAG;
    static const String LOG_ORIGIN_LOAD_CACHE;
    static const String LOG_ORIGIN_LOCATE;
    static const String LOG_ORIGIN_PARAMS_CHANGED;
    static const String LOG_ORIGIN_PLOT;
    static const String LOG_ORIGIN_RELEASE_CACHE;
    static const String LOG_ORIGIN_UNFLAG;
    static const String LOG_ORIGIN_SUMMARY;
    // </group>
    
    // Log event flags.
    // <group>
    static const int LOG_EVENT_DBUS;
    static const int LOG_EVENT_FLAG;
    static const int LOG_EVENT_LOAD_CACHE;
    static const int LOG_EVENT_LOCATE;
    static const int LOG_EVENT_PARAMS_CHANGED;
    static const int LOG_EVENT_PLOT;
    static const int LOG_EVENT_RELEASE_CACHE;
    static const int LOG_EVENT_UNFLAG;
    static const int LOG_EVENT_SUMMARY;
    // </group>
};

}

#endif /* PLOTMSCONSTANTS_H_ */
