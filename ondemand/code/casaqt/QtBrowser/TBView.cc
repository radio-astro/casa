//# TBView.cc: Current "view" or state of the browser that can be serialized.
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
#include <casaqt/QtBrowser/TBView.h>
#include <casaqt/QtBrowser/TBConstants.h>
#include <casaqt/QtBrowser/TBFilterRules.qo.h>
#include <casaqt/QtBrowser/TBFormat.qo.h>
#include <casaqt/QtBrowser/TBData.h>
#include <casaqt/QtUtilities/QtFileDialog.qo.h>

#include <fstream>
#include <sys/stat.h>

#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMLSSerializer.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include <sstream>

namespace casa {

/////////////////////////////
// TBTABLEVIEW DEFINITIONS //
////////////////////////////

// Constructors/Destructors //

TBTableView::TBTableView() : selected(false), filter(NULL), taql(false) { }

TBTableView::~TBTableView() { }

////////////////////////
// TBVIEW DEFINITIONS //
////////////////////////

// Constructors/Destructors //

TBView::TBView() : histLimit(QtFileDialog::historyLimit()) { }

TBView::~TBView() {
    for(unsigned int i = 0; i < views.size(); i++)
        if(views.at(i) != NULL) delete views.at(i);
}

// Accessors/Mutators //

void TBView::addTableView(TBTableView* view) {
    if(view == NULL) return;
    views.push_back(view);
}

vector<TBTableView*>* TBView::getTableViews() {
    return &views;
}

void TBView::setLastOpenedDirectory(String str) {
    lastOpenedDir = str;
}

String TBView::getLastOpenedDirectory() {
    return lastOpenedDir;
}

int TBView::chooserHistoryLimit() const { return histLimit; }
void TBView::setChooserHistoryLimit(int limit) { histLimit = limit; }

// Public Methods //

bool TBView::saveToFile(String file) {
    DOMImplementation* impl = DOMImplementation::getImplementation();
    if(impl != NULL) {
        try {
            DOMDocument* doc = impl->createDocument(0, TBConstants::xstr(
                               TBConstants::VIEW_DOCUMENT), 0);
            DOMElement* rootElem = doc->getDocumentElement();
            
            DOMElement* viewElem = doc->createElement(TBConstants::xstr(
                                   TBConstants::VIEW_VIEW));
            if(!lastOpenedDir.empty())
                viewElem->setAttribute(TBConstants::xstr(
                                       TBConstants::VIEW_LASTDIR),
                                       TBConstants::xstr(lastOpenedDir));
            viewElem->setAttribute(TBConstants::xstr(
                                   TBConstants::VIEW_HISTLIMIT),
                                   TBConstants::xstr(
                                   TBConstants::itoa(histLimit)));
            rootElem->appendChild(viewElem);
            
            for(unsigned int i = 0; i < views.size(); i++) {
                TBTableView* view = views.at(i);
                DOMElement* tableElem = doc->createElement(TBConstants::xstr(
                                        TBConstants::VIEW_TABLE));
                tableElem->setAttribute(TBConstants::xstr(
                                        TBConstants::VIEW_LOCATION),
                                        TBConstants::xstr(view->location));
                tableElem->setAttribute(TBConstants::xstr(
                                        TBConstants::VIEW_SELECTED),
                                    TBConstants::xstr(view->selected?"1":"0"));
                tableElem->setAttribute(TBConstants::xstr(
                                        TBConstants::VIEW_TAQL),
                                        TBConstants::xstr(view->taql?"1":"0"));
                viewElem->appendChild(tableElem);

                DOMElement* e;
                DOMText* t;
                // hidden
                
                stringstream ss;
                if(view->hidden.size() > 0) {
                    e = doc->createElement(TBConstants::xstr(
                            TBConstants::VIEW_HIDDEN));
                    e->setAttribute(TBConstants::xstr(
                            TBConstants::VIEW_HIDDEN_LENGTH),
                            TBConstants::xstr(TBConstants::itoa(
                                    view->hidden.size())));
                    tableElem->appendChild(e);

                    for(unsigned int j = 0; j < view->hidden.size(); j++) {
                        ss << view->hidden.at(j);
                        if(j < view->hidden.size() - 1) ss << ' ';
                    }
                    t = doc->createTextNode(TBConstants::xstr(ss.str()));
                    e->appendChild(t);
                }
                
                // visible indices
                if(view->visInd.size() > 0) {
                    e = doc->createElement(TBConstants::xstr(
                                       TBConstants::VIEW_VISIND));
                    e->setAttribute(TBConstants::xstr(
                            TBConstants::VIEW_HIDDEN_LENGTH),
                            TBConstants::xstr(TBConstants::itoa(
                                    view->visInd.size())));
                    tableElem->appendChild(e);
                
                    ss.str("");
                    for(unsigned int j = 0; j < view->visInd.size(); j++) {
                        ss << view->visInd.at(j);
                        if(j < view->visInd.size() - 1) ss << ' ';
                    }
                    t = doc->createTextNode(TBConstants::xstr(ss.str()));
                    e->appendChild(t);
                }

                // rows
                e = doc->createElement(TBConstants::xstr(
                                        TBConstants::VIEW_ROWS));
                e->setAttribute(TBConstants::xstr(TBConstants::VIEW_ROWS_FROM),
                                TBConstants::xstr(TBConstants::itoa(
                                                  view->loadedFrom)));
                e->setAttribute(TBConstants::xstr(TBConstants::VIEW_ROWS_NUM),
                                TBConstants::xstr(TBConstants::itoa(
                                                  view->loadedNum)));
                tableElem->appendChild(e);

                // filter
                if(view->filter != NULL) {
                    e = doc->createElement(TBConstants::xstr(
                                           TBConstants::VIEW_FILTER));
                    tableElem->appendChild(e);

                    DOMElement* el = NULL;
                    for(unsigned int j = 0; j < view->filter->size(); j++) {
                        TBFilterRule* rule = view->filter->at(j);
                        el = doc->createElement(TBConstants::xstr(
                                               TBConstants::VIEW_FILTER_RULE));
                        el->setAttribute(TBConstants::xstr(
                                         TBConstants::VIEW_FILTER_RULE_FIELD),
                                         TBConstants::xstr(rule->getField()));
                        el->setAttribute(TBConstants::xstr(
                                   TBConstants::VIEW_FILTER_RULE_COMPARATOR),
                                   TBConstants::xstr(TBConstants::compToString(
                                              rule->getComparator())));
                        el->setAttribute(TBConstants::xstr(
                                         TBConstants::VIEW_FILTER_RULE_VALUE),
                                         TBConstants::xstr(rule->getValue()->
                                                          asString().c_str()));
                        if(rule->getValue2() != NULL) {
                        el->setAttribute(TBConstants::xstr(
                                         TBConstants::VIEW_FILTER_RULE_VALUE2),
                                         TBConstants::xstr(rule->getValue2()->
                                                          asString().c_str()));
                        }
                        el->setAttribute(TBConstants::xstr(
                                         TBConstants::VIEW_FILTER_RULE_NOT),
                                         TBConstants::xstr(rule->getIsNot()?
                                                           "1":"0"));
                        el->setAttribute(TBConstants::xstr(
                                         TBConstants::VIEW_FILTER_RULE_ANY),
                                         TBConstants::xstr(rule->getAnyField()?
                                                           "1":"0"));
                        el->setAttribute(TBConstants::xstr(
                                         TBConstants::VIEW_FILTER_RULE_TYPE),
                                         TBConstants::xstr(
                                                 rule->getValue()->getType()));
                        
                        e->appendChild(el);
                    }
                }

                // formats
                bool format = false;
                if(view->formats.size() > 0) {
                    for(unsigned int j = 0; j < view->formats.size(); j++)
                        if(view->formats.at(j) != NULL) {
                            format = true;
                            break;
                        }
                }

                if(format) {
                    e = doc->createElement(TBConstants::xstr(
                                           TBConstants::VIEW_FORMATS));
                    
                    e->setAttribute(TBConstants::xstr(
                                    TBConstants::VIEW_HIDDEN_LENGTH),
                                    TBConstants::xstr(TBConstants::itoa(
                                                    view->formats.size())));
                    tableElem->appendChild(e);
                    DOMElement* el = NULL, *elem = NULL;
                    for(unsigned int j = 0; j < view->formats.size(); j++) {
                        if(view->formats.at(j) != NULL) {
                            TBFormat* form = view->formats.at(j);
                            el = doc->createElement(TBConstants::xstr(
                                                    TBConstants::VIEW_FORMAT));
                            el->setAttribute(TBConstants::xstr(
                                             TBConstants::VIEW_FORMAT_COL),
                                             TBConstants::xstr(
                                                     TBConstants::itoa(j)));
                            el->setAttribute(TBConstants::xstr(
                                            TBConstants::VIEW_FORMAT_DECIMALS),
                                             TBConstants::xstr(
                                             TBConstants::itoa(
                                             form->getDecimalPlaces())));
                            el->setAttribute(TBConstants::xstr(
                                             TBConstants::VIEW_FORMAT_SFORMAT),
                                             TBConstants::xstr(
                                         form->getScientificFormat()?"1":"0"));
                            el->setAttribute(TBConstants::xstr(
                                             TBConstants::VIEW_FORMAT_BFORMAT),
                                             TBConstants::xstr(
                                             TBConstants::bFormToString(
                                                     form->getBoolFormat())));
                            el->setAttribute(TBConstants::xstr(
                                             TBConstants::VIEW_FORMAT_DFORMAT),
                                             TBConstants::xstr(
                                                     form->getDateFormat()));
                            el->setAttribute(TBConstants::xstr(
                                          TBConstants::VIEW_FORMAT_VTHRESHOLD),
                                          TBConstants::xstr(TBConstants::itoa(
                                                form->getVectorThreshold())));

                            QFontColor* font = NULL;
                            if(form->getAllFont() != NULL) {
                                font = form->getAllFont();
                                elem = doc->createElement(TBConstants::xstr(
                                        TBConstants::VIEW_FORMAT_ALLFONT));
                                fontElem(elem, font);
                                el->appendChild(elem);
                            }

                            for(unsigned int k = 0; k<form->getFonts()->size();
                                k++) {
                                font = form->getFonts()->at(k);
                                if(font != NULL) {
                                    elem =doc->createElement(TBConstants::xstr(
                                            TBConstants::VIEW_FORMAT_FONT));
                                    fontElem(elem, font);
                                    el->appendChild(elem);
                                }
                            }
                                             
                            e->appendChild(el);
                        }
                    }
                }

                // sorting
                if(view->sort.size() > 0) {
                    e = doc->createElement(TBConstants::xstr(
                                           TBConstants::VIEW_SORT));
                    e->setAttribute(TBConstants::xstr(
                                    TBConstants::VIEW_HIDDEN_LENGTH),
                                    TBConstants::xstr(TBConstants::itoa(
                                            view->sort.size())));
                    tableElem->appendChild(e);
                    DOMElement* el = NULL;
                    for(unsigned int j = 0; j < view->sort.size(); j++) {
                        String field = view->sort.at(j).first;
                        bool asc = view->sort.at(j).second;
                        el = doc->createElement(TBConstants::xstr(
                                TBConstants::VIEW_FILTER_RULE_FIELD));
                        el->setAttribute(TBConstants::xstr(
                             TBConstants::VIEW_NAME),TBConstants::xstr(field));
                        el->setAttribute(TBConstants::xstr(
                                TBConstants::VIEW_SORT_ASCENDING),
                                TBConstants::xstr(asc?"1":"0"));
                        e->appendChild(el);
                    }
                }
            }

             // output to file
            DOMLSSerializer* writer = impl->createLSSerializer();
	    DOMLSOutput *output = impl->createLSOutput();
            LocalFileFormatTarget fileTarget(file.c_str());
	    output->setByteStream(&fileTarget);
            //writer->setEncoding(TBConstants::xstr("UTF-8"));
            //writer->writeNode(&fileTarget, *doc);
	    writer->write(doc, output);
	    output->release();
            writer->release();
            doc->release();
            return true;
        } catch(...) {
            return false;
        }
    }
    return false;
}

bool TBView::saveToDefaultFile() {
    return saveToFile(defaultFile());
}

// Public Static Methods //

TBView* TBView::loadFromFile(String file) {
    try {
        // make sure file exists
        fstream fin;
        fin.open(file.c_str(), ios::in);
        bool open = fin.is_open();
        fin.close();
        if(!open) return NULL;
    
        XercesDOMParser* parser = new XercesDOMParser();
        parser->parse(TBConstants::xstr(file));

        DOMDocument* doc = parser->getDocument();
        DOMElement* docElem = doc->getDocumentElement();

        if(TBConstants::equalsIgnoreCase(TBConstants::xstr(
                        docElem->getTagName()), TBConstants::VIEW_DOCUMENT)) {
            DOMNodeList* nodes = docElem->getElementsByTagName(
                    TBConstants::xstr(TBConstants::VIEW_VIEW));
            if(nodes != NULL && nodes->getLength() > 0) {
                // should only be one <view>
                DOMElement* e = (DOMElement*)nodes->item(0);
                TBView* view = new TBView();

                String lastdir = TBConstants::xstr(e->getAttribute(
                        TBConstants::xstr(TBConstants::VIEW_LASTDIR)));
                if(!lastdir.empty()) view->lastOpenedDir = lastdir;
                
                String histstr = TBConstants::xstr(e->getAttribute(
                        TBConstants::xstr(TBConstants::VIEW_HISTLIMIT)));
                if(!histstr.empty()) {
                    int historylimit;
                    if(TBConstants::atoi(histstr, &historylimit) >= 1)
                        view->histLimit = historylimit;
                }
                
                // get table views
                nodes = e->getElementsByTagName(TBConstants::xstr(
                        TBConstants::VIEW_TABLE));
                if(nodes == 0) return NULL;

                DOMNodeList* nl = NULL;
                DOMElement* el = NULL;
                for(unsigned int i = 0; i < nodes->getLength(); i++) {
                    e = (DOMElement*)nodes->item(i);

                    // table location
                    String location = TBConstants::xstr(e->getAttribute(
                            TBConstants::xstr(TBConstants::VIEW_LOCATION)));
                    if(location.empty()) continue;
                    
                    // table taql
                    String selStr = TBConstants::xstr(e->getAttribute(
                            TBConstants::xstr(TBConstants::VIEW_TAQL)));
                    bool taql = selStr == "1";
                    
                    if(!taql) {
                        // make sure file exists
                        struct stat st;
                        if(stat(location.c_str(), &st) == -1) continue;
                    }
                    
                    TBTableView* tv = new TBTableView();
                    tv->location = location;

                    // table selected
                    selStr = TBConstants::xstr(e->getAttribute(
                            TBConstants::xstr(TBConstants::VIEW_SELECTED)));
                    tv->selected = selStr == "1";
                    
                    tv->taql = taql;

                    // table hidden
                    nl = e->getElementsByTagName(TBConstants::xstr(
                            TBConstants::VIEW_HIDDEN));
                    if(nl != NULL && nl->getLength() > 0) {
                        el = (DOMElement*)nl->item(0);
                        // hidden length
                        int n;
                        String text = TBConstants::xstr(el->getAttribute(
                                TBConstants::xstr(
                                        TBConstants::VIEW_HIDDEN_LENGTH)));
                        if(TBConstants::atoi(text, &n) >= 1) {
                            // hidden text
                            String text = TBConstants::xstr(
                                    el->getTextContent());
                            if(n > 0 && (int)text.length() >= (n * 2 - 1)) {
                                stringstream ss(text);
                                int b;
                                for(int j = 0; j < n; j++) {
                                    ss >> b;
                                    if(b == 0) tv->hidden.push_back(false);
                                    else tv->hidden.push_back(true);
                                }
                            }
                        }
                    }
                    
                    // visual indices
                    nl = e->getElementsByTagName(TBConstants::xstr(
                            TBConstants::VIEW_VISIND));
                    if(nl != NULL && nl->getLength() > 0) {
                        el = (DOMElement*)nl->item(0);
                        // length
                        int n;
                        String text = TBConstants::xstr(el->getAttribute(
                                TBConstants::xstr(
                                        TBConstants::VIEW_HIDDEN_LENGTH)));
                        if(TBConstants::atoi(text, &n) >= 1) {
                            // visind text
                            String text = TBConstants::xstr(
                                    el->getTextContent());
                            if(n > 0 && (int)text.length() >= (n * 2 - 1)) {
                                stringstream ss(text);
                                int m;
                                for(int j = 0; j < n; j++) {
                                    ss >> m;
                                    tv->visInd.push_back(m);
                                }
                            }
                        }
                    }
                    
                    // table rows
                    nl = e->getElementsByTagName(TBConstants::xstr(
                            TBConstants::VIEW_ROWS));
                    if(nl != NULL && nl->getLength() > 0) {
                        el = (DOMElement*)nl->item(0);
                        String text = TBConstants::xstr(el->getAttribute(
                             TBConstants::xstr(TBConstants::VIEW_ROWS_FROM)));
                        if(!text.empty()) {
                            int n;
                            if(TBConstants::atoi(text, &n) >= 1) {
                                tv->loadedFrom = n;
                            }
                        }
                        text = TBConstants::xstr(el->getAttribute(
                               TBConstants::xstr(TBConstants::VIEW_ROWS_NUM)));
                        if(!text.empty()) {
                            int n;
                            if(TBConstants::atoi(text, &n) >= 1) {
                                tv->loadedNum = n;
                            }
                        }
                    }

                    // table filter
                    nl = e->getElementsByTagName(TBConstants::xstr(
                            TBConstants::VIEW_FILTER));
                    if(nl != NULL && nl->getLength() > 0) {
                        el = (DOMElement*)nl->item(0);
                        nl = el->getElementsByTagName(TBConstants::xstr(
                                TBConstants::VIEW_FILTER_RULE));
                        if(nl != NULL && nl->getLength() > 0) {
                            tv->filter = new TBFilterRuleSequence();
                        }
                        for(unsigned int j = 0; j < nl->getLength(); j++) {
                            el = (DOMElement*)nl->item(j);

                            String field = TBConstants::xstr(el->getAttribute(
                                    TBConstants::xstr(
                                        TBConstants::VIEW_FILTER_RULE_FIELD)));
                            if(field.empty()) continue;

                            String compStr=TBConstants::xstr(el->getAttribute(
                                   TBConstants::xstr(
                                   TBConstants::VIEW_FILTER_RULE_COMPARATOR)));
                            if(compStr.empty()) continue;

                            String val = TBConstants::xstr(el->getAttribute(
                                    TBConstants::xstr(
                                    TBConstants::VIEW_FILTER_RULE_VALUE)));
                            if(val.empty()) continue;
                            String type = TBConstants::xstr(el->getAttribute(
                                          TBConstants::xstr(
                                         TBConstants::VIEW_FILTER_RULE_TYPE)));
                            if(type.empty()) continue;

                            String val2 = TBConstants::xstr(el->getAttribute(
                                    TBConstants::xstr(
                                    TBConstants::VIEW_FILTER_RULE_VALUE2)));
                            String notStr = TBConstants::xstr(
                                    el->getAttribute(TBConstants::xstr(
                                    TBConstants::VIEW_FILTER_RULE_NOT)));
                            String anyStr = TBConstants::xstr(el->getAttribute(
                                    TBConstants::xstr(
                                    TBConstants::VIEW_FILTER_RULE_ANY)));
                            

                            Comparator comp=TBConstants::stringToComp(compStr);
                            bool notBool = notStr == "1";
                            bool any = anyStr == "1";

                            TBData* d1 = NULL;
                            TBData* d2 = NULL;
                            
                            bool valid = true;
                            if(any) {
                                if(comp != EQUALS && comp != CONTAINS) {
                                    String dt = TBConstants::TYPE_DATE;
                                    double d;
                                    
                                    if(TBConstants::atod(val, &d) != 1 &&
                                       !TBConstants::valueIsValid(val, dt))
                                        valid = false;
                                    
                                    if(valid && (comp == BETWEEN ||
                                       comp == CONTAINSBT)) {
                                        if(TBConstants::atod(val2, &d) != 1 &&
                                           !TBConstants::valueIsValid(val2,dt))
                                            valid = false;
                                    }
                                }
                                
                                if(valid) {
                                    String t = (comp == EQUALS ||
                                                comp == CONTAINS) ?
                                                TBConstants::TYPE_STRING :
                                                TBConstants::TYPE_DOUBLE;
                                    d1 = TBData::create(val, t);
                                    if(!val2.empty())
                                        d2 = TBData::create(val2, t);
                                }
                            } else {                            
                                if(comp == EQUALS || comp == LESSTHAN ||
                                   comp == GREATERTHAN) {
                                    valid=TBConstants::valueIsValid(val, type);
                                } else if(comp == CONTAINS ||
                                          comp == CONTAINSLT ||
                                          comp == CONTAINSGT) {
                                    type = TBConstants::arrayType(type);
                                    valid=TBConstants::valueIsValid(val, type);
                                } else if(comp == BETWEEN) {
                                    valid = TBConstants::valueIsValid(val,
                                            type) && TBConstants::valueIsValid(
                                                    val2, type);
                                } else if(comp == CONTAINSBT) {
                                    type = TBConstants::arrayType(type);
                                    valid = TBConstants::valueIsValid(val,
                                            type) && TBConstants::valueIsValid(
                                                    val2, type);
                                }
                                
                                if(valid) {
                                    d1 = TBData::create(val, type);
                                    if(!val2.empty())
                                        d2 = TBData::create(val2, type);
                                }
                            }
                                
                            if(valid) {                            
                                TBFilterRule* rule = new TBFilterRule(field,
                                                   comp, d1, d2, notBool, any);
                                tv->filter->addRule(rule);
                            }
                        }
                    }

                    // table formats
                    nl = e->getElementsByTagName(TBConstants::xstr(
                            TBConstants::VIEW_FORMATS));
                    if(nl != NULL && nl->getLength() > 0) {
                        el = (DOMElement*)nl->item(0);
                        int n;
                        String str = TBConstants::xstr(el->getAttribute(
                                TBConstants::xstr(
                                        TBConstants::VIEW_HIDDEN_LENGTH)));
                        if(TBConstants::atoi(str, &n) >= 1 && n > 0) {
                            tv->formats.resize(n);

                            for(int i = 0; i < n; i++) tv->formats[i] = NULL;

                            nl = el->getElementsByTagName(TBConstants::xstr(
                                    TBConstants::VIEW_FORMAT));
                            DOMNodeList* nl2 = NULL;
                            DOMElement* elem = NULL;
                            if(nl != NULL && nl->getLength() > 0) {
                                for(unsigned int j=0; j<nl->getLength(); j++) {
                                    el = (DOMElement*)nl->item(j);

                                    str = TBConstants::xstr(el->getAttribute(
                                            TBConstants::xstr(
                                               TBConstants::VIEW_FORMAT_COL)));
                                    if(TBConstants::atoi(str, &n)!=1 || n<0 ||
                                       n > (int)tv->formats.size()) continue;

                                    TBFormat* f = new TBFormat();
                                    tv->formats[n] = f;

                                    str = TBConstants::xstr(el->getAttribute(
                                            TBConstants::xstr(
                                          TBConstants::VIEW_FORMAT_DECIMALS)));
                                    if(TBConstants::atoi(str, &n)>=1 && n>=0)
                                        f->setDecimalPlaces(n);

                                    str = TBConstants::xstr(el->getAttribute(
                                           TBConstants::xstr(
                                           TBConstants::VIEW_FORMAT_SFORMAT)));
                                    f->setScientificFormat(str == "1");

                                    str = TBConstants::xstr(el->getAttribute(
                                           TBConstants::xstr(
                                           TBConstants::VIEW_FORMAT_BFORMAT)));
                                    f->setBoolFormat(
                                            TBConstants::stringToBForm(str));

                                    str = TBConstants::xstr(el->getAttribute(
                                           TBConstants::xstr(
                                           TBConstants::VIEW_FORMAT_DFORMAT)));
                                    f->setDateFormat(str);

                                    str = TBConstants::xstr(el->getAttribute(
                                        TBConstants::xstr(
                                        TBConstants::VIEW_FORMAT_VTHRESHOLD)));
                                    if(TBConstants::atoi(str, &n)>=1 && n >= 0)
                                        f->setVectorThreshold(n);

                                    nl2 = el->getElementsByTagName(
                                            TBConstants::xstr(
                                            TBConstants::VIEW_FORMAT_ALLFONT));
                                    if(nl2 != NULL && nl2->getLength() > 0) {
                                        elem = (DOMElement*)nl2->item(0);
                                        QFontColor* fc = fontElem(elem);
                                        f->setAllFont(fc);
                                    }

                                    nl2 = el->getElementsByTagName(
                                            TBConstants::xstr(
                                            TBConstants::VIEW_FORMAT_FONT));
                                    if(nl2 != NULL && nl2->getLength() > 0) {
                                        for(unsigned int k = 0;
                                            k < nl2->getLength(); k++) {
                                            elem = (DOMElement*)nl2->item(k);
                                            QFontColor* fc = fontElem(elem);
                                            f->addFont(fc);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // table sorting
                    nl = e->getElementsByTagName(TBConstants::xstr(
                            TBConstants::VIEW_SORT));
                    if(nl != NULL && nl->getLength() > 0) {
                        el = (DOMElement*)nl->item(0);
                        nl = el->getElementsByTagName(TBConstants::xstr(
                                TBConstants::VIEW_FILTER_RULE_FIELD));
                        if(nl != NULL && nl->getLength() > 0) {
                            for(unsigned int j = 0; j < nl->getLength(); j++) {
                                el = (DOMElement*)nl->item(j);
                                String field = TBConstants::xstr(
                                        el->getAttribute(TBConstants::xstr(
                                                TBConstants::VIEW_NAME)));
                                if(field.empty()) continue;
                                String ascStr = TBConstants::xstr(
                                        el->getAttribute(TBConstants::xstr(
                                        TBConstants::VIEW_SORT_ASCENDING)));
                                TBConstants::strtrim(ascStr);
                                bool asc = ascStr != "0";
                                pair<String, bool> p(field, asc);
                                tv->sort.push_back(p);
                            }
                        }
                    }

                    view->addTableView(tv);
                }
                
                return view;
            }
        }
    } catch(...) {
        return NULL;
    }
    return NULL;
}

TBView* TBView::loadFromDefaultFile() {
    return loadFromFile(defaultFile());
}

String TBView::defaultFile() {
    return TBConstants::dotCasapyDir() + TBConstants::VIEW_SAVE_LOC;
}

// Private Methods

void TBView::fontElem(DOMElement* elem, QFontColor* font) {
    elem->setAttribute(TBConstants::xstr(TBConstants::VIEW_FORMAT_COLOR),
                       TBConstants::xstr(qPrintable(font->color.name())));
    elem->setAttribute(TBConstants::xstr(TBConstants::VIEW_FORMAT_FAMILY),
                       TBConstants::xstr(qPrintable(font->font.family())));
    elem->setAttribute(TBConstants::xstr(TBConstants::VIEW_FORMAT_SIZE),
                       TBConstants::xstr(TBConstants::itoa(
                               font->font.pointSize())));
    elem->setAttribute(TBConstants::xstr(TBConstants::VIEW_FORMAT_BOLD),
                       TBConstants::xstr(font->font.bold()?"1":"0"));
    elem->setAttribute(TBConstants::xstr(TBConstants::VIEW_FORMAT_ITALICS),
                       TBConstants::xstr(font->font.italic()?"1":"0"));
    elem->setAttribute(TBConstants::xstr(TBConstants::VIEW_FORMAT_ULINE),
                       TBConstants::xstr(font->font.underline()?"1":"0"));
    elem->setAttribute(TBConstants::xstr(TBConstants::VIEW_FORMAT_STRIKE),
                       TBConstants::xstr(font->font.strikeOut()?"1":"0"));
}

QFontColor* TBView::fontElem(DOMElement* elem) {
    String color = TBConstants::xstr(elem->getAttribute(TBConstants::xstr(
            TBConstants::VIEW_FORMAT_COLOR)));
    if(color.empty()) return NULL;
    String family = TBConstants::xstr(elem->getAttribute(TBConstants::xstr(
            TBConstants::VIEW_FORMAT_FAMILY)));
    if(family.empty()) return NULL;
    String str = TBConstants::xstr(elem->getAttribute(TBConstants::xstr(
            TBConstants::VIEW_FORMAT_SIZE)));
    int size;
    if(TBConstants::atoi(str, &size) < 1 || size < 1) return NULL;

    str = TBConstants::xstr(elem->getAttribute(TBConstants::xstr(
            TBConstants::VIEW_FORMAT_BOLD)));
    bool bold = str == "1";
    str = TBConstants::xstr(elem->getAttribute(TBConstants::xstr(
            TBConstants::VIEW_FORMAT_ITALICS)));
    bool italics = str == "1";
    str = TBConstants::xstr(elem->getAttribute(TBConstants::xstr(
            TBConstants::VIEW_FORMAT_ULINE)));
    bool uline = str == "1";
    str = TBConstants::xstr(elem->getAttribute(TBConstants::xstr(
            TBConstants::VIEW_FORMAT_STRIKE)));
    bool strike = str == "1";

    QColor qcolor(color.c_str());
    QFont qfont(family.c_str(), size);
    qfont.setBold(bold);
    qfont.setItalic(italics);
    qfont.setUnderline(uline);
    qfont.setStrikeOut(strike);
    return new QFontColor(qfont, qcolor);
}

}
