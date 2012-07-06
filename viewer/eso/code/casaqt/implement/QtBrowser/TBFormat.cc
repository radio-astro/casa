//# TBFormat.cc: Rules used to format displayed values for fields.
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
#include <casaqt/QtBrowser/TBFormat.qo.h>
#include <casaqt/QtBrowser/TBData.h>

#include <sstream>
#include <iomanip>

namespace casa {

////////////////////////////
// QFONTCOLOR DEFINITIONS //
////////////////////////////

// Constructors/Destructors //

QFontColor::QFontColor(QFont f, QColor c) : font(f), color(c) { }

QFontColor::QFontColor(const QFontColor& fc): font(fc.font),color(fc.color) { }

QFontColor::~QFontColor() { }

//////////////////////////
// TBFORMAT DEFINITIONS //
//////////////////////////

// Constructors/Destructors //

TBFormat::TBFormat() : allFont(NULL), fonts(),
        decimalPlaces(TBConstants::DEFAULT_DECIMALS), scientificFormat(false),
        boolFormat(DEFAULT), dateFormat("%y-%m-%d-%h:%n:%s"),
        vectorThreshold(-1) { }

TBFormat::~TBFormat() {
    if(allFont != NULL) delete allFont;

    for(unsigned int i = 0; i < fonts.size(); i++)
        if(fonts.at(i) != NULL) delete fonts.at(i);
}

// Accessor/Mutator Methods //

QFontColor* TBFormat::getAllFont() { return allFont; }

void TBFormat::setAllFont(QFontColor* fc) { allFont = fc; }

vector<QFontColor*>* TBFormat::getFonts() { return &fonts; }

void TBFormat::setFonts(vector<QFontColor*>* f) {
    for(unsigned int i = 0; i < fonts.size(); i++)
        delete fonts.at(i);
        
    for(unsigned int i = 0; i < f->size(); i++)
        fonts.push_back(new QFontColor(*f->at(i)));
}

void TBFormat::addFont(QFontColor* fc) { if(fc != NULL) fonts.push_back(fc); }

String TBFormat::getDateFormat() { return dateFormat; }

void TBFormat::setDateFormat(String d) { dateFormat = d; }

BooleanFormat TBFormat::getBoolFormat() { return boolFormat; }

void TBFormat::setBoolFormat(BooleanFormat f) { boolFormat = f; }

bool TBFormat::getScientificFormat() { return scientificFormat; }

void TBFormat::setScientificFormat(bool sf) { scientificFormat = sf; }

int TBFormat::getDecimalPlaces() { return decimalPlaces; }

void TBFormat::setDecimalPlaces(int dp) { decimalPlaces = dp; }

int TBFormat::getVectorThreshold() { return vectorThreshold; }

void TBFormat::setVectorThreshold(int v) { vectorThreshold = v; }

// Public Methods //

void TBFormat::applyTo(QTableWidgetItem* item, TBData* data) {
    if(item == NULL || data == NULL) return;
    
    String type = data->getType();
    
    // Font
    if(allFont != NULL) {
        item->setFont(allFont->font);
        item->setForeground(QBrush(allFont->color));
    } else {
        if((type == TBConstants::TYPE_FLOAT ||
            type == TBConstants::TYPE_DOUBLE ||
            type == TBConstants::TYPE_INT ||
            type == TBConstants::TYPE_SHORT) && fonts.size() >= 2) {
            double d = data->asDouble();

            QFontColor* fc = NULL;
            if(d >= 0)
                fc = fonts.at(0);
            else
                fc = fonts.at(1);

            item->setFont(fc->font);
            item->setForeground(QBrush(fc->color));
                
        } else if(type == TBConstants::TYPE_BOOL && fonts.size() >= 2) {
            QFontColor* fc = NULL;
            if(data->asBool()) fc = fonts.at(0);
            else fc = fonts.at(1);

            item->setFont(fc->font);
            item->setForeground(QBrush(fc->color));
                
        } else if((TBConstants::typeIsComplex(type) &&
                   !TBConstants::typeIsArray(type)) && fonts.size() >= 2) {
            double a = data->asDComplex().first;

            QFontColor* fc = NULL;
            if(a >= 0) fc = fonts.at(0);
            else fc = fonts.at(1);

            item->setFont(fc->font);
            item->setForeground(QBrush(fc->color));
        } else {
            QTableWidget* tw = item->tableWidget();
            item->setFont(tw->font());
            item->setForeground(tw->palette().color(tw->foregroundRole()));
        }
    }

    // Format
    String str = data->asString();
    if(!TBConstants::typeIsArray(type)) {
        if(type == TBConstants::TYPE_DATE) {
            if(dateFormat != "%y-%m-%d-%h:%n:%s") {
                int y, m, d, h, n;
                double s;
                String sd = TBConstants::date(data->asDouble(), -1);
                int r = sscanf(sd.c_str(),"%d-%d-%d-%d:%d:%lf", &y, &m, &d, &h,
                               &n, &s);

                if(r == 6) {
                    stringstream sb;
                    for(unsigned i = 0; i < dateFormat.length(); i++) {
                        char c = dateFormat[i];
                        if(c == '%' && i < dateFormat.length() - 1) {
                            c = dateFormat[++i];
                            switch(c) {
                            case 'y': sb << y; break;
                            case 'm': sb << m; break;
                            case 'd': sb << d; break;
                            case 'h': sb << h; break;
                            case 'n': sb << n; break;
                            case 's':
                                if(decimalPlaces == -1) {
                                    sb << TBConstants::dtoa(s);
                                } else {
                                    sb << fixed << setprecision(decimalPlaces);
                                    sb << s;
                                }
                                break;

                            default: sb << dateFormat[i - 1] << c; break;
                            }
                        } else {
                            sb << c;
                        }
                    }
                    str = String(sb.str());
                }
            } else if(decimalPlaces != TBConstants::DEFAULT_DATE_DECIMALS) {
                str = TBConstants::date(data->asDouble(), decimalPlaces);
            }
            
        } else if(scientificFormat || decimalPlaces > -1) {
            if(type == TBConstants::TYPE_FLOAT ||
               type == TBConstants::TYPE_DOUBLE) {
                str = TBConstants::dtoa(data->asDouble(), decimalPlaces,
                        scientificFormat);

            } else if(TBConstants::typeIsComplex(type)) {
                pair<double, double> c = data->asDComplex();
                str = "(" + TBConstants::dtoa(c.first, decimalPlaces,
                      scientificFormat) + "," + TBConstants::dtoa(c.second,
                      decimalPlaces, scientificFormat) + ")";
            }
        
        } else if(type == TBConstants::TYPE_BOOL && boolFormat != DEFAULT) {
            if(boolFormat == TRUEFALSE) str = (data->asBool()?"true":"false");
            else if(boolFormat == TF) str = (data->asBool()?"T":"F");
        
        }
    } else {
        TBArrayData* ad = (TBArrayData*)data;
        String at = TBConstants::arrayType(type);
        if(vectorThreshold > -1 && ad->isOneDimensional()) {
            vector<int> shape = ad->getShape();
            int d = (shape.size() == 1) ? shape[0] : shape[1];
            
            if(d > vectorThreshold) {
                str = "[1, " + TBConstants::itoa(d) + "] " +
                      TBConstants::typeName(at);
            }
        }
    }
    item->setText(str.c_str());
}

/////////////////////////////
// TBFORMATTER DEFINITIONS //
/////////////////////////////

// Constructors/Destructors //

TBFormatter::TBFormatter(String fd, String t, int i, QFontColor fc, QWidget* p)
    : QDialog(p), field(fd), type(t), index(i), update(true) {
    
    setupUi(this);
    
    fieldLabel->setText(field.c_str());
    typeLabel->setText(TBConstants::typeName(type).c_str());
    setFontColor(&fc);

    QFont f = boldBox->font();
    f.setBold(true);
    boldBox->setFont(f);
    f = italicsBox->font();
    f.setItalic(true);
    italicsBox->setFont(f);
    f = underlineBox->font();
    f.setUnderline(true);
    underlineBox->setFont(f);
    f = strikethroughBox->font();
    f.setStrikeOut(true);
    strikethroughBox->setFont(f);

    // Connect widgets
    connect(colorButton, SIGNAL(clicked()), this, SLOT(changeColor()));
    connect(clearButton, SIGNAL(clicked()), this, SLOT(clearFormat()));
    connect(setButton, SIGNAL(clicked()), this, SLOT(setFormat()));
    connect(allCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(applyAllTurned(bool)));
    connect(listWidget, SIGNAL(currentRowChanged(int)),
            this, SLOT(applySelectionChanged(int)));
    setAttribute(Qt::WA_DeleteOnClose);

    connect(fontChooser,SIGNAL(currentIndexChanged(int)),
            this, SLOT(valuesChanged()));
    connect(fontSizeSpinner, SIGNAL(valueChanged(int)),
            this, SLOT(valuesChanged()));
    connect(boldBox, SIGNAL(toggled(bool)), this, SLOT(valuesChanged()));
    connect(italicsBox, SIGNAL(toggled(bool)), this, SLOT(valuesChanged()));
    connect(colorEdit, SIGNAL(textChanged(QString)),
            this, SLOT(valuesChanged()));
    connect(underlineBox, SIGNAL(toggled(bool)), this, SLOT(valuesChanged()));
    connect(strikethroughBox, SIGNAL(toggled(bool)),
            this, SLOT(valuesChanged()));

    if(TBConstants::typeIsArray(type)) type = TBConstants::arrayType(type);
    
    // Hide panels as appropriate
    if(type == TBConstants::TYPE_FLOAT || type == TBConstants::TYPE_DOUBLE) {
        noneLabel->hide();
        boolFrame->hide();
        dateFrame->hide();
        vectorFrame->hide();

        if(TBConstants::DEFAULT_DECIMALS > -1) {
            decCustom->setChecked(true);
            decimalsSpinBox->setValue(TBConstants::DEFAULT_DECIMALS);
        }
        
        listWidget->addItem("Nonnegative values");
        listWidget->addItem("Negative values");
        
    } else if(type== TBConstants::TYPE_INT || type== TBConstants::TYPE_SHORT) {
        noneLabel->hide();
        decimalFrame->hide();
        boolFrame->hide();
        dateFrame->hide();
        vectorFrame->hide();

        listWidget->addItem("Nonnegative values");
        listWidget->addItem("Negative values");
        
    } else if(type == TBConstants::TYPE_BOOL) {
        noneLabel->hide();
        decimalFrame->hide();
        sfFrame->hide();
        dateFrame->hide();
        vectorFrame->hide();

        listWidget->addItem("True values");
        listWidget->addItem("False values");
        
    } else if(TBConstants::typeIsComplex(type)) {
        noneLabel->hide();
        boolFrame->hide();
        dateFrame->hide();
        vectorFrame->hide();

        if(TBConstants::DEFAULT_DECIMALS > -1) {
            decCustom->setChecked(true);
            decimalsSpinBox->setValue(TBConstants::DEFAULT_DECIMALS);
        }
        
        listWidget->addItem("Nonnegative amplitude values");
        listWidget->addItem("Negative amplitude values");
        
    } else if(type == TBConstants::TYPE_DATE) {
        noneLabel->hide();
        sfFrame->hide();
        boolFrame->hide();
        vectorFrame->hide();
        
        if(TBConstants::DEFAULT_DATE_DECIMALS > -1) {
            decCustom->setChecked(true);
            decimalsSpinBox->setValue(TBConstants::DEFAULT_DATE_DECIMALS);
        }

        allCheckBox->setEnabled(false);
        
    } else {
        decimalFrame->hide();
        sfFrame->hide();
        boolFrame->hide();
        dateFrame->hide();
        vectorFrame->hide();

        allCheckBox->setEnabled(false);
    }

    type = t;
    if(TBConstants::typeIsArray(type)) {
        noneLabel->hide();
        vectorFrame->show();
    }
}

TBFormatter::~TBFormatter() {
    for(unsigned int i = 0; i < fonts.size(); i++)
        if(fonts.at(i) != NULL) delete fonts.at(i);
}

void TBFormatter::setFormat(TBFormat* f) {
    if(f->getDecimalPlaces() == UNLIMITED_DECIMALS) {
        decUnlimited->setChecked(true);
        decimalsSpinBox->setValue(0);
    } else if(f->getDecimalPlaces() >= 0) {
        decCustom->setChecked(true);
        decimalsSpinBox->setValue(f->getDecimalPlaces());
    } else {
        decDefault->setChecked(true);
        decimalsSpinBox->setValue(0);
    }
    sfCheckBox->setChecked(f->getScientificFormat());
    BooleanFormat bf = f->getBoolFormat();
    if(bf == TRUEFALSE) {
        boolTrueFalse->setChecked(true);
    } else if(bf == TF) {
        boolTF->setChecked(true);
    } else if(bf == B10) {
        bool10->setChecked(true);
    }
    dateEdit->setText(f->getDateFormat().c_str());
    if(f->getVectorThreshold() >= 0) {
        vectorCheckBox->setChecked(false);
        vectorSpinBox->setValue(f->getVectorThreshold());
    }

    if(f->getAllFont() != NULL) {
        allCheckBox->setChecked(true);
        setFontColor(f->getAllFont());
    } else {
        allCheckBox->setChecked(false);

        vector<QFontColor*>* fs = f->getFonts();
        for(unsigned int i = 0; i < fs->size(); i++) {
            QFontColor* fc = fs->at(i);
            fonts[i] = new QFontColor(*fc);
        }
        applySelectionChanged(0);
    }
}

// Private Methods //

QFontColor* TBFormatter::getFont() {
    QFont f(fontChooser->currentFont());
    f.setPointSize(fontSizeSpinner->value());
    f.setBold(boldBox->isChecked());
    f.setItalic(italicsBox->isChecked());
    f.setUnderline(underlineBox->isChecked());
    f.setStrikeOut(strikethroughBox->isChecked());

    QColor c(colorEdit->text());
    
    return new QFontColor(f, c);
}

void TBFormatter::setFontColor(QFontColor* fc) {
    if(fc == NULL) return;

    update = false;
    fontChooser->setCurrentFont(fc->font);
    fontSizeSpinner->setValue(fc->font.pointSize());
    boldBox->setChecked(fc->font.bold());
    italicsBox->setChecked(fc->font.italic());
    underlineBox->setChecked(fc->font.underline());
    strikethroughBox->setChecked(fc->font.strikeOut());
    colorEdit->setText(fc->color.name());
    update = true;
}

// Private Slots //

void TBFormatter::changeColor() {
    QColor c = QColorDialog::getColor(Qt::black, this);
    if(c.isValid())
        colorEdit->setText(c.name());
}

void TBFormatter::clearFormat() {
    emit clearRequested(index);
    close();
    if(parent() == NULL) deleteLater();
}

void TBFormatter::setFormat() {
    // check that date format is valid
    TBFormat* f = new TBFormat();
    String date;
    if(dateFrame->isVisible()) {
        date = qPrintable(dateEdit->text());

        if(!TBConstants::dateFormatIsValid(date)) {
            QMessageBox::critical(this, "Date Format Error",
                                  "The date format entered is invalid.");
            return;
        } else {
            TBConstants::strtrim(date);
            f->setDateFormat(date);
        }
    }
    if(boolFrame->isVisible()) {
        if(boolTrueFalse->isChecked()) {
            f->setBoolFormat(TRUEFALSE);
        } else if(boolTF->isChecked()) {
            f->setBoolFormat(TF);
        } else if(bool10->isChecked()) {
            f->setBoolFormat(B10);
        } else {
            f->setBoolFormat(DEFAULT);
        }
    }
    if(sfFrame->isVisible()) {
        f->setScientificFormat(sfCheckBox->isChecked());
    }
    if(decimalFrame->isVisible()) {
        if(decDefault->isChecked()) {
            f->setDecimalPlaces(-1);
        } else if(decUnlimited->isChecked()) {
            f->setDecimalPlaces(UNLIMITED_DECIMALS);
        } else {
            f->setDecimalPlaces(decimalsSpinBox->value());
        }
    }
    if(vectorFrame->isVisible()) {
        if(vectorCheckBox->isChecked()) {
            f->setVectorThreshold(-1);
        } else {
            f->setVectorThreshold(vectorSpinBox->value());
        }
    }

    // Fonts
    if(allCheckBox->isChecked()) {
        f->setAllFont(getFont());
    } else {
        f->setFonts(&fonts);
    }

    emit setRequested(index, f);
    close();
    if(parent() == NULL) deleteLater();
}

void TBFormatter::applyAllTurned(bool on) {
    for(unsigned int i = 0; i < fonts.size(); i++)
        delete fonts.at(i);
    fonts.clear();
    
    if(!on) {
        QFontColor* f = getFont();

        for(int i = 0; i < listWidget->count(); i++) {
            fonts.push_back(new QFontColor(*f));
        }

        if(listWidget->currentRow() == -1)
            listWidget->setCurrentRow(0);
        else
            applySelectionChanged(listWidget->currentRow());
    }
}

void TBFormatter::applySelectionChanged(int i) {
    if(i == -1 || allCheckBox->isChecked()) return;

    QFontColor* f = fonts.at(i);
    setFontColor(f);
}

void TBFormatter::valuesChanged() {
    if(!update || allCheckBox->isChecked() || listWidget->currentRow() == -1)
        return;
    
    QFontColor* f = getFont();
    QFontColor* font = fonts.at(listWidget->currentRow());

    font->font.setFamily(f->font.family());
    font->font.setBold(f->font.bold());
    font->font.setItalic(f->font.italic());
    font->font.setPointSize(f->font.pointSize());
    font->font.setUnderline(f->font.underline());
    font->font.setStrikeOut(f->font.strikeOut());
    font->color.setNamedColor(f->color.name());

    delete f;
}


const int TBFormatter::UNLIMITED_DECIMALS = 64;

}
