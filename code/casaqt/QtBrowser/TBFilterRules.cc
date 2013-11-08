//# TBFilterRules.cc: Rules used to filter rows based upon field values.
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
#include <casaqt/QtBrowser/TBFilterRules.qo.h>
#include <casaqt/QtBrowser/TBField.h>
#include <casaqt/QtBrowser/TBTable.h>
#include <casaqt/QtBrowser/TBArray.h>
#include <casaqt/QtBrowser/TBData.h>

namespace casa {

//////////////////////////////
// TBFILTERRULE DEFINITIONS //
//////////////////////////////

// Constructors/Destructors //

TBFilterRule::TBFilterRule(String f, Comparator c, TBData* v, TBData* v2,
        bool n, bool a): field(f), comparator(c), value(v), value2(v2),
        isNot(n), anyField(a) { }

TBFilterRule::~TBFilterRule() {
    delete value;
    delete value2;
}

// Accessor Methods //

String TBFilterRule::getField() { return field; }

Comparator TBFilterRule::getComparator() { return comparator; }

TBData* TBFilterRule::getValue() { return value; }

TBData* TBFilterRule::getValue2() { return value2; }

bool TBFilterRule::getAnyField() { return anyField; }

bool TBFilterRule::getIsNot() { return isNot; }

// Public Methods //

bool TBFilterRule::equals(TBFilterRule* r) {
    if(r == NULL || r->field != field || r->comparator != comparator ||
       r->isNot != isNot || r->anyField != r->anyField)
        return false;

    if(!r->value->equals(value)) return false;
    
    if(r->value2 == NULL && value2 == NULL) return true;
    else if(r->value2 == NULL || value2 == NULL) return false;
    else return r->value2->equals(value2);
}

int TBFilterRule::rowPasses(TBTable* table, int row) {
    if(anyField) return anyFieldPasses(table, row);

    // Find index of the field
    unsigned int j = 0;
    for(; j < table->getFields()->size(); j++) {
        if(table->getFields()->at(j)->getName() == field)
            break;
    }

    if(j >= table->getFields()->size())
        return -1;
    
    TBData* data = table->dataAt(row, j);
    String type = data->getType();

    if(comparator == EQUALS) {
        bool b = data->equals(value);
        if((b && isNot) || (!b && !isNot)) return -1;
        
    } else if(comparator == CONTAINS) {
        bool wasLoaded = ((TBArrayData*)data)->isLoaded();
        TBArrayData* array = table->loadArray(row, j);
        bool b = array == NULL;
        if(!b) {
            bool contains = array->contains(value);
            b = (!isNot && !contains) || (isNot && contains);
        }
        if(!wasLoaded) array->release();
        if(b) return -1;
        
    } else if(comparator == BETWEEN) {
        double v = data->asDouble();
        double v1 = value->asDouble();
        double v2 = value2->asDouble();

        if(v2 < v1) {
            double temp = v2;
            v2 = v1;
            v1 = temp;
        }

        if((!isNot && (v < v1 || v > v2)) ||
           (isNot && (v1 <= v && v <= v2))) return -1;
           
    } else if(comparator == CONTAINSBT) {
        bool wasLoaded = ((TBArrayData*)data)->isLoaded();
        TBArrayData* array = table->loadArray(row, j);
        bool b = array == NULL;
        if(!b) {
            bool contains = array->containsBetween(value, value2);
            b = (!isNot && !contains) || (isNot && contains);
        }
        if(!wasLoaded) array->release();
        if(b) return -1;
        
    } else if(comparator == LESSTHAN) {
        double v = data->asDouble();
        double v1 = value->asDouble();
        if((!isNot && v >= v1) || (isNot && v < v1)) return -1;
        
    } else if(comparator == CONTAINSLT) {
        bool wasLoaded = ((TBArrayData*)data)->isLoaded();
        TBArrayData* array = table->loadArray(row, j);
        bool b = array == NULL;
        if(!b) {
            bool contains = array->containsLessThan(value);
            b = (!isNot && !contains) || (isNot && contains);
        }
        if(!wasLoaded) array->release();
        if(b) return -1;
        
    } else if(comparator == GREATERTHAN) {
        double v = data->asDouble();
        double v1 = value->asDouble();
        if((!isNot && v <= v1) || (isNot && v > v1)) return -1;
        
    } else if(comparator == CONTAINSGT) {
        bool wasLoaded = ((TBArrayData*)data)->isLoaded();
        TBArrayData* array = table->loadArray(row, j);
        bool b = array == NULL;
        if(!b) {
            bool contains = array->containsGreaterThan(value);
            b = (!isNot && !contains) || (isNot && contains);
        }
        if(!wasLoaded) array->release();
        if(b) return -1;
    }

    // If we've gotten to this point, the row passed
    return j;
}

// Private Methods //

int TBFilterRule::anyFieldPasses(TBTable* table, int row) {
    unsigned int j = 0;
    for(; j < table->getFields()->size(); j++) {
        TBData* data = table->dataAt(row, j);
        String type = data->getType();

        // make sure type and comparator are compatable
        if(TBConstants::typeIsArray(type)) {
            if(comparator == EQUALS || comparator == BETWEEN ||
               comparator == LESSTHAN || comparator == GREATERTHAN)
                continue;
                        
            String t = TBConstants::arrayType(type);
            if(!TBConstants::typeIsNumberable(t) && (comparator== CONTAINSBT ||
               comparator == CONTAINSLT || comparator == CONTAINSGT)) continue;
        } else {
            if(!TBConstants::typeIsNumberable(type) && (comparator== BETWEEN ||
               comparator == LESSTHAN || comparator == GREATERTHAN)) continue;
        }

        if(comparator == EQUALS) {
            bool b;
            if(TBConstants::typeIsNumberable(type))
                b = data->asDouble() == value->asDouble();
            else
                b = data->asString() == value->asString();
            
            if((b && !isNot) || (!b && isNot)) return j;

        } else if(comparator == CONTAINS) {
            bool wasLoaded = ((TBArrayData*)data)->isLoaded();
            TBArrayData* array = table->loadArray(row, j);
            bool b = array != NULL;
            if(b) {
                TBData* d = TBData::create(value->asString(), type);
                if(d != NULL) {
                    bool bc = array->contains(d);
                    b = (!isNot && bc) || (isNot && !bc);
                }
                delete d;
            }
                    
            if(!wasLoaded) array->release();
            if(b) return j;
        
        } else if(comparator == BETWEEN) {
            double v = data->asDouble();
            double v1 = value->asDouble();
            double v2 = value2->asDouble();

            if(v2 < v1) {
                double temp = v2;
                v2 = v1;
                v1 = temp;
            }

            if((!isNot && v >= v1 && v <= v2) || (isNot && (v < v1 || v > v2)))
                return j;
            
        } else if(comparator == CONTAINSBT) {
            bool wasLoaded = ((TBArrayData*)data)->isLoaded();
            TBArrayData* array = table->loadArray(row, j);
            bool b = array != NULL;
            if(b) {
                TBData* d = TBData::create(value->asString(), type);
                TBData* d2 = TBData::create(value2->asString(), type);
                if(d != NULL && d2 != NULL) {
                    bool bc = array->containsBetween(d, d2);
                    b = (!isNot && bc) || (isNot && !bc);
                }
                if(d != NULL) delete d;
                if(d2 != NULL) delete d2;
            }
                
            if(!wasLoaded) array->release();
            if(b) return j;
            
        } else if(comparator == LESSTHAN) {
            double v = data->asDouble();
            double v1 = value->asDouble();
            if((!isNot && v < v1) || (isNot && v >= v1)) return j;
            
        } else if(comparator == CONTAINSLT) {
            bool wasLoaded = ((TBArrayData*)data)->isLoaded();
            TBArrayData* array = table->loadArray(row, j);
            bool b = array != NULL;
            if(b) {
                TBData* d = TBData::create(value->asString(), type);
                if(d != NULL) {
                    bool bc = array->containsLessThan(d);
                    b = (!isNot && bc) || (isNot && !bc);
                    delete d;
                }
            }
                    
            if(!wasLoaded) array->release();
            if(b) return j;
            
        } else if(comparator == GREATERTHAN) {
            double v = data->asDouble();
            double v1 = value->asDouble();
            if((!isNot && v > v1) || (isNot && v <= v1)) return j;
            
        } else if(comparator == CONTAINSGT) {
            bool wasLoaded = ((TBArrayData*)data)->isLoaded();
            TBArrayData* array = table->loadArray(row, j);
            bool b = array != NULL;
            if(b) {
                TBData* d = TBData::create(value->asString(), type);
                if(d != NULL) {
                    bool bc = array->containsGreaterThan(d);
                    b = (!isNot && bc) || (isNot && !bc);
                    delete d;
                }
            }
                    
            if(!wasLoaded) array->release();
            if(b) return j;
        }
    }
            
    return -1;
}

//////////////////////////////////////
// TBFILTERRULESEQUENCE DEFINITIONS //
//////////////////////////////////////

// Constructors/Destructors //

TBFilterRuleSequence::TBFilterRuleSequence(): rules() { }

TBFilterRuleSequence::~TBFilterRuleSequence() {
    for(unsigned int i = 0; i < rules.size(); i++)
        delete rules.at(i);
}

// Accessors/Mutators //

unsigned int TBFilterRuleSequence::size() { return rules.size(); }

TBFilterRule* TBFilterRuleSequence::at(unsigned int i) {
    if(i < rules.size()) return rules.at(i);
    else return NULL;
}

vector<TBFilterRule*>* TBFilterRuleSequence::getRules() { return &rules; }

// Public Methods //

int TBFilterRuleSequence::rowPasses(TBTable* table, int row) {
    int index = -1;
    // row must pass ALL tests
    for(unsigned int i = 0; i < rules.size(); i++) {
        int j = rules.at(i)->rowPasses(table, row);
        if(j == -1) return -1;
        if(index == -1) index = j;
    }

    return index;
}

void TBFilterRuleSequence::addRule(TBFilterRule* rule) {
    rules.push_back(rule);
}

void TBFilterRuleSequence::removeRule(int i) {
    TBFilterRule* r = rules.at(i);
    rules.erase(rules.begin() + i);
    delete r;
}

///////////////////////////////
// TBFILTERRULES DEFINITIONS //
///////////////////////////////

// Constructors/Destructors //

TBFilterRules::TBFilterRules(TBTable* t, QWidget* parent): QDialog(parent),
                                                           table(t) {
    setupUi(this);
    rules = NULL;

    if(table != NULL) {
        rules = new TBFilterRuleSequence();
        
        // add fields
        vector<TBField*>* fields = table->getFields();
        for(unsigned int i = 0; i < fields->size(); i++)
            fieldChooser->addItem(fields->at(i)->getName().c_str());
        fieldChooser->addItem("[ any field ]");

        // Connect widgets
        connect(fieldChooser, SIGNAL(currentIndexChanged(int)),
                this, SLOT(fieldChosen(int)));
        connect(comparatorChooser, SIGNAL(currentIndexChanged(QString)),
                this, SLOT(comparatorChosen(QString)));
        fieldChosen(0);

        connect(addButton, SIGNAL(clicked()), this, SLOT(addRule()));
        connect(removeButton, SIGNAL(clicked()), this, SLOT(removeRule()));
        connect(listWidget, SIGNAL(currentRowChanged(int)),
                this, SLOT(listIndexChanged(int)));
    }

    connect(runButton, SIGNAL(clicked()), this, SLOT(runClicked()));
}

TBFilterRules::~TBFilterRules() { }


void TBFilterRules::renameForSearch(bool isSearch) {
    setWindowTitle(isSearch ? "Search Rules" : "Filter Rules");
    runButton->setText(isSearch ? "Search" : "Run Filter");
}

// Private Slots //

void TBFilterRules::runClicked() { emit runRequested(rules, this); }

void TBFilterRules::fieldChosen(int i) {
    comparatorChooser->clear();
    
    if(i == fieldChooser->count() - 1) {
        comparatorChooser->addItem(TBConstants::compToString(EQUALS).c_str());
        comparatorChooser->addItem(TBConstants::compToString(BETWEEN).c_str());
        comparatorChooser->addItem(TBConstants::compToString(
                LESSTHAN).c_str());
        comparatorChooser->addItem(TBConstants::compToString(
                GREATERTHAN).c_str());
        comparatorChooser->addItem(TBConstants::compToString(
                CONTAINS).c_str());
        comparatorChooser->addItem(TBConstants::compToString
                (CONTAINSBT).c_str());
        comparatorChooser->addItem(TBConstants::compToString(
                CONTAINSLT).c_str());
        comparatorChooser->addItem(TBConstants::compToString(
                CONTAINSGT).c_str());
        
    } else {
        String type = table->getFields()->at(i)->getType();
        if(!TBConstants::typeIsArray(type)) {
            comparatorChooser->addItem(TBConstants::compToString(
                    EQUALS).c_str());

            if(TBConstants::typeIsNumberable(type)) {
                comparatorChooser->addItem(TBConstants::compToString(
                        BETWEEN).c_str());
                comparatorChooser->addItem(TBConstants::compToString(
                        LESSTHAN).c_str());
                comparatorChooser->addItem(TBConstants::compToString(
                        GREATERTHAN).c_str());
            }
            
        } else {
            comparatorChooser->addItem(TBConstants::compToString(
                    CONTAINS).c_str());

            type = TBConstants::arrayType(type);
            if(TBConstants::typeIsNumberable(type)) {
                comparatorChooser->addItem(TBConstants::compToString(
                        CONTAINSBT).c_str());
                comparatorChooser->addItem(TBConstants::compToString(
                        CONTAINSLT).c_str());
                comparatorChooser->addItem(TBConstants::compToString(
                        CONTAINSGT).c_str());
            }
        }
    }
}

void TBFilterRules::comparatorChosen(QString cs) {
    Comparator c = TBConstants::stringToComp(qPrintable(cs));
    bool b = c == BETWEEN || c == CONTAINSBT;
    valueEdit2->setEnabled(b);
    andLabel->setEnabled(b);

    if(c== CONTAINS || c== CONTAINSBT || c == CONTAINSLT || c == CONTAINSGT) {
        notChooser->setItemText(0, "does");
        notChooser->setItemText(1, "does not");
    } else {
        notChooser->setItemText(0, "is");
        notChooser->setItemText(1, "is not");
    }
}

void TBFilterRules::addRule() {
    // If a rule from the list is being displayed, clear the fields and return
    if(addButton->text() == "Clear") {
        valueEdit->setText("");
        valueEdit2->setText("");
        listWidget->setCurrentRow(-1);
        listWidget->clearSelection();

        notChooser->setEnabled(true);
        fieldChooser->setEnabled(true);
        comparatorChooser->setEnabled(true);
        valueEdit->setEnabled(true);
        addButton->setText("Add Rule");
        fieldChosen(fieldChooser->currentIndex());
        return;
    }

    // Otherwise, collect entered info and add the rule
    String field = qPrintable(fieldChooser->currentText());
    String comp = qPrintable(comparatorChooser->currentText());
    String value = qPrintable(valueEdit->text());
    TBConstants::strtrim(value);
    if(field.empty() || comp.empty() || value.empty()) return;

    Comparator comparator = TBConstants::stringToComp(comp);

    String value2;
    bool anyField = fieldChooser->currentIndex() == fieldChooser->count() - 1;
    bool isNot = notChooser->currentIndex() == 1;
    bool valid = true;
    
    TBData* d1 = NULL;
    TBData* d2 = NULL;

    // check validity
    if(anyField) {
        if(comparator != EQUALS && comparator != CONTAINS) {
            String dt = TBConstants::TYPE_DATE;
            double d;
            
            if(TBConstants::atod(value, &d) != 1 &&
               !TBConstants::valueIsValid(value, dt))
                valid = false;
            
            if(valid && (comparator == BETWEEN || comparator == CONTAINSBT)) {
                value2 = qPrintable(valueEdit2->text());
                if(TBConstants::atod(value2, &d) != 1 &&
                   !TBConstants::valueIsValid(value2, dt))
                    valid = false;
            }
        }
        
        if(valid) {
            String t = (comparator == EQUALS || comparator == CONTAINS) ?
                       TBConstants::TYPE_STRING : TBConstants::TYPE_DOUBLE;
            d1 = TBData::create(value, t);
            if(!value2.empty())
                d2 = TBData::create(value2, t);
        }
    } else {
        String type;
        for(unsigned int i = 0; i < table->getFields()->size(); i++) {
            if(field == table->field(i)->getName()) {
                type = table->field(i)->getType();
                break;
            }
        }
    
        if(comparator == EQUALS || comparator == LESSTHAN ||
           comparator == GREATERTHAN) {
            valid = TBConstants::valueIsValid(value, type);
        } else if(comparator == CONTAINS || comparator == CONTAINSLT ||
                  comparator == CONTAINSGT) {
            type = TBConstants::arrayType(type);
            valid = TBConstants::valueIsValid(value, type);
        } else if(comparator == BETWEEN) {
            value2 = qPrintable(valueEdit2->text());
            TBConstants::strtrim(value2);
            valid = TBConstants::valueIsValid(value, type) &&
                    TBConstants::valueIsValid(value2, type);
        } else if(comparator == CONTAINSBT) {
            type = TBConstants::arrayType(type);
            value2 = qPrintable(valueEdit2->text());
            TBConstants::strtrim(value2);
            valid = TBConstants::valueIsValid(value, type) &&
                    TBConstants::valueIsValid(value2, type);
        }
        
        if(valid) {
            d1 = TBData::create(value, type);
            if(!value2.empty()) d2 = TBData::create(value2, type);
        }
    }
        
    if(valid) {
        TBFilterRule* fr = new TBFilterRule(field, comparator, d1, d2,
                                            isNot, anyField);

        bool duplicate = false;
        for(unsigned int i = 0; i < rules->size() && !duplicate; i++) {
            TBFilterRule* r = rules->at(i);
            if(r != NULL && r->equals(fr)) duplicate = true;
        }

        if(duplicate) {
            String message = "This rule is a duplicate.  Not adding to list.";
            QMessageBox::warning(this, "Rule Error", message.c_str());
            return;
        }
        
        rules->addRule(fr);
        String str = (comparator == CONTAINS || comparator == CONTAINSBT ||
                      comparator == CONTAINSLT || comparator == CONTAINSGT)?
                        (isNot?"does not":"does"):(isNot?"is not":"is");
        str = field + " " + str + " " + comp + " " + value;
        if(comparator == BETWEEN || comparator == CONTAINSBT)
            str += " and " + value2;
        str += ".";
        listWidget->addItem(str.c_str());
        valueEdit->setText("");
        valueEdit2->setText("");
    } else {
        String message = "Value is not valid for field " + field + ".";
        QMessageBox::critical(this, "Rule Error", message.c_str());
    }
}

void TBFilterRules::removeRule() {
    if(listWidget->count() < 1 || listWidget->currentRow() < 0)
        return;

    int i = listWidget->currentRow();
    QListWidgetItem* item = listWidget->takeItem(i);
    delete item;
    rules->removeRule(i);

    if(listWidget->count() == 0) addButton->click();
}

void TBFilterRules::listIndexChanged(int i) {
    if(i == -1) return;
    
    TBFilterRule* r = rules->at(i);
    if(r == NULL) return;

    notChooser->setCurrentIndex(r->getIsNot()?1:0);
    
    String f = r->getField();
    for(int j = 0; j < fieldChooser->count(); j++) {
        if(fieldChooser->itemText(j) == f.c_str()) {
            fieldChooser->setCurrentIndex(j);
            break;
        }
    }
    
    comparatorChooser->clear();
    comparatorChooser->addItem(TBConstants::compToString(
            r->getComparator()).c_str());

    if(r->getValue())
       valueEdit->setText(r->getValue()->asString().c_str());
    if(r->getValue2())
       valueEdit2->setText(r->getValue2()->asString().c_str());

    notChooser->setEnabled(false);
    fieldChooser->setEnabled(false);
    comparatorChooser->setEnabled(false);
    valueEdit->setEnabled(false);
    andLabel->setEnabled(false);
    valueEdit2->setEnabled(false);
    addButton->setText("Clear");
}

}
