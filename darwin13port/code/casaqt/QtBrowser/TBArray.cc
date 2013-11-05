//# TBArray.cc: Holds a potentially multi-dimensional array.
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
#include <casaqt/QtBrowser/TBArray.h>
#include <casaqt/QtBrowser/TBTable.h>
#include <casaqt/QtBrowser/TBField.h>
#include <casaqt/QtBrowser/TBKeyword.h>

namespace casa {

/////////////////////////
// TBARRAY DEFINITIONS //
/////////////////////////

// Constructors/Destructors //

TBArray::TBArray(int x, int y, String tp, String tb) : dimensions(),
                 data(), valid(false), type(tp), row(x), col(y), oneDim(false),
                 isData(true), isColKeyword(false) {
    parseArray(&tb);
}

TBArray::TBArray(int k, String tp, String tb): dimensions(), data(),
                 valid(false), type(tp), row(k), col(-1), oneDim(false),
                 isData(false), isColKeyword(false) {
    parseArray(&tb);
}

TBArray::TBArray(String col, int index, String tp, String tb): dimensions(),
                 data(), valid(false), type(tp), row(index), col(-1),
                 oneDim(false), isData(false), isColKeyword(true), field(col) {
    parseArray(&tb);
}

TBArray::TBArray(String tp, String tb) : dimensions(), data(), valid(false),
                 type(tp), row(-1), col(-1), oneDim(false), isData(true),
                 isColKeyword(false) {
    parseArray(&tb);
}

TBArray::~TBArray() {
    int n = dimensions.size();
    for(unsigned int i = 0; i < data.size(); i++) {
        deleteData((vector<void*>*)(data.at(i)), n - 1);
        delete (vector<void*>*)(data.at(i));
    }
}

// Accessor Methods //

bool TBArray::isValid() { return valid; }

vector<int> TBArray::getDimensions() { return vector<int>(dimensions); }

unsigned int TBArray::dim() { return dimensions.size(); }

int TBArray::dimensionAt(unsigned int i) {
    if(i < dimensions.size()) return dimensions.at(i);
    else return -1;
}

bool TBArray::isOneDimensional() { return oneDim; }

String TBArray::getType() { return type; }

vector<void*>* TBArray::getData() { return &data; }

int TBArray::getRow() { return row; }

int TBArray::getCol() { return col; }

// Public Methods //

bool TBArray::sameLocationAs(TBArray* array) {
    if(isData) {
        return row == array->row && col == array->col;
    } else {
        if(isColKeyword) {
            return field == array->field && row == array->row;
        } else {
            return row == array->row;
        }
    }
}

String TBArray::getName(TBTable* table) {
    stringstream ss;
    ss << table->getName();

    if(isData) {
        ss << '[' << row << ", " << col << ']';
    } else if(isColKeyword) {
        ss << " [field " << field << ", keyword ";
        ss << table->field(field)->keyword(row) << "]";
    } else {
        ss << " [keyword " << table->keyword(row)->getName() << "]";
    }

    return ss.str();
}

String TBArray::dataAt(vector<int> d) {
    if(oneDim && d.size() == 1)
        d.insert(d.begin(), 0);
        
    if(!dimensionIsValid(d)) return "";

    // go to the "last" dimension
    vector<void*>* row = &data;
    for(unsigned int i = 0; i < d.size() - 1; i++) {
        row = (vector<void*>*)row->at(d.at(i));
    }
    String s = *(String*)(row->at(d.at(d.size() - 1)));
    return s;
}

void TBArray::setDataAt(vector<int> d, String newVal) {
    if(!valid || !dimensionIsValid(d)) return;
    
    // go to the "last" dimension
    vector<void*>* row = &data;
    for(unsigned int i = 0; i < d.size() - 1; i++) {
        row = (vector<void*>*)(row->at(d.at(i)));
    }
    
    delete (String*)(*row)[d.at(d.size() - 1)];
    (*row)[d.at(d.size() - 1)] = new String(newVal);
}

bool TBArray::dimensionIsValid(vector<int> d) {
    if(d.size() != dimensions.size()) return false;
    for(unsigned int i = 0; i < d.size(); i++) {
        if(d.at(i) < 0 || d.at(i) >= dimensions.at(i)) return false;
    }
    return true;
}

String TBArray::toFlattenedString() {
    String str;
    int d = dimensions.size() - 1;
    
    for(unsigned int i = 0; i < data.size(); i++) {
        str += toFlattenedString((vector<void*>*)data.at(i), d);
    }
    return str;
}

bool TBArray::contains(String v) {
    int d = dimensions.size() - 1;
    for(unsigned int i = 0; i < data.size(); i++)
        if(contains((vector<void*>*)data.at(i), d, v))
            return true;

    return false;
}

bool TBArray::containsBetween(String v1, String v2) {
    String t = TBConstants::arrayType(type);
    if(!TBConstants::typeIsNumberable(t)) return false;

    double dv1 = TBConstants::valueToDouble(v1, t);
    double dv2 = TBConstants::valueToDouble(v2, t);

    if(dv2 < dv1) {
        double temp = dv2;
        dv2 = dv1;
        dv1 = temp;
    }

    int d = dimensions.size() - 1;
    for(unsigned int i = 0; i < data.size(); i++)
        if(containsBetween((vector<void*>*)data.at(i), d, dv1, dv2))
            return true;

    return false;
}

bool TBArray::containsLessThan(String v) {
    String t = TBConstants::arrayType(type);
    if(!TBConstants::typeIsNumberable(t)) return false;

    double dv = TBConstants::valueToDouble(v, t);

    int d = dimensions.size() - 1;
    for(unsigned int i = 0; i < data.size(); i++)
        if(containsLessThan((vector<void*>*)data.at(i), d, dv))
            return true;

    return false;
}

bool TBArray::containsGreaterThan(String v) {
    String t = TBConstants::arrayType(type);
    if(!TBConstants::typeIsNumberable(t)) return false;

    double dv = TBConstants::valueToDouble(v, t);
    
    int d = dimensions.size() - 1;
    for(unsigned int i = 0; i < data.size(); i++)
        if(containsGreaterThan((vector<void*>*)data.at(i), d, dv))
            return true;

    return false;
}

// Private Methods //

void TBArray::parseArray(String* tbl) {
    unsigned int i = tbl->find('[');
    String str = TBConstants::ARRAY_AXES_LENGTHS;
    unsigned int j = TBConstants::findIgnoreCase(*tbl, str);
    unsigned int k = tbl->find(']', i);

    if(i < tbl->length()) {
        if(j < tbl->length() && j < i) { // lengths are specified
            // parse dimensions
            String dStr = tbl->substr(i + 1, (k - i - 1));

            i = 0;
            j = dStr.find(',');
            while(i < dStr.length()) {
                int x;
                if(j < dStr.length()) {
                    String str = dStr.substr(i, j - i);
                    TBConstants::strtrim(str);
                    TBConstants::atoi(str, &x);
                    i = j + 1;
                    j = dStr.find(',', i);
                } else {
                    String str = dStr.substr(i);
                    TBConstants::strtrim(str);
                    TBConstants::atoi(str, &x);
                    i = dStr.size();
                }
                dimensions.push_back(x);
            }

            if(dimensions.size() == 1) {
                oneDim = true;
                int x = dimensions.at(0);
                dimensions.pop_back();
                dimensions.push_back(1);
                dimensions.push_back(x);
            }

            i = tbl->find('[', k);
            j = tbl->find(']', i);
            
            if(i < tbl->length()) {
                valid = false;
                return;
            }

            if(dimensions.size() > 2) {
                String str = tbl->substr(i);
                TBConstants::strtrim(str);
                valid = true;
                parseMultidimensionalTable(str);
            }
            
            else if(i < tbl->length() && j <tbl->length()) {
                String tStr = tbl->substr(i + 1, (j - i - 1));
                vector<int> d(dimensions);
                d.erase(d.begin());
                
                data.resize(dimensions.at(0));
                for(int z = 0; z < dimensions.at(0); z++)
                    tStr = parseRow(tStr, &data, d, z);

                valid = true;
            }
            
        } else { // it's just a vector
            oneDim = true;
            dimensions.push_back(1);
            int x = 0;
            String tStr = tbl->substr(i + 1, (k - i - 1));
            vector<void*>* row = new vector<void*>();
            
            i = 0;
            j = tStr.find(',');
            while(i < tStr.length()) {
                String* str;
                String tmp;
                if(j < tStr.length()) {
                    tmp = tStr.substr(i, j - i);
                    TBConstants::strtrim(tmp);
                    str = new String(tmp);
                    i = j + 1;
                    j = tStr.find(',', i);
                } else {
                    tmp = tStr.substr(i);
                    TBConstants::strtrim(tmp);
                    str = new String(tmp);
                    i = tStr.length();
                }
                x++;
                row->push_back(str);
            }

            dimensions.push_back(x);
            data.push_back(row);
            valid = true;
        }
    }
}

String TBArray::parseRow(String& str, vector<void*>* r, vector<int> d, int x) {
    if(d.size() == 1) { // on "last" dimension
        vector<void*>* row = new vector<void*>(d.at(0));

        unsigned int i = TBConstants::findWS(str);
        for(int k = 0; k < d.at(0); k++) {
            String* s;
            if(i < str.length()) {
                String tmp = str.substr(0, i);
                TBConstants::strtrim(tmp);
                s = new String(tmp);
                str = str.substr(i + 1);
            } else {
                TBConstants::strtrim(str);
                s = new String(str);
                str = "";
            }

            if(s->length() > 0 && s->at(s->length() - 1) == ',')
                s->erase(s->length() - 1);
            (*row)[k] = s;
            i = TBConstants::findWS(str);
        }
        (*r)[x] = row;
        TBConstants::strtrim(str);
        return str;
    } else {
        int s = d.at(0);
        vector<void*>* row = new vector<void*>(s);

        vector<int> d2;
        for(unsigned int i = 1; i < d.size(); i++)
            d2.push_back(d.at(i));

        for(int i = 0; i < s; i++)
            str = parseRow(str, row, d2, i);

        (*r)[x] = row;
        return str;
    }
}

void TBArray::parseMultidimensionalTable(String str) {
    // first insert placeholders
    vector<int> d(dimensions);
    d.erase(d.begin());

    data.resize(dimensions.at(0));
    for(int i = 0; i < dimensions.at(0); i++)
        insertPlaceholders(&data, d, i);

    unsigned int i = 0, j = str.find('\n');
    while(i < str.length()) {
        String cell;
        if(j < str.length()) {
            cell = str.substr(i, j - i);
            i = j + 1;
            j = str.find('\n', i);
        } else {
            cell = str.substr(i);
            i = str.length();
        }

        unsigned int x = cell.find('[');
        unsigned int y = cell.find(']', x + 1);
        String coord = cell.substr(x + 1, y - x - 1);

        d = vector<int>();
        unsigned int a = 0, b = TBConstants::findWS(coord);
        while(a < coord.length()) {
            int temp;
            if(b < coord.length()) {
                String str = coord.substr(a, b);
                TBConstants::atoi(str, &temp);
                d.push_back(temp);
                a = b + 1;
                b = TBConstants::findWS(coord, a);
            } else {
                String str = coord.substr(a);
                TBConstants::atoi(str, &temp);
                d.push_back(temp);
                a = coord.length();
            }
        }
        
        x = cell.find('[', y);
        y = cell.find(']', x + 1);
        String data = cell.substr(x + 1, y - x - 1);

        // usually data is in form [coordinates][data], but sometimes it is
        // [coordinates][data1, data2], for example [0, 0, 0][0, 1], where
        // 0 is the value for [0, 0, 0] and 1 is the value for [1, 0, 0].
        // Stop-gap measure, this needs to be addressed better.
        
        x = 0;
        y = data.find(',');
        
        if(y < data.length()) {
            int index = 0;
            String cdata;

            while(y < data.length()) {
                cdata = data.substr(x, y);
                TBConstants::strtrim(cdata);
                
                d[0] = index;
                setDataAt(d, cdata);
                
                index++;
                x = y + 1;
                y = data.find(x, ',');
            }
            
            // last
            cdata = data.substr(x);
            cdata.erase(cdata.size());
            TBConstants::strtrim(cdata);
            d[0] = index;
            setDataAt(d, cdata);
        } else setDataAt(d, data);
    }
}

void TBArray::insertPlaceholders(vector<void*>* r, vector<int> d, int x) {
    if(d.size() == 1) {
        vector<void*>* row = new vector<void*>(d.at(0));

        (*r)[x] = row;
    } else {
        int s = d.at(0);
        vector<void*>* row = new vector<void*>(s);

        vector<int> d2;
        for(unsigned int i = 1; i < d.size(); i++)
            d2.push_back(d.at(i));

        for(int i = 0; i < s; i++)
            insertPlaceholders(row, d2, i);

        (*r)[x] = row;
    }
}

String TBArray::toFlattenedString(vector<void*>* row, int d) {
    String str;
    if(d == 1) { // "last" dimension
        for(unsigned int i = 0; i < row->size(); i++) {
            str += *((String*)row->at(i)) + " ";
        }
    } else {
        d--;
        for(unsigned int i = 0; i < row->size(); i++) {
            str += toFlattenedString((vector<void*>*)row->at(i), d);
        }
    }
    return str;
}

bool TBArray::contains(vector<void*>* data, int n, String v) {
    if(n == 1) { // "last" dimension
        for(unsigned int i = 0; i < data->size(); i++) {
            String s = *(String*)data->at(i);
            if(s == v) return true;
        }
        return false;
    } else {
        for(unsigned int i = 0; i < data->size(); i++) {
            if(contains((vector<void*>*)data->at(i), n - 1, v)) return true;
        }
        return false;
    }
}

bool TBArray::containsBetween(vector<void*>* data,int n,double v1, double v2) {
    if(n == 1) { // "last" dimension
        String t = TBConstants::arrayType(type);
        for(unsigned int i = 0; i < data->size(); i++) {
            String s = *(String*)data->at(i);

            double d = TBConstants::valueToDouble(s, t);
            
            if(d >= v1 && d <= v2) return true;
        }
        return false;
    } else {
        for(unsigned int i = 0; i < data->size(); i++) {
            if(containsBetween((vector<void*>*)data->at(i), n - 1, v1, v2))
                return true;
        }
        return false;
    }
}

bool TBArray::containsLessThan(vector<void*>* data, int n, double v) {
    if(n == 1) { // "last" dimension
        String t = TBConstants::arrayType(type);
        for(unsigned int i = 0; i < data->size(); i++) {
            String s = *(String*)data->at(i);

            double d = TBConstants::valueToDouble(s, t);
            
            if(d < v) return true;
        }
        return false;
    } else {
        for(unsigned int i = 0; i < data->size(); i++) {
            if(containsLessThan((vector<void*>*)data->at(i), n - 1, v))
                return true;
        }
        return false;
    }
}

bool TBArray::containsGreaterThan(vector<void*>* data, int n, double v) {
    if(n == 1) { // "last" dimension
        String t = TBConstants::arrayType(type);
        for(unsigned int i = 0; i < data->size(); i++) {
            String s = *(String*)data->at(i);

            double d = TBConstants::valueToDouble(s, t);
            
            if(d > v) return true;
        }
        return false;
    } else {
        for(unsigned int i = 0; i < data->size(); i++) {
            if(containsGreaterThan((vector<void*>*)data->at(i), n - 1, v))
                return true;
        }
        return false;
    }
}

void TBArray::deleteData(vector<void*>* data, int n) {
    if(n == 1) {
        for(unsigned int i = 0; i < data->size(); i++)
            delete (String*)data->at(i);
    } else {
        for(unsigned int i = 0; i < data->size(); i++) {
            deleteData((vector<void*>*)(data->at(i)), n - 1);
            delete (vector<void*>*)(data->at(i));
        }
    }
}

}
