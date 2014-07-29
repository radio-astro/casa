//# TBFilterRules.qo.h: Rules used to filter rows based upon field values.
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
#ifndef TBFILTERRULES_H_
#define TBFILTERRULES_H_

#include <casaqt/QtBrowser/TBFilterRules.ui.h>
#include <casaqt/QtBrowser/TBConstants.h>

#include <QtGui>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class TBTable;
class TBData;

// <summary>
// Rule that can be used to filter rows based upon field values.
// <summary>
//
// <synopsis>
// A TBFilterRule is a single rule that consists of a field, a Comparator, and
// the rule parameters.  A rule consists of one comparator and at least one
// value.  A rule can be negated (i.e., "not equal"), or applied to any field
// (i.e., any field with a value > 5).
// </synopsis>

class TBFilterRule {
public:
    // Constructor that takes the rule parameters.  value should be the value
    // being compared against; value2 can only be NULL if
    // the comparator only takes one argument.  isNot should be true for
    // negated rules, false otherwise.  anyField should be true if the rule
    // applies to any field, false otherwise.  The rule "owns" value and value2
    // in that, upon deletion, they are deleted as well.
    TBFilterRule(String field, tb::Comparator comparator, TBData* value,
             TBData* value2 = NULL, bool isNot = false, bool anyField = false);
    
    ~TBFilterRule();

    
    // Returns the field this rule applies to, or blank if it applies to any
    // field.
    String getField();

    // Returns this rule's comparator.
    tb::Comparator getComparator();

    // Returns this rule's value.
    TBData* getValue();

    // Returns this rule's second value, or blank if it doesn't have one.
    TBData* getValue2();

    // Returns true if this rule is negated (i.e., "not equal" rather than
    // "equal"), false otherwise.
    bool getIsNot();

    // Returns true if this rule applies to any field, false otherwise.
    bool getAnyField();

    
    // Returns true if this rule is equal to the given rule, false otherwise.
    bool equals(TBFilterRule* r);

    // Checks the given row in the given table.  If the row passes, the index
    // of the first field to pass is returned; if the row does not pass, -1 is
    // returned.
    int rowPasses(TBTable* table, int row);
    
private:
    // This rule's field.
    String field;

    // This rule's comparator.
    tb::Comparator comparator;

    // This rule's value.
    TBData* value;

    // This rule's second value.
    TBData* value2;
    
    // Indicates whether this rule is negated or not.
    bool isNot;

    // Indicates whether this rule applies to any field or not.
    bool anyField;

    
    // Checks all fields in the given row.  If any of the fields pass, the
    // index of the first field to pass is returned; otherwise, -1 is returned.
    int anyFieldPasses(TBTable* table, int row);
};

// <summary>
// A sequence of TBFilterRules that can be used to filter rows.
// <summary>
//
// <synopsis>
// A TBFilterRuleSequence is basically just a list of TBFilterRules.  In the
// future, more complex sequences can be implemented such as logical clauses.
// In order for a row to pass a rule sequence, it must pass ALL rules in the
// sequence.
// </synopsis>

class TBFilterRuleSequence {
public:
    // Default Constructor.
    TBFilterRuleSequence();

    ~TBFilterRuleSequence();
    
    
    // Returns the number of rules in this sequence.
    unsigned int size();

    // Returns the rule at index i in this sequence.
    TBFilterRule* at(unsigned int i);
    
    // Returns the rules in this sequence.
    vector<TBFilterRule*>* getRules();

    
    // Checks the given row in the given table.  If the row passes ALL of the
    // rules in this sequence, the index of the first field that passed is
    // returned.  If the row fails at least one rule in the sequence, -1 is
    // returned.  See TBFilterRule::rowPasses().
    int rowPasses(TBTable* table, int row);

    // Adds the given rule to the end of the sequence.
    void addRule(TBFilterRule* rule);

    // Removes the rule at index i from this sequence.
    void removeRule(int i);

private:
    // Rule sequence.
    vector<TBFilterRule*> rules;
};

// <summary>
// Widget to allow the user to enter a filter rule sequence.
// <summary>
//
// <synopsis>
// TBFilterRules can be presented to the user as a modal dialog or as a widget
// displayed in any layout.  Once the user has finished entering the rules,
// the TBFilterRules emits a signal with the entered rules.  Important: the
// caller/parent is responsible for connecting this signal and taking care of
// the dialog.
// </synopsis>

class TBFilterRules : public QDialog, Ui::FilterRules {
    Q_OBJECT
    
public:
    // Constructor that takes a table and an optional parent parameter.  If
    // parent is NULL, the GUI is presented as a dialog.
    TBFilterRules(TBTable* table, QWidget* parent = NULL);

    ~TBFilterRules();
    
    // Renames the window title and run button text to distinguish between
    // a search and a filter.  If isSearch is true, the title is set to "Search
    // Rules" and the run button text is set to "Search"; otherwise the title
    // is set to "Filter Rules" and the run button text is set to "Run Filter."
    // The default behavior is for filter, not search.
    void renameForSearch(bool isSearch);

signals:
    // This signal is emitted when the user has entered a TBFilterRuleSequence
    // and has indicated that he/she is finished.  The entered rules are passed
    // as a parameter as is a pointer to this widget.  The parent/caller is
    // responsible for implementing the rules and deleting the dialog upon
    // completion.
    void runRequested(TBFilterRuleSequence* rules, TBFilterRules* rDialog);

private:
    // Associated table.
    TBTable* table;

    // Entered rule sequence.
    TBFilterRuleSequence* rules;

private slots:
    // Slot for the "run" button.  Emits the runRequested() signal.
    void runClicked();

    // Slot for when the field QComboBox changes.  Updates the rule parameters
    // to apply to the chosen field.
    void fieldChosen(int i);

    // Slot for when the comparator QComboBox value changes.  Updates the rule
    // parameters accordingly.
    void comparatorChosen(QString c);

    // Slot for the "add" button.  If the user has just entered a new rule,
    // it is added to the sequence.  If the user is viewing a
    // previously-entered rule, the display is cleared so that a new rule
    // can be entered.
    void addRule();

    // Slot for the "remove" button.  Removes the currently selected rule
    // from the sequence.
    void removeRule();

    // Slot for when the user selects a rule in the list.  Displays the
    // selected rule.
    void listIndexChanged(int i);
};

}

#endif /* TBFILTERRULES_H_ */
