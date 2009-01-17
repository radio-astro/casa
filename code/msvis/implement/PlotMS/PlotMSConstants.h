//# PlotMSConstants.h: Constants and useful methods for plotms.
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
//#
//# $Id:  $
#ifndef PLOTMSCONSTANTS_H_
#define PLOTMSCONSTANTS_H_

#include <casa/BasicSL/String.h>
#include <graphics/GenericPlotter/PlotOptions.h>

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


// Container class for useful constants.
class PMS {
public:
    // Enum for the axis choices that are available to be plotted.  Used both
    // by the user to select what to plot and by the cache loading system.
    // <group>
    PMS_ENUM1(Axis, axes, axesStrings, axis,
              TIME, UVDIST, CHANNEL, CORR, FREQUENCY, VEL_RADIO,
              VEL_OPTICAL, VEL_RELATIVISTIC, U, V, W, AZIMUTH,
              ELEVATION, BASELINE, HOURANGLE, PARALLACTICANGLE, AMP,
              PHASE, REAL, IMAG, WEIGHT, ANTENNA1, ANTENNA2,
              TIME_INTERVAL, FIELD, SCAN, SPW, FLAG, FLAG_ROW)
    PMS_ENUM2(Axis, axes, axesStrings, axis,
              "time", "uvdist", "channel", "corr", "frequency", "vel_radio",
              "vel_optical", "vel_relativistic", "u", "v", "w", "azimuth",
              "elevation", "baseline", "hourangle", "parallacticangle", "amp",
              "phase", "real", "imag", "weight", "antenna1", "antenna2",
              "time_interval", "field", "scan", "spw", "flag", "flag_row")
    // </group>
              
    // Returns the axes scale for the given axis.  Currently NORMAL unless the
    // axis is TIME, in which case the scale is DATE_MJ_SEC.
    static PlotAxisScale axisScale(Axis axis);
              
    // Returns whether or not the given axis needs the second data parameter to
    // indicate which data column to use or not.  Currently false except for
    // AMP, PHASE, REAL, and IMAG.
    static bool axisIsData(Axis axis);
    
    // Enum for the different data columns.
    // <group>
    PMS_ENUM1(DataColumn, dataColumns, dataColumnStrings, dataColumn,
              DATA, CORRECTED, MODEL, RESIDUAL)
    PMS_ENUM2(DataColumn, dataColumns, dataColumnStrings, dataColumn,
              "data", "corrected", "model", "residual")
    // </group>
              
    // Returns true if the given Strings are equals, false otherwise.  If
    // ignoreCase is false then it is a direct String comparison using ==;
    // otherwise the Strings are compared while ignoring case.
    static bool strEq(const String& str1, const String& str2,
                      bool ignoreCase = false);
};

}

#endif /* PLOTMSCONSTANTS_H_ */
