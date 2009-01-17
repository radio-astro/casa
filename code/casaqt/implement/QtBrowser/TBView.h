//# TBView.h: Current "view" or state of the browser that can be serialized.
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
//# $Id: $
#ifndef TBVIEW_H_
#define TBVIEW_H_

#include <vector>

#include <xercesc/dom/DOM.hpp>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace std;
using namespace xercesc;

namespace casa {

//# Forward Declarations
class TBFilterRuleSequence;
class TBFormat;
class QFontColor;

// <summary>
// Current "view" or state of a single table.
// </summary>
//
// <synopsis>
// The TBTableView defines the state of an opened table and includes: the
// location, whether the table is the currently selected table, which
// columns are hidden, the visual index of each column, the loaded rows, the
// row filter (if any), the field formats (if any), and the sort order (if any).
// </synopsis>

class TBTableView {
public:
    // Default Constructor.
    TBTableView();

    ~TBTableView();

    
    // Location of the table.
    String location;
    
    // Whether the table is currently selected.
    bool selected;
    
    // Whether each column is hidden (true) or not (false).
    vector<bool> hidden;
    
    // The visual index of each column in case they were moved.
    vector<int> visInd;
    
    // The starting number of the loaded rows.
    int loadedFrom;
    
    // The number of loaded rows.
    int loadedNum;
    
    // Row filter, or NULL if there is none.
    TBFilterRuleSequence* filter;
    
    // The format for each field, or NULL if that field has none.
    vector<TBFormat*> formats;
    
    // The current sort order, or an empty list if there is none.
    vector<pair<String, bool> > sort;
    
    // Whether this table is from a TaQL command or not.
    bool taql;
};

// <summary>
// Current "view" or state of the browser that can be serialized.
// </summary>
//
// <synopsis>
// The TBView defines the state of the browser and includes: the last opened
// directory, and the TBTableViews of all currently opened tables.  TBView
// has methods to save to and load from a file on disk.
// </synopsis>

class TBView {
public:
    // Default Constructor.
    TBView();

    ~TBView();

    
    // Adds the given table view to this view.
    void addTableView(TBTableView* view);

    // Returns the list of table views in this view.
    vector<TBTableView*>* getTableViews();

    // Sets the last opened directory for this view.
    void setLastOpenedDirectory(String str);

    // Returns the last opened directory for this view.
    String getLastOpenedDirectory();

    
    // Saves this view in XML format to the given file.
    bool saveToFile(String file);

    // Equivalent to saveToFile(defaultFile()).
    bool saveToDefaultFile();

    
    // Loads the view saved in the given file and returns it.  If the file
    // is invalid or there are other problems, NULL is returned.
    static TBView* loadFromFile(String file);

    // Equivalent to loadFromFile(defaultFile()).
    static TBView* loadFromDefaultFile();
    
    // Returns the default location to save and load a view:
    // TBConstants::dotCasapyDir() + TBConstants::VIEW_SAVE_LOC.
    static String defaultFile();

private:
    // Last opened directory.
    String lastOpenedDir;
    
    // Table views.
    vector<TBTableView*> views;

    
    // Creates an XML element based upon the given QFontColor.
    static void fontElem(DOMElement* elem, QFontColor* font);

    // Creates a QFontColor based upon the given XML element.
    static QFontColor* fontElem(DOMElement* elem);
};

/** XML format for view file:

    <casabrowser>
        <view lastdir="[lastdir]">
            <table location="[location]" selected="[0/1]" taql="[0/1]">
                <visibleIndices length="[#]">[visible indices vector
                                             </visibleIndices>
                <hidden length="[#]">[hidden column vector]</hidden>
                <rows loadedFrom="[#]" loadedNum="[#]" />
                <filter>
                    <rule field="[field]" comparator="[comparator]"
                          value="[value]" value2="[value2]" not="[0/1]"
                          any="[0/1]" type="[type]">
                </filter>
                <formats length="[#]">
                    <format col="[#]" decimals="[#]" sformat="[0/1]"
                     bformat="[bformat]" dformat="[dformat]" vthreshold="[#]">
                        <allfont color="[#color]" family="[family]" size="[#]"
                         bold="[0/1]" italics="[0/1]" uline="[0/1]"
                         strike="[0/1]">
                        <font col="[#]" color="[#color]" family="[family]"
                         size="[#]" bold="[0/1]" italics="[0/1]" uline="[0/1]"
                         strike="[0/1]">
                    </format>
                </formats>
                <sort length="[#]">
                    <field name="[name]" ascending="[0/1]" />
                </sort>
            </table>
        </view>
    </casabrowser>
    
*/

}

#endif /* TBVIEW_H_ */
