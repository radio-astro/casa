//# PlotMSLabelFormat.h: Class for generating labels based on a format.
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
#ifndef PLOTMSLABELFORMAT_H_
#define PLOTMSLABELFORMAT_H_

#include <casa/BasicSL/String.h>
#include <plotms/PlotMS/PlotMSConstants.h>

#include <casa/namespace.h>

namespace casa {

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
    static const String& TAGSEPARATOR();
    
    // Tag for axes, both the single case and the double case.  This tag is
    // replaced with the axis name (see PMS::axis()) during generation.
    // <group>
    static const String& TAG_AXIS();
    static const String& TAG_XAXIS();
    static const String& TAG_YAXIS();
    // </group>
    
    // Tag for axes units, both the single case and the double case.  This tag
    // is replaced with the axis unit name (see PMS::axisUnit()) during
    // generation.
    // <group>
    static const String& TAG_UNIT();
    static const String& TAG_XUNIT();
    static const String& TAG_YUNIT();
    // </group>
    
    // Tags for if/endif for axes units.  Parts of the format that are
    // surrounded with an IF at the beginning and ENDIF at the end are only
    // copied to the label if the given unit is NOT PMS::UNONE.  For example,
    // "TAG_IF_UNIT(sample text)TAG_ENDIF_UNIT" would copy over "(sample text)"
    // only if the given unit was NOT PMS::UNONE.
    // <group>
    static const String& TAG_IF_UNIT();
    static const String& TAG_IF_XUNIT();
    static const String& TAG_IF_YUNIT();
    static const String& TAG_ENDIF_UNIT();
    static const String& TAG_ENDIF_XUNIT();
    static const String& TAG_ENDIF_YUNIT();
    // </group>
    
    // Convenience method to surround the given tag with the separator.
    static String TAG(const String& tag);
    
    
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

#endif /* PLOTMSLABELFORMAT_H_ */
