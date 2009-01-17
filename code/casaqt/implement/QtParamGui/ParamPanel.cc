//# ParamPanel.cc: Panel for a single parameter.
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
//# $Id: $
#include <casaqt/QtParamGui/QtParamGUI.qo.h>
#include <casaqt/QtParamGui/ParamPanel.qo.h>

namespace casa {

// PARAMPANEL DEFINITIONS //

ParamPanel::ParamPanel(const QtParamGUI& parent, String name,
        const RecordInterface& rec, const Record& globals, const Record& last,
        const QtParamGUI::ResetMode& rm, int labWidth, const String& consts,
        bool subparam) : m_name(name), m_record(rec), m_globals(globals),
        m_last(last), m_resetMode(rm), m_units(NULL), m_constDialog(NULL) {
    setupUi(this);
    
    // indent for subparam
    if(subparam)
        dynamic_cast<QHBoxLayout*>(frame->layout())->insertSpacing(0, 20);
    
    // if a label width is specified, use it
    QFontMetrics metric = nameLabel->fontMetrics();
    if(labWidth >= metric.width(m_name.c_str()))
        nameLabel->setFixedWidth(labWidth);
        
    // name/mustexist
    stringstream ss;
    bool b = m_record.isDefined(QtParamGUI::MUSTEXIST) &&
             m_record.asBool(QtParamGUI::MUSTEXIST);
    if(b) ss << "<font color=\"red\">";
    ss << (subparam ? "<i>" : "<b>") << m_name << (subparam ? "</i>" : "</b>");
    if(b) ss << "</font>";
    nameLabel->setText(ss.str().c_str());
    
    // description
    ss.str("");
    if(m_record.isDefined(QtParamGUI::DESCRIPTION)) {
        String desc = m_record.asString(QtParamGUI::DESCRIPTION);
        QtParamGUI::replaceSlashQuotes(desc);
        ss << desc << "<br />";
    }
    ss << "<i>Type</i>: " << m_record.asString(QtParamGUI::TYPE) << '.';
    if(m_record.isDefined(QtParamGUI::UNITS))
        ss << " <i>Units</i>: " << m_record.asString(QtParamGUI::UNITS) << '.';
    if(m_record.isDefined(QtParamGUI::EXAMPLE)) {
        const Array<String>& a = m_record.asArrayString(QtParamGUI::EXAMPLE);
        bool empty = false;
        IPosition s = a.shape();
        if(s.size() == 0) empty = true;
        for(unsigned int i = 0; i < s.size() && !empty; i++)
            if(s[i] == 0) empty = true;
        if(!empty) {
            ss << "<br /><i>Example</i>:<pre style=\"margin-top: 0px\">";
            Array<String>::ConstIteratorSTL it = a.begin();
            String str;
            for(; it != a.end(); it++) {
                str = *it;
                QtParamGUI::replaceSlashQuotes(str);
                if(it != a.begin()) ss << '\n';
                ss << "     " << str;
            }
            ss << "</pre>";
        }
    }
    descLabel->setText(ss.str().c_str());
    descLabel->setVisible(false);
    constraintsButton->setVisible(false);
    connect(descButton, SIGNAL(clicked()), this, SLOT(showHideDesc()));
    nameLabel->setToolTip(ss.str().c_str());
    
    // value panel
    m_value = ValuePanel::create(name, m_record);

    // units
    if(m_record.isDefined(QtParamGUI::UNITS))
        m_units = new QLabel(m_record.asString(QtParamGUI::UNITS).c_str());
    
    // constraints
    if(!consts.empty()) {
        connect(constraintsButton, SIGNAL(clicked(bool)),
                this, SLOT(showHideConstraints(bool)));
        m_constDialog = new QDialog(this);
        QtParamGUI::setupDialog(m_constDialog, parent.taskName(),
                                m_name + " constraints", consts);
        connect(m_constDialog, SIGNAL(rejected()),
                this, SLOT(constraintsClosed()));
        connect(&parent, SIGNAL(finished(int)), m_constDialog, SLOT(close()));
    }
    
    setValue();
    
    QHBoxLayout* l = new QHBoxLayout(valueFrame);    
#if QT_VERSION >= 0x040300
    l->setContentsMargins(0, 0, 0, 0);
#else
    l->setMargin(0);
#endif
    l->setSpacing(3);
    l->addWidget(m_value);
    if(m_units != NULL) {
        l->addWidget(m_units);
        l->addStretch();
    }
    
    connect(m_value, SIGNAL(valueChanged()), this, SLOT(changedValue()));
    connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));
}

ParamPanel::~ParamPanel() {
    if(m_constDialog != NULL) delete m_constDialog;
}


// Public Methods //

String ParamPanel::name() { return m_name; }

ValuePanel* ParamPanel::value() { return m_value; }

void ParamPanel::getValue(Record& record) {
    m_value->getValue(m_name, record);
}

pair<bool, String> ParamPanel::isValid() {
    return m_value->valueIsValid();
}

void ParamPanel::reset() { setValue(); }


// Private Methods //

void ParamPanel::setValue() {
    const RecordInterface* rec(NULL);
    String id;
    if(m_resetMode == QtParamGUI::GLOBAL) {
        if(!m_globals.isDefined(m_name)) return;
        rec = &m_globals;
        id = m_name;
    } else if(m_resetMode == QtParamGUI::LAST) {
        if(!m_last.isDefined(m_name)) return;
        rec = &m_last;
        id = m_name;
    } else if(m_resetMode == QtParamGUI::DEFAULT) {
        if(!m_record.isDefined(QtParamGUI::VALUE)) return;
        rec = &m_record;
        id = QtParamGUI::VALUE;
    }
    
    m_value->setValue(*rec, id);
}


// Private Slots //

void ParamPanel::showHideDesc() {
    descLabel->setVisible(!descLabel->isVisible());
    descButton->setText((descLabel->isVisible() ? "-" : "+"));
    constraintsButton->setVisible(descLabel->isVisible() &&
                                  m_constDialog != NULL);
}

void ParamPanel::showHideConstraints(bool checked) {
    m_constDialog->setVisible(checked);
}

void ParamPanel::constraintsClosed() {
    constraintsButton->setChecked(false);
}

}
